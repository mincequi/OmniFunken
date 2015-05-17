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

    enum State {
        Empty,
        Filling,
        Ready,
        Flushing
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
    void stateChanged(RtpBuffer::State state);

public slots:
    void flush(quint16 sequenceNumber);
    void teardown();

private:
    void alloc();
    void free();

    void setState(State state);

    enum PacketOrder {
        TooLate,    // packet is too late
        Early,      // missing packets
        Expected,   // packet is in expected order
        Late,       // packet is late (eventually from request resend)
        Twice       // packet with this seqno has been sent twice
    };
    PacketOrder orderPacket(quint16);

    void emitStateChanged(RtpBuffer::State state);

private:
    State       m_state;
    State       m_lastEmittedState;

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

Q_DECLARE_METATYPE(RtpBuffer::State)

#endif // RTPBUFFER_H
