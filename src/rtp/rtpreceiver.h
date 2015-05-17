#ifndef RTPRECEIVER_H
#define RTPRECEIVER_H

#include "alac.h"
#include "rtpbuffer.h"
#include "airtunes/airtunesconstants.h"
#include "rtsp/rtspmessage.h"

#include <openssl/aes.h>

#include <QObject>

class QElapsedTimer;
class QUdpSocket;

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
    QUdpSocket* socket() { return m_udpSocket; }

public slots:
    void announce(const RtspMessage::Announcement &announcement);
    void setSenderSocket(airtunes::PayloadType payloadType, quint16 port);
    void bindSocket(airtunes::PayloadType payloadType, quint16 *port);
    void teardown();

private slots:
    void readPendingDatagrams();
    void requestRetransmit();

private:
    void readHeader(const char* data, RtpHeader *header);
    void decrypt(const char *in, unsigned char *out, int length);

    void initAlac(const QByteArray &fmtp);

private:
    quint16     m_senderControlPort;

    QUdpSocket  *m_udpSocket;
    AES_KEY     m_aesKey;
    alac_file   *m_alac;
    RtspMessage::Announcement m_announcement;

    RtpBuffer   *m_rtpBuffer;

    quint16         m_retryInterval;
    QElapsedTimer   *m_elapsedTimer;
};

#endif // RTPRECEIVER_H
