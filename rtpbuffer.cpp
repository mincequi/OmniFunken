#include "rtpbuffer.h"

#include <QThread>

RtpBuffer::RtpBuffer(int latency, QObject *parent) :
    QObject(parent),
    m_init(true),
    m_ready(false),
    m_latency(latency),
    m_size(0),
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
    m_size = 0;
}

RtpBuffer::RtpPacket* RtpBuffer::putPacket(quint16 index)
{
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

    ++m_size;
    m_last = index % m_capacity;
    RtpPacket* packet = &(m_data[m_last]);
    packet->sequenceNumber = index;
    packet->status = PacketOk;
    packet->requestCount = 0;

    if (!m_ready && (m_size >= (44100*m_latency)/(m_packetSize*1000))) {
        m_ready = true;
        locker.unlock();
        emit ready();
    }

    return packet;
}


const RtpBuffer::RtpPacket* RtpBuffer::takePacket()
{
    QMutexLocker locker(&m_mutex);

    if (m_size <= 0) {
        reset();
        return NULL;
    }

    --m_size;
    RtpPacket* packet = &(m_data[m_first]);
    m_first = (m_first+1) % m_capacity;

    if (packet->status == PacketMissing) {
        qCritical("takePacket: missing packet: %d", packet->sequenceNumber);
    }

    //packet->status = PacketFree;
    packet->requestCount = 0;
    return packet;
}

void RtpBuffer::checkPacketOrder(quint16 index)
{
    // if packet disorder
    if (m_lastIndex+1 != index)
    {
        if (m_lastIndex == index)
        {
            qWarning("checkPacketOrder: packet sent twice");
        }
        else if (m_lastIndex+1 < index)
        {
            qWarning("checkPacketOrder: packet sent early or previous is missing");
            // mark missing
            for (quint16 i = m_lastIndex+1; i != index; ++i) {
                m_data[i%m_capacity].status = PacketMissing;
                m_data[i%m_capacity].sequenceNumber = i;
            }
        }
        else
        {
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
            qWarning("requestMissingPackets: %d to %d", quint16(startOfSequence), quint16(endOfSequence-1));
            emit missingSequence(startOfSequence, (endOfSequence-startOfSequence));
            startOfSequence = -1;
            endOfSequence   = -1;
        }

        if (startOfSequence != -1) {
            m_data[i].requestCount++;
        }
    }
    if (startOfSequence != -1) {
        qWarning("requestMissingPackets: %d to %d", quint16(startOfSequence), quint16(m_data[m_last%m_capacity].sequenceNumber-1));
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
}

void RtpBuffer::free()
{
    for (int i = 0; i < m_capacity; ++i)
    {
        delete[] m_data[i].payload;
    }
    delete[] m_data;

    m_data = NULL;
}
