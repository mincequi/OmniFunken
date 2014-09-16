#ifndef RTPRECEIVER_H
#define RTPRECEIVER_H

#include "Airtunes/Airtunesconstants.h"
#include "rtsp/rtspmessage.h"

#include "alac.h"

#include <openssl/aes.h>

#include <QFile>
#include <QList>
#include <QObject>
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
        Airtunes::PayloadType payloadType;
        quint16 sequenceNumber;
        quint32 timestamp;
        quint32 ssrc;
    };

    explicit RtpReceiver(RtpBuffer *rtpBuffer, QObject *parent = 0);

signals:


public slots:
    void announce(const RtspMessage::Announcement &announcement);
    void setSenderSocket(Airtunes::PayloadType payloadType, quint16 port);
    void bindSocket(Airtunes::PayloadType payloadType, quint16 *port);
    void teardown();

private slots:
    void readPendingDatagrams();
    void requestRetransmit(quint16 first, quint16 num);

private:
    void readHeader(const char* data, RtpHeader *header);
    void writeHeader(const RtpHeader *header, char* data);
    void decrypt(const char *in, unsigned char *out, int length);

    void initAlac(const QByteArray &fmtp);
    void decodeAlac();

    quint16     m_senderControlPort;

    QUdpSocket  m_udpSocket;
    AES_KEY     m_aesKey;
    alac_file   *m_alac;
    RtspMessage::Announcement m_announcement;

    RtpBuffer   *m_rtpBuffer;
};

#endif // RTPRECEIVER_H
