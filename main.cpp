#include <QCoreApplication>

#include "rtspserver.h"
#include "zeroconf_dns_sd.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    RtspServer rtspServer;
    rtspServer.listen(QHostAddress::AnyIPv4, 5002);

    ZeroconfDnsSd dnsSd;
    int error = dnsSd.registerService("MINZIII", 5002);

    return a.exec();
}
