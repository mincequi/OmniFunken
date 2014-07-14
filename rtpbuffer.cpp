#include "rtpbuffer.h"

#include <QThread>

RtpBuffer::RtpBuffer(int latency, QObject *parent) :
    QObject(parent),
    m_init(true),
    m_ready(false),
    m_latency(latency),
    m_first(0),
    m_last(0),
    m_data(NULL)
{
}

void RtpBuffer::setPacketSize(uint frames)
{
    m_packetSize = frames;
    reset();
    alloc();
}

void RtpBuffer::reset()
{
    m_init  = true;
    m_ready = false;
}

quint16 RtpBuffer::size()
{
    QMutexLocker locker(&m_mutex);

    quint16 size = (m_data[m_last].sequenceNumber-m_data[m_first].sequenceNumber+1)%m_capacity;
    return size;
}

RtpBuffer::RtpPacket* RtpBuffer::putPacket(quint16 index)
{
    quint16 size = this->size();
    QMutexLocker locker(&m_mutex);

    // set first index
    if (m_init) {
        m_lastIndex = index;
        m_first = index % m_capacity;
        m_init = false;
    } else {
        requestMissingPackets();
        checkPacketOrder(index);
    }

    m_last = index % m_capacity;
    RtpPacket* packet = &(m_data[m_last]);
    packet->sequenceNumber = index;
    packet->status = PacketOk;
    packet->requestCount = 0;

    if (!m_ready && (size >= (44100*m_latency)/(m_packetSize*1000))) {
        m_ready = true;
        locker.unlock();
        emit ready();
    }

    return packet;
}

RtpBuffer::RtpPacket* RtpBuffer::putLatePacket(quint16 index)
{
    QMutexLocker locker(&m_mutex);

    qDebug("RtpBuffer::putLatePacket: late packet arrived: %d", index);
    // check if it's too late (seqno must be greater than first)
    //quint16 diff = index-m_data[m_first].sequenceNumber

    //(44100*m_latency)/(m_packetSize*1000)

    RtpPacket* packet = &(m_data[index%m_capacity]);
    packet->sequenceNumber = index;
    packet->status = PacketOk;
    packet->requestCount = 0;

    return packet;
}

const RtpBuffer::RtpPacket* RtpBuffer::takePacket()
{
    QMutexLocker locker(&m_mutex);

    RtpPacket* packet = &(m_data[m_first]);

    switch (packet->status)
    {
    case PacketFree:
        reset();
        packet = NULL;
        break;
    case PacketMissing:
        qCritical("takePacket: missing packet: %d", packet->sequenceNumber);
        memcpy(packet->payload, m_silence, packet->payloadSize);
    case PacketOk:
        m_first = (m_first+1) % m_capacity;
        packet->status = PacketFree;
        packet->requestCount = 0;
        break;
    default:
        qCritical("takePacket: invalid packet status");
        packet = NULL;
    }

    return packet;
}

void RtpBuffer::checkPacketOrder(quint16 index)
{
    // if packet disorder
    if (m_lastIndex+1 != index) {
        if (m_lastIndex == index) {
            qWarning("checkPacketOrder: packet sent twice");
        } else if (m_lastIndex+1 < index) {
            qWarning("checkPacketOrder: packet sent early or previous is missing");
            for (quint16 i = m_lastIndex+1; i != index; ++i) { // mark missing
                m_data[i%m_capacity].status = PacketMissing;
                m_data[i%m_capacity].sequenceNumber = i;
            }
        } else {
            qWarning("checkPacketOrder: packet sent late");
        }
    }
    m_lastIndex = index;
}

void RtpBuffer::requestMissingPackets()
{
    int startOfSequence = -1;
    int endOfSequence   = -1;

    for (int i = m_first; i != m_last; i = (i+1)%m_capacity) {
        // find startOfSequence of missing packets
        if (m_data[i].status == PacketMissing && startOfSequence == -1) {
            startOfSequence = m_data[i].sequenceNumber;
        }
        else if ((m_data[i].status != PacketMissing) && startOfSequence != -1) {
            endOfSequence = m_data[i].sequenceNumber;
            qDebug("requestMissingPackets: %d to %d", quint16(startOfSequence), quint16(endOfSequence-1));
            if (((m_data[(i-1)%m_capacity].requestCount)%5)==0) // request every fifth time
                emit missingSequence(startOfSequence, (endOfSequence-startOfSequence));
            startOfSequence = -1;
            endOfSequence   = -1;
        }

        if (startOfSequence != -1) {
            m_data[i].requestCount++;
        }
    }
    if (startOfSequence != -1) {
        qDebug("requestMissingPackets: %d to %d", quint16(startOfSequence), quint16(m_data[m_last%m_capacity].sequenceNumber-1));
        if (((m_data[m_last%m_capacity].requestCount)%5)==0)// request every fifth time
            emit missingSequence(startOfSequence, (m_data[m_last%m_capacity].sequenceNumber-startOfSequence));
    }
}

void RtpBuffer::alloc()
{
    if (m_data) free();

    m_capacity = 2*(44100*m_latency)/(m_packetSize*1000);
    m_data = new RtpPacket[m_capacity];
    for (int i = 0; i < m_capacity; ++i) {
        m_data[i].status = PacketFree;
        m_data[i].requestCount = 0;
        m_data[i].payloadSize = 0;
        m_data[i].payload = new char[m_packetSize*5]; //should be 4, we put some extra space
    }
    m_silence = new char[m_packetSize*5];
    memset(m_silence, 0, m_packetSize*5);
}

void RtpBuffer::free()
{
    for (int i = 0; i < m_capacity; ++i) {
        delete[] m_data[i].payload;
    }
    delete[] m_data;
    delete[] m_silence;

    m_data = NULL;
    m_silence = NULL;
}
