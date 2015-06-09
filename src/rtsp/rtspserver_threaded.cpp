#include "rtspserver_threaded.h"

#include "rtspworker.h"
#include "util.h"

RtspServer::RtspServer(QObject *parent)
    : QTcpServer(parent),
      m_dacpId(0)
{
}

void RtspServer::incomingConnection(qintptr socketDescriptor)
{
    RtspWorker *worker = new RtspWorker(socketDescriptor, this);
    connect(worker, &RtspWorker::finished, worker, &RtspWorker::deleteLater);
    worker->start();
}

void RtspServer::reset()
{
    if (isListening()) {
        close();
    }
}
