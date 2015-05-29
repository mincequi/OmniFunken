#ifndef RTPBUFFER_H
#define RTPBUFFER_H

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

signals:
    void ready();

private:
    RtpPacket* obtainRegularPacket(const RtpHeader& rtpHeader);
    RtpPacket* obtainMissingPacket(const RtpHeader& rtpHeader);

    // size, fill level
    quint16 size();
    RtpPacket* front() const;
    bool empty() const;
    qint16 seqDiff(quint16 sequenceNumber);
    void clear();

    void syncTo(const RtpHeader& rtpHeader);

    void alloc();
    void free();

private:
    const uint      m_framesPerPacket;
    const uint      m_latency;
    const int       m_desiredFill;
    const quint16   m_capacity;
    RtpPacket       *m_data;
    char            *m_silence;

    QMutex      m_mutex;
    quint16     m_begin;    // owned by consumer thread
    quint16     m_end;     // owned by producer thread

    bool        m_ready;
};
#endif // RTPBUFFER_H
