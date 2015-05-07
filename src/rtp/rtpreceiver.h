#ifndef RTPRECEIVER_H
#define RTPRECEIVER_H

#include "airtunes/airtunesconstants.h"
#include "rtsp/rtspmessage.h"

#include "alac.h"

#include <openssl/aes.h>

#include <QFile>
#include <QList>
#include <QObject>
#include <QTimer>
#include <QUdpSocket>

class RtpBuffer;

class RtpReceiver : public QObject
{
    Q_OBJECT
public:
    struct RtpHeader {
        quint8  version;
        bool    padding;
        bool    extension;
        quint8  csrcCount;
        bool    marker;
        airtunes::PayloadType payloadType;
        quint16 sequenceNumber;
        quint32 timestamp;
        quint32 ssrc;
    };

    explicit RtpReceiver(RtpBuffer *rtpBuffer, quint16 retryInterval = 25, QObject *parent = 0);

public slots:
    void announce(const RtspMessage::Announcement &announcement);
    void setSenderSocket(airtunes::PayloadType payloadType, quint16 port);
    void bindSocket(airtunes::PayloadType payloadType, quint16 *port);
    void teardown();

private slots:
    void readPendingDatagrams();
    void requestRetransmit();
    //void doRequestRetransmit(quint16 seq, quint16 count);

private:
    void readHeader(const char* data, RtpHeader *header);
    void writeHeader(const RtpHeader *header, char* data);
    void decrypt(const char *in, unsigned char *out, int length);

    void initAlac(const QByteArray &fmtp);
    void decodeAlac();

private:
    quint16     m_senderControlPort;

    QUdpSocket  *m_udpSocket;
    AES_KEY     m_aesKey;
    alac_file   *m_alac;
    RtspMessage::Announcement m_announcement;

    RtpBuffer   *m_rtpBuffer;

    QTimer      *m_retryTimer;
    quint16     m_retryInterval;
};

#endif // RTPRECEIVER_H
