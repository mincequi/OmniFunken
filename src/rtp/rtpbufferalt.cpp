#include "rtpbufferalt.h"

#include "rtppacket.h"

#include <util.h>
#include <airtunes/airtunesconstants.h>

#include <QtDebug>

namespace alt
{
RtpBuffer::RtpBuffer(uint framesPerPacket, uint latency) :
    m_framesPerPacket(framesPerPacket),
    m_latency(latency),
    m_desiredFill((airtunes::sampleRate*m_latency)/(m_framesPerPacket*1000)),
    m_capacity(Util::roundToPowerOfTwo(m_desiredFill*2)),
    m_first(0),
    m_last(0),
    m_data(NULL),
    m_silence(NULL)
{
    alloc();
}

RtpBuffer::~RtpBuffer()
{
    free();
}

RtpPacket* RtpBuffer::obtainPacket(quint16 sequenceNumber)
{
    RtpPacket* packet = &(m_data[sequenceNumber % m_capacity]);
    packet->sequenceNumber  = sequenceNumber;
    packet->status          = RtpPacket::PacketOk;

    return packet;
}

void RtpBuffer::commitPacket()
{
    quint16 fill = (m_data[m_last].sequenceNumber-m_data[m_first].sequenceNumber);
}

const RtpPacket* RtpBuffer::takePacket()
{
    RtpPacket* packet = &(m_data[m_first]);

    switch (packet->status) {
    case RtpPacket::PacketFree:
        qWarning()<<Q_FUNC_INFO<<": free packet: " << packet->sequenceNumber;
        packet = NULL;
        break;
    case RtpPacket::PacketMissing:
        qWarning()<<Q_FUNC_INFO<<": missing packet: " << packet->sequenceNumber;
        memcpy(packet->payload, m_silence, packet->payloadSize);
    case RtpPacket::PacketOk:
        m_first = (m_first+1) % m_capacity;
        packet->init();
        break;
    default:
        qFatal(Q_FUNC_INFO);
    }

    return packet;
}

void RtpBuffer::alloc()
{
    free();

    const uint bytesPerPacket = m_framesPerPacket*airtunes::channels*(airtunes::sampleSize/8);

    m_data = new RtpPacket[m_capacity];
    for (uint i = 0; i < m_capacity; ++i) {
        m_data[i].payload = new char[bytesPerPacket];
    }

    m_silence = new char[bytesPerPacket];
    memset(m_silence, 0, bytesPerPacket);
}

void RtpBuffer::free()
{
    if (m_data) {
        for (uint i = 0; i < m_capacity; ++i) {
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
} // namespace alt
