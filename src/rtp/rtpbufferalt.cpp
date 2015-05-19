#include "rtpbuffer.h"

#include "rtppacket.h"

#include "util.h"

#include <QtDebug>

#if 0
RtpBuffer::RtpBuffer(int latency, QObject *parent) :
    m_packetSize(0)
{
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
}

const RtpPacket* RtpBuffer::takePacket()
{
    QMutexLocker locker(&m_mutex);

    if (m_status == Init || m_status == Filling) {
        return NULL;
    }

    // take from top, until free packet or flush
    RtpPacket* packet = &(m_data[m_first]);

    // we might catch a packet which is marked as flush but is not received yet
    if (packet->flush) {
        qDebug() << __func__ << ": flush packet: " << packet->sequenceNumber;
        m_timer.stop();
        m_status = Filling;
        packet->flush = false;
        return NULL;
    }

    switch (packet->status) {
    case RtpPacket::PacketFree:
        qWarning() << __func__ << ": free packet: " << packet->sequenceNumber;
        m_timer.stop();
        m_status = Init;
        return NULL;
        break;
    case RtpPacket::PacketMissing:
        qWarning() << __func__ << ": missing packet: " << packet->sequenceNumber;
        memcpy(packet->payload, m_silence, packet->payloadSize);
    case RtpPacket::PacketOk:
        if (m_first == m_last) {
            qDebug() << __func__ << ": buffer empty";
            m_timer.stop();
            m_status = Init;
        } else {
            m_first = (m_first+1) % m_capacity;
        }
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

void RtpBuffer::flush(quint16 sequenceNumber)
{
    QMutexLocker locker(&m_mutex);

    // if flush
    if (m_status == Ready) {
        qDebug() << __func__ << ": flush at: " << sequenceNumber;
        m_timer.stop();
        m_status = Flushing;

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

    m_timer.stop();
    m_status = Init;
    m_first = 0;
    m_last = 0;

    for (int i = 0; i < m_capacity; ++i) {
        m_data[i].init();
    }
}

void RtpBuffer::timeout()
{
    m_mutex.lock();
    quint16 size = m_data[m_last].sequenceNumber-m_data[m_first].sequenceNumber;
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

void RtpBuffer::setStatus(Status status)
{
    if (m_status == status) {
        return;
    }

    if (status == Ready) {
        m_timer.start(1000);
    } else {
        m_timer.stop();
    }

    m_status = status;
}

RtpBuffer::PacketOrder RtpBuffer::orderPacket(quint16 sequenceNumber)
{
    if (m_status == Init) {
        m_first = sequenceNumber % m_capacity;
        return Expected;
    } else {
        // compute sequence diff from previous packet
        qint16 firstDiff = m_data[m_first].sequenceNumber-sequenceNumber; // shall be negative
        qint16 diff = sequenceNumber-m_data[m_last].sequenceNumber;

        // if packet is too late. this should happen rarely.
        if (firstDiff > 0) {
            qDebug() << __func__ << ": packet too late: " << sequenceNumber;
            return Discard;
        } else if (diff == 1) {
            return Expected;
        } else if (diff > 1) {
            qDebug() << __func__ << ": early packet/lost packets";
            // mark missing
            for (quint16 i = m_data[m_last].sequenceNumber+1; i != sequenceNumber; ++i) {
                m_data[i%m_capacity].status = RtpPacket::PacketMissing;
                m_data[i%m_capacity].sequenceNumber = i;
            }
            return Early;
        } else if (diff == 0) {
            qWarning() << __func__ << ": packet sent twice: " << sequenceNumber;
            // TODO memcmp packets
            return Twice;
        } else /*if (diff < 0)*/ {
            qDebug() << __func__ << ": late packet: " << sequenceNumber << ", last: " << m_data[m_last].sequenceNumber;
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
        if (m_data[i].status == RtpPacket::PacketMissing && startOfSequence == -1) {
            startOfSequence = m_data[i].sequenceNumber;
        }
        // find endOfSequence of missing packets
        if ((m_data[(i+1)%m_capacity].status != RtpPacket::PacketMissing) && startOfSequence != -1) {
            endOfSequence = m_data[i].sequenceNumber;

            // request every fifth time
            if (((m_data[startOfSequence%m_capacity].requestCount)%5)==0) {
                qDebug("requestMissingPackets: %d to %d", quint16(startOfSequence), quint16(endOfSequence));
                emit missingSequence(startOfSequence, (endOfSequence-startOfSequence+1));
            }

            // increment counter
            for (quint16 j = startOfSequence; j != endOfSequence+1; ++j) {
                m_data[j%m_capacity].requestCount++;
            }

            startOfSequence = -1;
            endOfSequence   = -1;
        }
    }
    if (startOfSequence != -1 || endOfSequence != -1) {
        qFatal(__func__);
    }
}
#endif
