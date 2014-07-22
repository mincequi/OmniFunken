#include "rtpbufferalt.h"

#include <QtDebug>

RtpBuffer::RtpBuffer(int latency, QObject *parent) :
    QObject(parent),
    m_latency(latency),
    m_first(0),
    m_last(0),
    m_ready(false),
    m_data(NULL),
    m_packetSize(0),
    m_initSeq(-1)
{
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
}

RtpBuffer::~RtpBuffer()
{
    free();
}

void RtpBuffer::setPacketSize(int frames)
{
    m_packetSize = frames;
    m_capacity = 2*(44100*m_latency)/(m_packetSize*1000);
    alloc();
}

RtpBuffer::RtpPacket* RtpBuffer::obtainPacket(quint16 sequenceNumber)
{
    // producer wants to put a packet
    RtpPacket* packet = NULL;

    m_mutex.lock();

    requestMissingPackets();

    // if we wait for start of stream
    if (m_initSeq != -1) {
        qint16 initDiff = sequenceNumber-m_initSeq;
        if (initDiff >= 0) {
            m_first = m_initSeq % m_capacity;
            m_data[m_first].sequenceNumber   = m_initSeq;
            m_data[m_first].status           = PacketMissing;
            m_last  = sequenceNumber % m_capacity;
            packet = &(m_data[m_last]);
            packet->sequenceNumber  = sequenceNumber;
            packet->status          = PacketOk;
            m_initSeq = -1;
        }
    } else {
        // compute sequence diff from previous packet
        qint16 diff = sequenceNumber-m_data[m_last].sequenceNumber;

        // if packet is too late. this should happen rarely.
        if (diff <= (-44100*m_latency)/(m_packetSize*1000)) {
            qCritical() << __func__ << ": packet too late: " << diff;
            m_mutex.unlock();
            return NULL;
        }

        // if packet is in order or skipped
        if (diff >= 1) {
            // if packets are skipped
            if (diff > 1) {
                qDebug() << __func__ << ": early packet/lost packets";
                for (quint16 i = m_data[m_last].sequenceNumber+1; i != sequenceNumber; ++i) { // mark missing
                    m_data[i%m_capacity].status = PacketMissing;
                    m_data[i%m_capacity].sequenceNumber = i;
                }
            }
            // if packet was rendered as ok before
            if (m_data[sequenceNumber % m_capacity].status == PacketOk) {
                qDebug() << __func__ << ": buffer full, overwriting samples";
            }
            m_last = sequenceNumber % m_capacity;
        // if packet is late (it is not too late)
        } else if (diff < 0) {
            qDebug() << __func__ << ": late packet/eventually from retransmit";
        // if packet is sent twice (this shall NEVER happen)
        } else if (diff == 0) {
            qDebug() << __func__ << ": packet sent twice";
        }

        packet = &(m_data[sequenceNumber % m_capacity]);
        packet->sequenceNumber  = sequenceNumber;
        packet->status          = PacketOk;
    }

    if (!packet) {
        m_mutex.unlock();
    }
    return packet;
}

void RtpBuffer::commitPacket()
{
    quint16 size = (m_data[m_last].sequenceNumber-m_data[m_first].sequenceNumber);
    if (!m_ready && size >= (44100*m_latency)/(m_packetSize*1000)) {
        m_ready = true;
        m_timer.start(1000);
        m_mutex.unlock();
        emit ready();
        return;
    }

    m_mutex.unlock();
}

const RtpBuffer::RtpPacket* RtpBuffer::takePacket()
{
    QMutexLocker locker(&m_mutex);

    // take from top, until free packet
    RtpPacket* packet = &(m_data[m_first]);

    switch (packet->status) {
    case PacketFree:
        qDebug("RtpBuffer::takePacket: free packet: %d", packet->sequenceNumber);
        m_timer.stop();
        m_ready = false;
        packet = NULL;
        break;
    case PacketMissing:
        qCritical("RtpBuffer::takePacket: missing packet: %d", packet->sequenceNumber);
        memcpy(packet->payload, m_silence, packet->payloadSize);
    case PacketOk:
        m_first = (m_first+1) % m_capacity;
        packet->init();
        break;
    default:
        qFatal("RtpBuffer::takePacket: invalid packet status");
        packet = NULL;
    }

    return packet;
}

void RtpBuffer::silence(char **silence, int *size) const
{
    *silence = m_silence;
    *size   = m_packetSize*4;
}

void RtpBuffer::init(quint16 seq)
{
    QMutexLocker locker(&m_mutex);

    m_ready = false;
    for (int i = 0; i < m_capacity; ++i) {
        m_data[i].init();
    }
    m_initSeq = seq;
}

void RtpBuffer::timeout()
{
    quint16 size = 0;
    m_mutex.lock();
    size = m_data[m_last].sequenceNumber-m_data[m_first].sequenceNumber;
    m_mutex.unlock();
    emit notify(size);
}

void RtpBuffer::alloc()
{
    free();

    m_data = new RtpPacket[m_capacity];
    for (int i = 0; i < m_capacity; ++i) {
        m_data[i].payload = new char[m_packetSize*5]; //should be 4, we put some extra space
    }
    m_silence = new char[m_packetSize*5];
    memset(m_silence, 0, m_packetSize*5);
}

void RtpBuffer::free()
{
    if (m_data) {
        for (int i = 0; i < m_capacity; ++i) {
            delete[] m_data[i].payload;
        }
        delete[] m_data;
        m_data = NULL;
    }

    if (m_silence) {
        delete[] m_silence;
        m_silence = NULL;
    }
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
        // find endOfSequence of missing packets
        if ((m_data[(i+1)%m_capacity].status != PacketMissing) && startOfSequence != -1) {
            endOfSequence = m_data[i].sequenceNumber;

            // request every fifth time
            if (((m_data[startOfSequence%m_capacity].requestCount)%5)==0) {
                qDebug("requestMissingPackets: %d to %d", quint16(startOfSequence), quint16(endOfSequence));
                emit missingSequence(startOfSequence, (endOfSequence-startOfSequence+1));
            }

            startOfSequence = -1;
            endOfSequence   = -1;
        }

        if (startOfSequence != -1) {
            m_data[i].requestCount++;
        }
    }
    if (startOfSequence != -1) {
        qFatal("impossible");
    }
}
