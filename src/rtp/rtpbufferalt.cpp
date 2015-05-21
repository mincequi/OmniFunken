#include "rtpbufferalt.h"

#include "rtppacket.h"

#include <util.h>
#include <airtunes/airtunesconstants.h>

#include <QtDebug>
#include <QThread>

RtpBuffer::RtpBuffer(uint framesPerPacket, uint latency) :
    m_framesPerPacket(framesPerPacket),
    m_latency(latency),
    m_desiredFill((airtunes::sampleRate*m_latency)/(m_framesPerPacket*1000)),
    m_capacity(Util::roundToPowerOfTwo(m_desiredFill*2)),
    m_data(NULL),
    m_silence(NULL),
    m_fill(),
    m_first(0),
    m_last(0)
{
    alloc();
}

RtpBuffer::~RtpBuffer()
{
    free();
}

RtpPacket* RtpBuffer::obtainPacket(quint16 sequenceNumber)
{
    // We save anything for now
    RtpPacket* packet = &(m_data[sequenceNumber%m_capacity]);
    packet->sequenceNumber  = sequenceNumber;

    return packet;
}

void RtpBuffer::commitPacket(RtpPacket* packet)
{
    // Mark packet as ok
    packet->status = RtpPacket::PacketOk;

    // If our buffer is empty this is the first packet (and our last one).
    if (m_fill.available() == 0) {
        m_first = packet->sequenceNumber%m_capacity;
        m_last = packet->sequenceNumber%m_capacity;
        m_fill.release(1);
    }

    // Our packet count is the diff between this seqNo and the previous one.
    quint16 newPackets = packet->sequenceNumber-m_data[m_last].sequenceNumber;

    // Mark missing packets
    for (quint16 i = m_data[m_last].sequenceNumber+1; (i != packet->sequenceNumber) && (m_first != m_last); ++i) {
        m_data[i%m_capacity].status = RtpPacket::PacketMissing;
        m_data[i%m_capacity].sequenceNumber = i;
    }

    // Our last packet is always this one for now
    m_last = packet->sequenceNumber%m_capacity;

    // We have new packets
    m_fill.release(newPackets);

    // Dump buffer size
    static quint32 count = 0;
    if ((count%125) == 0) {
        qDebug()<<Q_FUNC_INFO<< "fill: "<<m_fill.available();
    }
    ++count;

    // Wake player thread
    if (m_fill.available() >= m_desiredFill) {
        m_ready.wakeAll();
    }
}

void RtpBuffer::waitUntilReady()
{
    //QThread::msleep(500);
    m_readyMutex.lock();
    m_ready.wait(&m_readyMutex);
    m_readyMutex.unlock();
}

const RtpPacket* RtpBuffer::takePacket()
{
    if (!m_fill.tryAcquire(1, m_latency)) {
        return NULL;
    }

    RtpPacket* packet = &(m_data[m_first]);

    switch (packet->status) {
    case RtpPacket::PacketFree:
        qFatal(Q_FUNC_INFO);
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

void RtpBuffer::silence(char **silence, int *size) const
{
    *silence = m_silence;
    *size   = m_framesPerPacket*airtunes::channels*(airtunes::sampleSize/8);
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
