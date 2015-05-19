#ifndef RTPBUFFER_H
#define RTPBUFFER_H

#include <QList>
#include <QObject>

struct RtpPacket;

namespace alt
{
class RtpBuffer
{
public:
    // framesPerPacket = stereo frames per second.
    RtpBuffer(uint framesPerPacket, uint latency = 500);
    ~RtpBuffer();

    // producer thread
    RtpPacket* obtainPacket(quint16 sequenceNumber);
    void commitPacket();
    // consumer thread
    const RtpPacket* takePacket();

private:
    void alloc();
    void free();

private:
    const uint  m_framesPerPacket;
    const uint  m_latency;
    const uint  m_desiredFill;
    const uint  m_capacity;
    int         m_first;
    int         m_last;
    RtpPacket   *m_data;
    char        *m_silence;
};
} // namespace alt
#endif // RTPBUFFER_H
