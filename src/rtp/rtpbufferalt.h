#ifndef RTPBUFFER_H
#define RTPBUFFER_H

#include <QList>
#include <QObject>
#include <QSemaphore>
#include <QWaitCondition>

struct RtpPacket;

class RtpBuffer
{
public:
    // framesPerPacket = stereo frames per second.
    RtpBuffer(uint framesPerPacket, uint latency = 500);
    ~RtpBuffer();

    // producer thread
    RtpPacket* obtainPacket(quint16 sequenceNumber);
    void commitPacket(RtpPacket* packet);

    // consumer thread
    void waitUntilReady();
    const RtpPacket* takePacket();

    // silence for missing packets
    void silence(char **silence, int *size) const;

private:
    void alloc();
    void free();

private:
    const uint      m_framesPerPacket;
    const uint      m_latency;
    const int       m_desiredFill;
    const quint16   m_capacity;
    RtpPacket       *m_data;
    char            *m_silence;

    QSemaphore      m_fill;
    QWaitCondition  m_ready;
    QMutex          m_readyMutex;

    quint16     m_first;
    quint16     m_last;
};
#endif // RTPBUFFER_H
