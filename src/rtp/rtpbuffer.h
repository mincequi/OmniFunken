#ifndef RTPBUFFER_H
#define RTPBUFFER_H

#include <QList>
#include <QMutex>
#include <QObject>

struct RtpPacket;

class RtpBuffer : public QObject
{
    Q_OBJECT
public:
    struct Sequence {
        quint16 first;
        quint16 count;
    };

    // framesPerPacket = stereo frames per second.
    RtpBuffer(uint framesPerPacket, uint latency = 500, QObject *parent = 0);
    ~RtpBuffer();

    // producer thread
    RtpPacket* obtainPacket(quint16 sequenceNumber);
    void commitPacket();
    // consumer thread
    const RtpPacket* takePacket();

    // size, fill level
    quint16 size() const;

    // get missing sequences
    QList<Sequence> missingSequences() const;

    void silence(char **silence, int *size) const;

signals:
    void ready();

public slots:
    void flush(quint16 sequenceNumber);
    void teardown();

private:
    enum Status {
        Init,
        Filling,
        Ready,
        Flushing
    };

    void alloc();
    void free();

    void setStatus(Status status);

    enum PacketOrder {
        TooLate,    // packet is too late
        Early,      // missing packets
        Expected,   // packet is in expected order
        Late,       // packet is late (eventually from request resend)
        Twice       // packet with this seqno has been sent twice
    };
    PacketOrder orderPacket(quint16);

private:
    Status      m_status;

    const uint  m_framesPerPacket;
    const uint  m_latency;
    int         m_desiredFill;
    int         m_capacity;
    int         m_first;
    int         m_last;
    RtpPacket   *m_data;
    char        *m_silence;

    mutable QMutex  m_mutex;
};

#endif // RTPBUFFER_H
