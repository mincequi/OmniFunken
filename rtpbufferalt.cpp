#include "rtpbufferalt.h"

RtpBuffer::RtpBuffer(int latency, QObject *parent) :
    QObject(parent),
    m_latency(latency),
    m_first(0),
    m_last(0),
    m_init(true),
    m_ready(false),
    m_data(NULL),
    m_packetSize(0),
    m_flushSeq(-1)
{
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

    // might be start of stream or an expected packet (usually)
    // ->   if (sequenceNumber == lastSequenceNumber+1)
    // ->   return packet
    m_mutex.lock();
    //if (m_init || sequenceNumber == m_data[m_last].sequenceNumber+1) {
        m_last = sequenceNumber % m_capacity;
        packet = &(m_data[m_last]);
        packet->init();
        packet->sequenceNumber  = sequenceNumber;
        packet->status          = PacketOk;

        if (!m_init && m_last == m_first) {
            qCritical("RtpBuffer::obtainPacket: buffer full, overwriting samples");
            //m_first = (m_first+1) % m_capacity;
        }
    /*} else {
        qDebug("RtpBuffer::obtainPacket: inconsistency %d -> %d", m_data[m_last].sequenceNumber, sequenceNumber);
        m_mutex.unlock();
    }*/

    // might be newer than expected
    // ->   if(sequenceNumber >= lastSequenceNumber+1)
    // ->
    // ->   return packet
    // might be a late packet
    // ->   if (sequenceNumer >
    // ->   return packet
    // might be a too late packet (rarely)
    // ->   if (sequenceNumber <= firstSequenceNumber
    // ->   return NULL

    return packet;
}

void RtpBuffer::commitPacket()
{
    /*
    if (m_init) {
        m_first = m_last;
        m_init = false;
    } else {*/
        quint16 size = (m_data[m_last].sequenceNumber-m_data[m_first].sequenceNumber);
        if (!m_ready && size >= (44100*m_latency)/(m_packetSize*1000)) {
            m_ready = true;
            m_mutex.unlock();
            emit ready();
            return;
        }
    //}

    m_mutex.unlock();
}

const RtpBuffer::RtpPacket* RtpBuffer::takePacket()
{
    QMutexLocker locker(&m_mutex);

    // take from top, until free packet
    RtpPacket* packet = &(m_data[m_first]);

    // if sequence is flushed
    if (m_flushSeq == packet->sequenceNumber) {
        m_first = m_flushSeq % m_capacity;
        m_last  = m_first;
        m_flushSeq = -1;
        m_ready = false;
        return NULL;
    }

    switch (packet->status) {
    case PacketFree:
        qCritical("RtpBuffer::takePacket: free packet: %d", packet->sequenceNumber);
        //Q_ASSERT(m_first == m_last);
        //locker.unlock();
        //init();
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

void RtpBuffer::flush(quint16 seq)
{
    m_flushSeq = seq;
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

void RtpBuffer::init()
{
    qDebug("RtpBuffer::init: init");

    QMutexLocker locker(&m_mutex);
    m_init  = true;
    m_ready = false;
    for (int i = 0; i < m_capacity; ++i) {
        m_data[i].init();
    }
}

bool RtpBuffer::isFull()
{
    // TODO
    return (m_last + 1) % m_capacity == m_first;
}

bool RtpBuffer::isEmpty()
{
    // TODO
    return m_last == m_first;
}
