#include "rtspserver_threaded.h"

#include "rtspsession.h"
#include "util.h"

RtspServer::RtspServer(QObject *parent)
    : QTcpServer(parent),
      m_dacpId(0)
{
}

void RtspServer::incomingConnection(qintptr socketDescriptor)
{
    RtspSession *worker = new RtspSession(socketDescriptor, this);
    connect(worker, &RtspSession::finished, worker, &RtspSession::deleteLater);
    worker->start();
}

void RtspServer::reset()
{
    if (isListening()) {
        close();
    }
}
