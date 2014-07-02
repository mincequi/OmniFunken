#ifndef RTPRECEIVER_H
#define RTPRECEIVER_H

#include "rtspmessage.h"
#include "alac.h"

#include <ao/ao.h>
#include <openssl/aes.h>

#include <QObject>
#include <QUdpSocket>


class RtpReceiver : public QObject
{
    Q_OBJECT
public:
    enum PayloadType {
        TimingRequest       = 82,
        TimingResponse      = 83,
        Sync                = 84,
        RetransmitRequest   = 85,
        RetransmitResponse  = 86,
        AudioData           = 96,
    };

    /*
    struct RtpHeader {
        quint8  version : 2;
        bool    padding : 1;
        bool    extension : 1;
        quint8  csrcCount : 4;
        bool    marker : 1;
        quint8  payloadType : 7;
        quint16 sequenceNumber;
        quint32 timestamp;
        quint32 ssrc;
    };
    */

    struct RtpHeader {
        quint8  version;
        bool    padding;
        bool    extension;
        quint8  csrcCount;
        bool    marker;
        PayloadType payloadType;
        quint16 sequenceNumber;
        quint32 timestamp;
        quint32 ssrc;
    };

    explicit RtpReceiver(QObject *parent = 0);

signals:


public slots:
    void announce(const RtspMessage::Announcement &announcement);
    void setSenderSocket();
    void bindSocket(RtpReceiver::PayloadType payloadType, quint16 *port);
    void teardown();

private slots:
    void readPendingDatagrams();

private:
    void decodeHeader(const char* data, RtpHeader *header);
    void decrypt(const char *in, unsigned char *out, int length);

    void initAlac(const QByteArray &fmtp);
    void decodeAlac();

    int init();
    void play(char buf[], int samples);
    void deinit();

    QUdpSocket  m_udpSocket;
    AES_KEY     m_aesKey;
    alac_file   *m_alac;
    RtspMessage::Announcement m_announcement;
    ao_device *m_aoDevice;
};

#endif // RTPRECEIVER_H
