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

protected:
    void incomingConnection(qintptr socketDescriptor) Q_DECL_OVERRIDE;

signals:
    void announce(const RtspMessage::Announcement & announcement);
    void senderSocketAvailable(airtunes::PayloadType payloadType, quint16 port);
    // Note: It is not possible to use a QueuedConnection to connect to this signal
    void receiverSocketRequired(airtunes::PayloadType payloadType, quint16 *port);
    void record(quint16 seq);
    void flush(quint16 seq);
    void volume(float db);
    void teardown();

public slots:
    void reset();

private:
    quint8      m_macAddress[6];
    ulong       m_dacpId;
};

#endif // RTSPSERVER_H
