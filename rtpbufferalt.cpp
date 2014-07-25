#include "rtpbufferalt.h"

#include <QtDebug>

RtpBuffer::RtpBuffer(int latency, QObject *parent) :
    QObject(parent),
    m_status(Init),
    m_latency(latency),
    m_first(0),
    m_last(0),
    m_data(NULL),
    m_silence(NULL),
    m_packetSize(0),
    m_initSequenceNumber(-1)
{
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
}

RtpBuffer::~RtpBuffer()
{
    free();
}

void RtpBuffer::setPacketSize(int frames)
{
    QMutexLocker locker(&m_mutex);

    m_packetSize = frames;
    m_capacity = 2*(44100*m_latency)/(m_packetSize*1000);
    alloc();
}

RtpBuffer::RtpPacket* RtpBuffer::obtainPacket(quint16 sequenceNumber)
{
    // producer wants to put a packet
    m_mutex.lock();

    requestMissingPackets();

    switch (orderPacket(sequenceNumber)) {
    case Twice:
    case Discard:
        m_mutex.unlock();
        return NULL;
        break;
    case Early:
    case Expected:
        m_last = sequenceNumber % m_capacity;
        // if packet was rendered as ok before
        if (m_data[m_last].status == PacketOk && m_status != Init) {
            qDebug() << __func__ << ": buffer full, overwriting samples";
        }
    case Late:
        if (m_status == Init) {
            qDebug() << __func__ << ": buffer filling";
            m_status = Filling;
        }
        break;
    default:
        qFatal(__func__, ": illegal packet order status");
    }

    RtpPacket* packet = &(m_data[sequenceNumber % m_capacity]);
    packet->sequenceNumber  = sequenceNumber;
    packet->status          = PacketOk;

    return packet;
}

void RtpBuffer::commitPacket()
{
    quint16 size = (m_data[m_last].sequenceNumber-m_data[m_first].sequenceNumber);
    if ((m_status == Filling) && (size >= (44100*m_latency)/(m_packetSize*1000))) {
        qDebug() << __func__ << ": buffer size is " << size << "start playing";
        m_status = Ready;
        m_timer.start(1000);
        m_mutex.unlock();
        emit ready();
        return;
    } else {
        m_mutex.unlock();
    }
}

const RtpBuffer::RtpPacket* RtpBuffer::takePacket()
{
    QMutexLocker locker(&m_mutex);

    // take from top, until free packet or flush
    RtpPacket* packet = &(m_data[m_first]);

    if (packet->flush) {
        qDebug() << __func__ << ": flush packet: " << packet->sequenceNumber;
        m_timer.stop();
        m_status = Filling;
        packet->flush = false;
        return NULL;
    }

    switch (packet->status) {
    case PacketFree:
        qDebug() << __func__ << ": free packet: " << packet->sequenceNumber;
        m_timer.stop();
        m_status = Init;
        return NULL;
        break;
    case PacketMissing:
        qCritical() << __func__ << ": missing packet: ", packet->sequenceNumber;
        memcpy(packet->payload, m_silence, packet->payloadSize);
    case PacketOk:
        m_first = (m_first+1) % m_capacity;
        packet->init();
        break;
    default:
        qFatal(__func__);
        return NULL;
    }

    return packet;
}

void RtpBuffer::silence(char **silence, int *size) const
{
    *silence = m_silence;
    *size   = m_packetSize*4;
}

void RtpBuffer::init(quint16 sequenceNumber)
{
    QMutexLocker locker(&m_mutex);

    // if real init
    if (m_status == Init) {
        qDebug() << __func__ << ": init: " << sequenceNumber;
        m_initSequenceNumber = sequenceNumber;
    // or flush
    } else {
        qDebug() << __func__ << ": flush: " << sequenceNumber;
        m_data[sequenceNumber%m_capacity].flush = true;
    }
}

void RtpBuffer::teardown()
{
    QMutexLocker locker(&m_mutex);

    m_status = Init;
    m_first = 0;
    m_last = 0;
    m_initSequenceNumber = -1;

    for (int i = 0; i < m_capacity; ++i) {
        m_data[i].init();
    }
}

void RtpBuffer::timeout()
{
    m_mutex.lock();
    quint16 size = m_data[m_last].sequenceNumber-m_data[m_first].sequenceNumber;
    m_mutex.unlock();
    //emit notify(size);
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

RtpBuffer::PacketOrder RtpBuffer::orderPacket(quint16 sequenceNumber)
{
    if (m_status == Init) {
        qint16 initDiff = sequenceNumber-m_initSequenceNumber;
        if (initDiff == 0) {
            qDebug() << __func__ << ": expected init packet received";
            m_first = m_initSequenceNumber % m_capacity;
            m_data[m_first].sequenceNumber   = m_initSequenceNumber;
            m_data[m_first].status           = PacketOk;
            return Expected;
        } else if (initDiff > 0) {
            qDebug() << __func__ << ": early init packet/lost packets";
            m_first = m_initSequenceNumber % m_capacity;
            m_data[m_first].sequenceNumber   = m_initSequenceNumber;
            // mark missing
            for (quint16 i = m_data[m_first].sequenceNumber; i != sequenceNumber; ++i) {
                m_data[i%m_capacity].status = PacketMissing;
                m_data[i%m_capacity].sequenceNumber = i;
            }
            return Early;
        } else {
            qCritical() << __func__ << ": unexpected init packet " << sequenceNumber << ", shall be " << m_initSequenceNumber;
            return Discard;
        }
    } else {
        // compute sequence diff from previous packet
        qint16 firstDiff = m_data[m_first].sequenceNumber-sequenceNumber; // shall be negative
        qint16 diff = sequenceNumber-m_data[m_last].sequenceNumber;

        // if packet is too late. this should happen rarely.
        if (firstDiff > 0) {
            qCritical() << __func__ << ": packet too late: " << sequenceNumber;
            return Discard;
        } else if (diff == 1) {
            return Expected;
        } else if (diff > 1) {
            qDebug() << __func__ << ": early packet/lost packets";
            // mark missing
            for (quint16 i = m_data[m_last].sequenceNumber+1; i != sequenceNumber; ++i) {
                m_data[i%m_capacity].status = PacketMissing;
                m_data[i%m_capacity].sequenceNumber = i;
            }
            return Early;
        } else if (diff == 0) {
            qWarning() << __func__ << ": packet sent twice: " << sequenceNumber;
            // TODO memcmp packets
            return Twice;
        } else /*if (diff < 0)*/ {
            qDebug() << __func__ << ": late packet/eventually from retransmit";
            return Late;
        }
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

            // increment counter
            for (quint16 j = startOfSequence; j != endOfSequence+1; ++j) {
                m_data[j].requestCount++;
            }

            startOfSequence = -1;
            endOfSequence   = -1;
        }
    }
    if (startOfSequence != -1 || endOfSequence != -1) {
        qFatal(__func__, "impossible");
    }
}
