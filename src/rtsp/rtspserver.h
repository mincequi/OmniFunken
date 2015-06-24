#ifndef RTSPSERVER_H
#define RTSPSERVER_H

#include "airtunes/airtunesconstants.h"
#include "rtspmessage.h"
#include <QTcpServer>

class RtspServer : public QTcpServer
{
    Q_OBJECT
    
public:
    RtspServer(QObject *parent = 0);

signals:
    void announce(const RtspMessage::Announcement & announcement);
    void senderSocketAvailable(airtunes::PayloadType payloadType, quint16 port);
    // Note: It is not possible to use a QueuedConnection to connect to this signal
    void receiverSocketRequired(airtunes::PayloadType payloadType, quint16 *port);
    void record(quint16 seq);
    void flush(quint16 seq);
    void volume(float db);
    void teardown();

protected:
    virtual void incomingConnection(qintptr socketDescriptor) Q_DECL_OVERRIDE;

public slots:
    void reset();

private slots:
    void onRequest();

private:
    void handleOptions(const RtspMessage &request, RtspMessage *response);
    void handleAnnounce(const RtspMessage &request, RtspMessage *response);
    void handleSetup(const RtspMessage &request, RtspMessage *response);
    void handleRecord(const RtspMessage &request, RtspMessage *response);
    void handleFlush(const RtspMessage &request, RtspMessage *response);
    void handleTeardown(const RtspMessage &request, RtspMessage *response);
    void handleSetParameter(const RtspMessage &request, RtspMessage *response);
    void handleAppleChallenge(const RtspMessage &request, RtspMessage *response, quint32 localAddress);

private:
    quint8      m_macAddress[6];
    quint64     m_dacpId;
};

#endif // RTSPSERVER_H
