#ifndef RTPBUFFER_H
#define RTPBUFFER_H


#include "rtppacket.h"

#include <QMutex>
#include <QObject>
#include <QTimer>
#include <QVector>


class RtpBuffer : public QObject
{
    Q_OBJECT
public:
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
        Discard,    // packet is too late
        Early,      // missing packets
        Expected,   // packet is in expected order
        Late,       // packet is late (eventually from request resend)
        Twice       // packet with this seqno has been sent twice
    };
    PacketOrder orderPacket(quint16);

    void requestMissingPackets();

    Status      m_status;

    const int   m_latency;
    int         m_desiredFill;
    int         m_capacity;
    int         m_first;
    int         m_last;
    RtpPacket   *m_data;
    QVector<RtpPacket> m_qdata;
    char        *m_silence;
    int         m_packetSize;

    mutable QMutex  m_mutex;
    QTimer          m_timer;
};

#endif // RTPBUFFER_H
