#ifndef RTPBUFFERALT_H
#define RTPBUFFERALT_H

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
            requestCount(0),
            payloadSize(0),
            payload(NULL) {}
        void init() {
            status = PacketFree;
            requestCount = 0;
        }

        quint16         sequenceNumber;
        RtpPacketStatus status;
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
    void init(quint16 seq);

private slots:
    void timeout();

private:
    void alloc();
    void free();

    void requestMissingPackets();

    const int   m_latency;
    int         m_capacity;
    int         m_first;
    int         m_last;
    bool        m_ready;
    RtpPacket   *m_data;
    char        *m_silence;
    int         m_packetSize;
    int         m_initSeq;

    mutable QMutex  m_mutex;
    QTimer          m_timer;
};

#endif // RTPBUFFERALT_H
