#include "rtspserver_threaded.h"

#include "rtspsession.h"
#include "util.h"

RtspServer::RtspServer(QObject *parent)
    : QTcpServer(parent)
{
}

void RtspServer::incomingConnection(qintptr socketDescriptor)
{
    RtspSession *rtspSession = new RtspSession(socketDescriptor, this);
    connect(rtspSession, &RtspSession::finished, rtspSession, &RtspSession::deleteLater);
    rtspSession->start();
}

