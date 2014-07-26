#ifndef RTPBUFFER_H
#define RTPBUFFER_H

#include <QObject>
#include <QTimer>

class RtpBuffer : public QObject
{
    Q_OBJECT
public:
    enum RtpPacketStatus {
        PacketFree,
        PacketOk,
        PacketMissing
    };

    struct RtpPacket {
        RtpPacket() :
            sequenceNumber(0),
            status(PacketFree),
            flush(false),
            requestCount(0),
            payloadSize(0),
            payload(NULL) {}
        void init() {
            sequenceNumber = 0;
            status = PacketFree;
            flush = false;
            requestCount = 0;
        }

        quint16         sequenceNumber;
        RtpPacketStatus status;
        bool            flush;
        quint16         requestCount;
        int             payloadSize;
        char            *payload;
    };

    explicit RtpBuffer(int latency = 500, QObject *parent = 0);
    ~RtpBuffer();
    // packetSize (bytes per frame = packetSize * numChannels * numBitsPerChannel)
    void setPacketSize(int frames);

    // producer thread
    RtpPacket* obtainPacket(quint16 sequenceNumber);
    void commitPacket();
    // consumer thread
    const RtpPacket* takePacket();

    void silence(char **silence, int *size) const;

signals:
    void ready();
    void full();
    void missingSequence(quint16 first, quint16 num);
    void notify(quint16 size);

public slots:
    void flush(quint16 sequenceNumber);
    void teardown();

private slots:
    void timeout();

private:
    void alloc();
    void free();

    enum PacketOrder {
        Discard,    // packet is too late
        Early,      // missing packets
        Expected,   // packet is in expected order
        Late,       // packet is late (eventually from request resend)
        Twice       // packet with this seqno has been sent twice
    };
    PacketOrder orderPacket(quint16);

    void requestMissingPackets();

    enum Status {
        Init,
        Filling,
        Ready,
        Flushing
    };
    Status      m_status;

    const int   m_latency;
    int         m_desiredFill;
    int         m_capacity;
    int         m_first;
    int         m_last;
    RtpPacket   *m_data;
    char        *m_silence;
    int         m_packetSize;

    mutable QMutex  m_mutex;
    QTimer          m_timer;
};

#endif // RTPBUFFER_H
