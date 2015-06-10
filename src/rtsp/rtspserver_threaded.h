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

};

#endif // RTSPSERVER_H
