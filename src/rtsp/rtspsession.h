#ifndef RTSPWORKER_H
#define RTSPWORKER_H

#include "rtspmessage.h"
#include "airtunes/airtunesconstants.h"
#include "rtp/rtpreceiver.h"

#include <QThread>

class QTcpSocket;

class RtspSession : public QThread
{
    Q_OBJECT

public:
    RtspSession(qintptr socketDescriptor, QObject *parent = 0);
    void run() Q_DECL_OVERRIDE;

signals:
    void announce(const RtspMessage::Announcement & announcement);
    void senderSocketAvailable(airtunes::PayloadType payloadType, quint16 port);
    // Note: It is not possible to use a QueuedConnection to connect to this signal
    void receiverSocketRequired(airtunes::PayloadType payloadType, quint16 *port);
    void record(quint16 seq);
    void flush(quint16 seq);
    void volume(float db);
    void teardown();

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
    qintptr m_socketDescriptor;
    QTcpSocket *m_socket;

    quint8  m_macAddress[6];
    ulong   m_dacpId;
};

#endif // RTSPWORKER_H
