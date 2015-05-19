#include "rtpbuffer.h"

#include "rtppacket.h"
#include "util.h"
#include <QtDebug>

RtpBuffer::RtpBuffer(uint framesPerPacket, uint latency, QObject *parent) :
    QObject(parent),
    m_state(Empty),
    m_lastEmittedState(Empty),
    m_framesPerPacket(framesPerPacket),
    m_latency(latency),
    m_desiredFill(0),
    m_first(0),
    m_last(0),
    m_data(NULL),
    m_silence(NULL)
{
    qRegisterMetaType<RtpBuffer::State>();

    m_desiredFill = (44100*m_latency)/(m_framesPerPacket*1000);
    m_capacity = Util::roundToPowerOfTwo(m_desiredFill*2);
    alloc();
}

RtpBuffer::~RtpBuffer()
{
    free();
}

RtpPacket* RtpBuffer::obtainPacket(quint16 sequenceNumber)
{
    // producer wants to put a packet
    m_mutex.lock();

    switch (orderPacket(sequenceNumber)) {
    case Twice:
    case TooLate:
        return NULL;
        break;
    case Early:
    case Expected:
        m_last = sequenceNumber % m_capacity;
    case Late:
        if (m_state == Empty) {
            qDebug()<<Q_FUNC_INFO<<": buffer now filling";
            setState(Filling);
        }
        if (m_data[sequenceNumber % m_capacity].status == RtpPacket::PacketOk) {
            qDebug()<<Q_FUNC_INFO<< "packet already received";
            return NULL;
        }
        break;
    default:
        qFatal(__func__);
    }

    RtpPacket* packet = &(m_data[sequenceNumber % m_capacity]);
    packet->sequenceNumber  = sequenceNumber;
    packet->status          = RtpPacket::PacketOk;

    return packet;
}

void RtpBuffer::commitPacket()
{
    quint16 fill = (m_data[m_last].sequenceNumber-m_data[m_first].sequenceNumber);
    if ((m_state == Filling) && (fill >= m_desiredFill)) {
        qDebug()<<Q_FUNC_INFO<<": start playing at: " << m_data[m_first].sequenceNumber << ", last: " << m_last << ": " << m_data[m_last].sequenceNumber;
        setState(Ready);
        m_mutex.unlock();
        emitStateChanged(Ready);
        emit ready();
        return;
    } else {
        m_mutex.unlock();
    }
}

const RtpPacket* RtpBuffer::takePacket()
{
    RtpPacket* packet = NULL;

    m_mutex.lock();
    if (m_state == Ready || m_state == Flushing) {
        // take from top, until free packet or flush
        packet = &(m_data[m_first]);

        // we might catch a packet which is marked as flush but is not received yet
        if (packet->flush) {
            qDebug()<<Q_FUNC_INFO<<": flush packet: " << packet->sequenceNumber;
            setState(Filling);
            packet->flush = false;
        } else {
            switch (packet->status) {
            case RtpPacket::PacketFree:
                qWarning()<<Q_FUNC_INFO<<": free packet: " << packet->sequenceNumber;
                setState(Empty);
                packet = NULL;
                break;
            case RtpPacket::PacketMissing:
                qWarning()<<Q_FUNC_INFO<< "missing packet: "<<packet->sequenceNumber<<", payload size: "<<packet->payloadSize;
                memcpy(packet->payload, m_silence, packet->payloadSize);
            case RtpPacket::PacketOk:
                if (m_first == m_last) {
                    qDebug()<<Q_FUNC_INFO<<": buffer empty";
                    setState(Empty);
                } else {
                    m_first = (m_first+1) % m_capacity;
                }
                packet->init();
                break;
            default:
                qFatal(__func__);
                packet = NULL;
            }
        }
    }
    State currentState = m_state;
    m_mutex.unlock();

    emitStateChanged(currentState);

    return packet;
}

quint16 RtpBuffer::size() const
{
    QMutexLocker locker(&m_mutex);

    return m_data[m_last].sequenceNumber-m_data[m_first].sequenceNumber;
}

QList<RtpBuffer::Sequence> RtpBuffer::missingSequences() const
{
    QList<RtpBuffer::Sequence> missingSequences;

    int startOfSequence = -1;
    int endOfSequence   = -1;

    m_mutex.lock();
    for (int i = m_first; i != m_last; i = (i+1)%m_capacity) {
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
    m_mutex.unlock();

    return missingSequences;
}

void RtpBuffer::silence(char **silence, int *size) const
{
    *silence = m_silence;
    *size   = m_framesPerPacket*4;
}

void RtpBuffer::flush(quint16 sequenceNumber)
{
    QMutexLocker locker(&m_mutex);

    // if flush
    if (m_state == Ready) {
        qDebug()<<Q_FUNC_INFO<<": flush at: " << sequenceNumber;
        setState(Flushing);

        RtpPacket* packet = &(m_data[sequenceNumber%m_capacity]);
        packet->sequenceNumber = sequenceNumber;
        packet->flush = true;
        if (packet->status != RtpPacket::PacketOk) {
            packet->status = RtpPacket::PacketMissing;
        }
    }
}

void RtpBuffer::teardown()
{
    QMutexLocker locker(&m_mutex);

    setState(Empty);
    m_first = 0;
    m_last = 0;

    for (int i = 0; i < m_capacity; ++i) {
        m_data[i].init();
    }
}

void RtpBuffer::alloc()
{
    free();

    m_data = new RtpPacket[m_capacity];
    for (int i = 0; i < m_capacity; ++i) {
        m_data[i].payload = new char[m_framesPerPacket*5]; //should be 4, we put some extra space
    }

    m_silence = new char[m_framesPerPacket*5];
    memset(m_silence, 0, m_framesPerPacket*5);
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

void RtpBuffer::setState(State state)
{
    if (m_state == state) {
        return;
    }

    m_state = state;
}

RtpBuffer::PacketOrder RtpBuffer::orderPacket(quint16 sequenceNumber)
{
    // Check for initial packet
    if (m_state == Empty) {
        m_first = sequenceNumber % m_capacity;
        return Expected;
    }

    // compute sequence diff from previous packet
    qint16 firstDiff = m_data[m_first].sequenceNumber-sequenceNumber; // shall be negative
    qint16 diff = sequenceNumber-m_data[m_last].sequenceNumber;

    // if packet is too late. this should happen rarely.
    if (firstDiff > 0) {
        qDebug()<<Q_FUNC_INFO<<": packet too late: " << sequenceNumber;
        return TooLate;
    } else if (diff == 1) {
        return Expected;
    } else if (diff > 1) {
        //qDebug()<<Q_FUNC_INFO<<": early packet/lost packets";
        // mark missing
        for (quint16 i = m_data[m_last].sequenceNumber+1; i != sequenceNumber; ++i) {
            m_data[i%m_capacity].status = RtpPacket::PacketMissing;
            m_data[i%m_capacity].sequenceNumber = i;
        }
        return Early;
    } else if (diff == 0) {
        //qDebug()<<Q_FUNC_INFO<<": packet sent twice: " << sequenceNumber;
        // TODO memcmp packets
        return Twice;
    } else /*if (diff < 0)*/ {
        //qDebug()<<Q_FUNC_INFO<<": late packet: " << sequenceNumber << ", last: " << m_data[m_last].sequenceNumber;
        return Late;
    }
}

void RtpBuffer::emitStateChanged(RtpBuffer::State state)
{
    if (m_lastEmittedState != state) {
        m_lastEmittedState = state;
        emit stateChanged(state);
    }
}
