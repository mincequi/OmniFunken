#include "rtpbufferalt.h"

RtpBufferAlt::RtpBufferAlt(int latency, QObject *parent) :
    QObject(parent),
    m_latency(latency),
    m_first(0),
    m_last(0),
    m_data(NULL),
    m_packetSize(0)
{
}

RtpBufferAlt::~RtpBufferAlt()
{
    free();
}

void RtpBufferAlt::setPacketSize(int frames)
{
    m_packetSize = frames;
    m_capacity = 2*(44100*m_latency)/(m_packetSize*1000);
    alloc();
}

RtpBufferAlt::RtpPacket* RtpBufferAlt::obtainPacket(quint16 sequenceNumber)
{
    // producer wants to put a packet
    // might be an expected packet (usual)
    // ->   if (sequenceNumber == lastSequenceNumber+1)
    // ->   return packet
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
}

void RtpBufferAlt::commitPacket()
{
    //
}

const RtpBufferAlt::RtpPacket* RtpBufferAlt::takePacket()
{
    // take from top, until free packet
    RtpPacket* packet = &(m_data[m_first]);

    switch (packet->status) {
    case PacketFree:
        init();
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

void RtpBufferAlt::alloc()
{
    free();

    m_data = new RtpPacket[m_capacity];
    for (int i = 0; i < m_capacity; ++i) {
        m_data[i].payload = new char[m_packetSize*5]; //should be 4, we put some extra space
    }
    m_silence = new char[m_packetSize*5];
    memset(m_silence, 0, m_packetSize*5);
}

void RtpBufferAlt::free()
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

bool RtpBufferAlt::isFull()
{
    // TODO
    return (m_last + 1) % m_capacity == m_first;
}

bool RtpBufferAlt::isEmpty()
{
    // TODO
    return m_last == m_first;
}
