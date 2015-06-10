#include "rtpbuffer.h"

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
    m_ready(false),
    m_stat(),
    m_lastPlayed(0)
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

    RtpPacket* packet = NULL;

    switch (ratePacket(rtpHeader)) {
    case Start:
        m_begin = rtpHeader.sequenceNumber;
        m_end = rtpHeader.sequenceNumber+1;
        break;
    case Early:
        // Mark missing packets
        for (quint16 i = m_end; i != rtpHeader.sequenceNumber; ++i) {
            m_data[i%m_capacity].status = RtpPacket::PacketMissing;
            m_data[i%m_capacity].sequenceNumber = i;
        }
    case Expected:
        //  Check for buffer overflow.
        if (size() >= (m_capacity-1)) {
            qWarning()<<Q_FUNC_INFO<< "buffer overflow, clipping front";
            m_begin = m_end-m_desiredFill;
            m_stat.overflow++;
        }
        // End is one slot beyond last valid packet
        m_end = rtpHeader.sequenceNumber+1;
        break;
    case Late:
        break;
    case Duplicate:
    case TooLate:
    default:
        m_mutex.unlock();
        return NULL;
        break;
    }

    // Fetch slot from buffer
    packet = &(m_data[rtpHeader.sequenceNumber%m_capacity]);
    packet->sequenceNumber  = rtpHeader.sequenceNumber;

    return packet;
}

void RtpBuffer::commitPacket(RtpPacket* packet)
{
    // Mark packet as ok
    packet->status = RtpPacket::PacketOk;

    // Dump buffer size
    static quint32 count = 0;
    if ((count%250) == 0) {
        qDebug()<<Q_FUNC_INFO<<"fill:"<<size();
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

    // If we flushed the buffer before, we return null until buffer is ready again
    if (!m_ready) return NULL;

    RtpPacket* packet = &(m_data[m_begin%m_capacity]);
    ++m_begin;

    switch (packet->status) {
    case RtpPacket::PacketFree:
        qWarning()<<Q_FUNC_INFO<<"last packet:"<<m_lastPlayed;
        return NULL;
        break;
    case RtpPacket::PacketMissing:
        qWarning()<<Q_FUNC_INFO<<"missing packet:"<<packet->sequenceNumber;
        memcpy(packet->payload, m_silence, packet->payloadSize);
    case RtpPacket::PacketOk:
        if (packet->flush) {
            qDebug()<<Q_FUNC_INFO<<"flush packet:"<<packet->sequenceNumber;
        }
        m_lastPlayed = packet->sequenceNumber;
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

QList<RtpBuffer::Sequence> RtpBuffer::missingSequences() const
{
    QMutexLocker locker(&m_mutex);

    QList<RtpBuffer::Sequence> missingSequences;
    int startOfSequence = -1;
    int endOfSequence   = -1;

    for (int i = m_begin%m_capacity; i != m_end%m_capacity; i = (i+1)%m_capacity) {
        // find startOfSequence of missing packets
        if (m_data[i].status == RtpPacket::PacketMissing && startOfSequence == -1) {
            startOfSequence = m_data[i].sequenceNumber;
        }
        // find endOfSequence of missing packets
        if ((m_data[(i+1)%m_capacity].status != RtpPacket::PacketMissing) && startOfSequence != -1) {
            endOfSequence = m_data[i].sequenceNumber;

            missingSequences.push_back( { static_cast<quint16>(startOfSequence), static_cast<quint16>(endOfSequence-startOfSequence+1) } );

            startOfSequence = -1;
            endOfSequence   = -1;
        }
    }

    return missingSequences;
}

RtpBuffer::PacketRating RtpBuffer::ratePacket(const RtpHeader& rtpHeader)
{
    m_stat.put++;

    // 1. Check for duplicate (PacketStatus == ok)
    // 2. Regular packet (always take it)
    // 2.1 Start (new stream) (m_begin == empty)
    // 2.2 seqDiff: –32.768 to 32.767
    // 2.2.1 early (0 < seqDiff < desiredFill(63))
    // 2.2.2 expected (seqDiff == 0)
    // 2.2.3 late (seqDiff < 0 && PacketStatus == missing) // also handle retransmission here
    // 2.2.4 start (new stream here, 'cause (seqDiff > desiredFill || seqDiff < 0)
    // 3. Retransmitted packet (m_begin <= seqNo < m_end)

    // Fetch slot from buffer
    RtpPacket* packet = &(m_data[rtpHeader.sequenceNumber%m_capacity]);

    // 1. Check for duplicate
    if ((packet->sequenceNumber == rtpHeader.sequenceNumber) && (packet->status == RtpPacket::PacketOk)) {
        qDebug()<<Q_FUNC_INFO<< "packet already received:"<<packet->sequenceNumber;
        m_stat.duplicates++;
        return Duplicate;
    }

    // 2.1 New stream if empty
    if (m_data[m_begin%m_capacity].status == RtpPacket::PacketFree) {
        qDebug()<<Q_FUNC_INFO<< "new stream:"<<rtpHeader.sequenceNumber;
        return Start;
    }

    // 2.2 Interpret seqDiff
    qint16 seqDiff = rtpHeader.sequenceNumber-m_end;
    // 2.2.1 Early (0 < seqDiff < desiredFill(63))
    if ((seqDiff > 0) && (seqDiff < m_desiredFill)) {
        qDebug()<<Q_FUNC_INFO<< "missing packets:"<<m_end<<"-"<<(rtpHeader.sequenceNumber-1);
        m_stat.early++;
        return Early;
    }
    // 2.2.2 Expected (seqDiff == 0)
    if (seqDiff == 0) {
        return Expected;
    }
    // 2.2.3 late (seqDiff < 0 && PacketStatus == missing) // also handle retransmission here
    if ((packet->sequenceNumber == rtpHeader.sequenceNumber) && (packet->status == RtpPacket::PacketMissing)) {
        //qDebug()<<Q_FUNC_INFO<< "late packet:"<<packet->sequenceNumber;
        m_stat.late++;
        return Late;
    }
    // 3. Retransmitted packet
    if ((rtpHeader.payloadType == airtunes::RetransmitResponse) /*&& (packet->status != RtpPacket::PacketMissing)*/) {
        qDebug()<<Q_FUNC_INFO<< "too late packet:"<<rtpHeader.sequenceNumber;
        m_stat.lost++;
        return TooLate;
    }

    qDebug()<<Q_FUNC_INFO<< "flush buffer, end:"<<m_end<<", this:"<<rtpHeader.sequenceNumber;
    flush();
    return RtpBuffer::Start;
}

quint16 RtpBuffer::size()
{
    return m_end-m_begin-1;
}

void RtpBuffer::flush()
{
    for (quint16 i = 0; i < m_capacity; ++i) {
        m_data[i].init();
    }
    m_ready = false;
    m_stat.flush++;
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
