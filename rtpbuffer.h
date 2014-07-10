#ifndef RTPBUFFER_H
#define RTPBUFFER_H

#include <QObject>
#include <QQueue>
#include <QPair>

// need intrusive, (avoid copying)
// thread safe, (producer-, consumer-thread)
// circular buffer (fixed memory size)

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
        quint16         sequenceNumber;
        RtpPacketStatus status;
        quint16         requestCount;
        int             payloadSize;
        char*           payload;
    };

    explicit RtpBuffer(int latency = 500, QObject *parent = 0);

    void reset();

    RtpPacket* putPacket(quint16 index);
    const RtpPacket* takePacket();

    // fill

    // packetSize (bytes per frame = packetSize * numChannels * numBitsPerChannel)
    void setPacketSize(uint frames);


signals:
    void ready();
    void missingSequence(quint16 first, quint16 num);

public slots:

private:
    void alloc();
    void free();

    void checkPacketOrder(quint16 index);
    void requestMissingPackets();

    bool m_init;
    bool m_ready;

    int m_latency;
    int m_capacity;
    int m_size;
    int m_first;
    int m_last;
    quint16 m_lastIndex;
    int m_fillCount;

    RtpPacket* m_data;
    int m_packetSize;
    void*       m_silence;

    mutable QMutex m_mutex;
};

#endif // RTPBUFFER_H
