#include <QCoreApplication>

#include "rtspserver.h"
#include "zeroconf_dns_sd.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    RtpReceiver rtpReceiver;
    RtspServer  rtspServer;

    QObject::connect(&rtspServer, SIGNAL(announced(RtspMessage::Announcement)),
                     &rtpReceiver, SLOT(announce(RtspMessage::Announcement)));
    QObject::connect(&rtspServer, SIGNAL(senderSocketAvailable(RtpReceiver::PayloadType, quint16)),
                     &rtpReceiver, SLOT(setSenderSocket()));
    QObject::connect(&rtspServer, SIGNAL(receiverSocketRequired(RtpReceiver::PayloadType, quint16*)),
                     &rtpReceiver, SLOT(bindSocket(RtpReceiver::PayloadType, quint16*)));
    QObject::connect(&rtspServer, SIGNAL(teareddown()),
                     &rtpReceiver, SLOT(teardown()));

    rtspServer.listen(QHostAddress::AnyIPv4, 5002);

    ZeroconfDnsSd dnsSd;
    int error = dnsSd.registerService("MINZIII", 5002);

    return a.exec();
}
