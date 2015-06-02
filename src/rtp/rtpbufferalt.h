#ifndef RTPBUFFER_H
#define RTPBUFFER_H

#include "rtpstat.h"

#include <QList>
#include <QObject>
#include <QSemaphore>
#include <QWaitCondition>

struct RtpHeader;
struct RtpPacket;

class RtpBuffer : public QObject
{
    Q_OBJECT
public:
    // framesPerPacket = stereo frames per second.
    RtpBuffer(uint framesPerPacket, uint latency = 500, QObject *parent = 0);
    ~RtpBuffer();

    // producer thread
    RtpPacket* obtainPacket(const RtpHeader& rtpHeader);
    void commitPacket(RtpPacket* packet);

    // consumer thread
    const RtpPacket* takePacket();

    // silence for missing packets
    void silence(char **silence, int *size) const;

    // get missing sequences
    struct Sequence {
        quint16 first;
        quint16 count;
    };
    QList<Sequence> missingSequences();

signals:
    void ready();

private:
    enum PacketRating {
        Start,      // This packet starts a new stream
        Duplicate,  // packet with this seqno has been sent twice
        Expected,   // packet is in expected order
        Early,      // packet is early, missing packets
        Late,       // packet is late (eventually from retransmit)
        TooLate,    // packet is too late (from retransmit)
    };
    PacketRating ratePacket(const RtpHeader& rtpHeader);

    // Buffer helpers
    quint16 size();
    void flush();

    // Memory
    void alloc();
    void free();

private:
    // General buffer settings and data
    const uint      m_framesPerPacket;
    const uint      m_latency;
    const int       m_desiredFill;
    const quint16   m_capacity;
    RtpPacket       *m_data;
    char            *m_silence;

    quint16     m_begin;
    quint16     m_end;
    QMutex      m_mutex;

    // Needed to stop consumer
    bool        m_ready;

    // Statistics
    RtpStat     m_stat;
};
#endif // RTPBUFFER_H
