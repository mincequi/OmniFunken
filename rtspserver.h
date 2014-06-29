#ifndef RTSPSERVER_H
#define RTSPSERVER_H

#include "rtspmessage.h"

#include <QTcpServer>



class RtspServer : public QObject
{
    Q_OBJECT

public:
    struct Announcement {
        uint framesPerPacket;
        QByteArray rsaAesKey;
        QByteArray aesIv;
    };

public:
    RtspServer(QObject *parent = 0);

    bool listen(const QHostAddress &address = QHostAddress::Any, quint16 port = 0);

signals:
    void announced(const Announcement &announcement);
    void setup();

public slots:

private slots:
    void onNewConnection();
    void onRequest();

private:
    void handleOptions(const RtspMessage &request, RtspMessage *response);
    void handleAnnounce(const RtspMessage &request, RtspMessage *response);
    void handleSetupRequest(const RtspMessage &request);
    void handleSetupResponse(RtspMessage *response);
    void handleRecord(const RtspMessage &request, RtspMessage *response);
    void handleFlush(const RtspMessage &request, RtspMessage *response);
    void handleTeardown(const RtspMessage &request, RtspMessage *response);
    void handleAppleChallenge(const RtspMessage &request, RtspMessage *response, quint32 localAddress);


private:
    QTcpServer *m_tcpServer;
};

#endif // RTSPSERVER_H
