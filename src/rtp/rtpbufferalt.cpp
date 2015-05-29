#include "rtpbufferalt.h"

#include "rtpheader.h"
#include "rtppacket.h"

#include <util.h>
#include <airtunes/airtunesconstants.h>

#include <QtDebug>
#include <QThread>

RtpBuffer::RtpBuffer(uint framesPerPacket, uint latency, QObject *parent) :
    QObject(parent),
    m_framesPerPacket(framesPerPacket),
    m_latency(latency),
    m_desiredFill((airtunes::sampleRate*m_latency)/(m_framesPerPacket*1000)),
    m_capacity(Util::roundToPowerOfTwo(m_desiredFill*2)),
    m_data(NULL),
    m_silence(NULL),
    m_begin(0),
    m_end(0),
    m_ready(false)
{
    alloc();
}

RtpBuffer::~RtpBuffer()
{
    free();
}

RtpPacket* RtpBuffer::obtainPacket(const RtpHeader& rtpHeader)
{
    m_mutex.lock();

    // 1. if packet already arrived, discard
    // 2. if it is a regular packet, always sort it in
    // 2.1 start of stream
    // 2.2 expected
    // 2.3 early, but skipped < desired fill
    // 2.3 late, but in range -> mark missing
    // 2.4 late, not in range -> start of stream
    // 2.5 early
    // 3. if it is a retransmitted packet, check if it is in range

    // Fetch slot from buffer
    RtpPacket* packet = &(m_data[rtpHeader.sequenceNumber%m_capacity]);

    // Check if this packet is already received
    if ((packet->sequenceNumber == rtpHeader.sequenceNumber) && (packet->status == RtpPacket::PacketOk)) {
        qDebug()<<Q_FUNC_INFO<< "packet already received";
        m_mutex.unlock();
        return NULL;
    }

    // If new packet is VERY late, we clear our buffer
    if (abs(seqDiff(rtpHeader.sequenceNumber) >= m_desiredFill)) {
        qDebug()<<Q_FUNC_INFO<< "XXXXXXXXXXXXXXXXX CLEAR XXXXXXXXXXXXXXXXXX";
        clear();
    }

    // If our buffer is empty then this is the first packet.
    if (empty()) {
        m_begin = rtpHeader.sequenceNumber;
        m_end = rtpHeader.sequenceNumber;
    }
    // Else check if buffer is almost full and clip it.
    else if (size() >= (m_capacity-1)) {
        qWarning()<<Q_FUNC_INFO<< "buffer overflow, clipping front";
        m_begin = m_end-m_desiredFill;
    }

    if (packet) {
        packet->sequenceNumber  = rtpHeader.sequenceNumber;
    } else {
        m_mutex.unlock();
    }

    return packet;
}

void RtpBuffer::commitPacket(RtpPacket* packet)
{
    // Mark packet as ok
    packet->status = RtpPacket::PacketOk;

    // Our packet count is the diff between this seqNo and the previous one.
    qint16 skippedPackets = packet->sequenceNumber-m_end;
    if (skippedPackets > 0) {
        qWarning()<<Q_FUNC_INFO<< "missing packets, last: "<<m_end<<", this: "<<packet->sequenceNumber;
    }

    // Mark missing packets
    for (quint16 i = m_end; (i != packet->sequenceNumber); ++i) {
        m_data[i%m_capacity].status = RtpPacket::PacketMissing;
        m_data[i%m_capacity].sequenceNumber = i;
    }

    // End is one slot beyond last valid packet
    m_end = packet->sequenceNumber+1;

    // Dump buffer size
    static quint32 count = 0;
    if ((count%250) == 0) {
        qDebug()<<Q_FUNC_INFO<< "fill: "<<size();
    }
    ++count;

    // Wake player thread
    if (size() >= m_desiredFill) {
        m_ready = true;
        emit ready();
    }

    m_mutex.unlock();
}

const RtpPacket* RtpBuffer::takePacket()
{
    QMutexLocker locker(&m_mutex);

    if (!m_ready) return NULL;

    RtpPacket* packet = front();
    ++m_begin;

    switch (packet->status) {
    case RtpPacket::PacketFree:
        return NULL;
        break;
    case RtpPacket::PacketMissing:
        qWarning()<<Q_FUNC_INFO<< "missing packet: " << packet->sequenceNumber;
        memcpy(packet->payload, m_silence, packet->payloadSize);
    case RtpPacket::PacketOk:
        if (packet->flush) {
            qDebug()<<Q_FUNC_INFO<< "flush packet: "<<packet->sequenceNumber;
        }
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

RtpPacket* RtpBuffer::obtainRegularPacket(const RtpHeader& rtpHeader)
{
    // If new packet is VERY late, we clear our buffer
    if (abs(seqDiff(rtpHeader.sequenceNumber) >= m_desiredFill)) {
        qDebug()<<Q_FUNC_INFO<< "XXXXXXXXXXXXXXXXX CLEAR XXXXXXXXXXXXXXXXXX";
        clear();
    }

    // If our buffer is empty then this is the first packet.
    if (empty()) {
        m_begin = rtpHeader.sequenceNumber;
        m_end = rtpHeader.sequenceNumber;
    }
    // Else check if buffer is almost full and clip it.
    else if (size() >= (m_capacity-1)) {
        qWarning()<<Q_FUNC_INFO<< "buffer overflow, dropping packets";
        m_begin = m_end-(m_desiredFill);
    }
}

RtpPacket* RtpBuffer::obtainMissingPacket(const RtpHeader& rtpHeader)
{
    RtpPacket* packet = &(m_data[rtpHeader.sequenceNumber%m_capacity]);
    if ((packet->sequenceNumber == rtpHeader.sequenceNumber) &&
        (packet->status == RtpPacket::PacketMissing)) {
        qDebug()<<Q_FUNC_INFO<< "missing packet arrived: "<<rtpHeader.sequenceNumber;
        packet->status = RtpPacket::PacketOk;
        return packet;
    }
    qWarning()<<Q_FUNC_INFO<< "missing packet arrived too late: "<<rtpHeader.sequenceNumber;
    return NULL;
}

quint16 RtpBuffer::size()
{
    return m_end-m_begin-1;
}

RtpPacket* RtpBuffer::front() const
{
    return &(m_data[m_begin%m_capacity]);
}

bool RtpBuffer::empty() const
{
    return front()->status == RtpPacket::PacketFree;
}

qint16 RtpBuffer::seqDiff(quint16 sequenceNumber)
{
    return sequenceNumber-m_end;
}

void RtpBuffer::clear()
{
    for (quint16 i = 0; i < m_capacity; ++i) {
        m_data[i].init();
    }
    m_ready = false;
}

void RtpBuffer::syncTo(const RtpHeader& rtpHeader)
{
    m_begin = rtpHeader.sequenceNumber-1;
    m_end = rtpHeader.sequenceNumber;
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
