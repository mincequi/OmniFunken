#ifndef RTPRETRANSMISSIONREQUESTER_H
#define RTPRETRANSMISSIONREQUESTER_H

#include "rtpbuffer.h"
#include "airtunes/airtunesconstants.h"
#include "rtsp/rtspmessage.h"
#include <QObject>

class QTimer;
class QUdpSocket;

class RtpRetransmissionRequester : public QObject
{
    Q_OBJECT
public:
    explicit RtpRetransmissionRequester(RtpBuffer* rtpBuffer, QUdpSocket* udpSocket, int retryInterval, QObject *parent = 0);

public slots:
    void announce(const RtspMessage::Announcement &announcement);
    void setSenderSocket(airtunes::PayloadType payloadType, quint16 port);

private slots:
    void onBufferStateChanged(RtpBuffer::State state);

private:
    bool createSocket();
    void closeSocket();
    void sendRequest();

    RtpBuffer   *m_rtpBuffer;
    QUdpSocket  *m_udpSocket;
    quint16     m_senderControlPort;
    RtspMessage::Announcement m_announcement;
    QTimer      *m_retryTimer;
    int         m_retryInterval;

    int         m_socketDescriptor;
};

#endif // RTPRETRANSMISSIONREQUESTER_H
