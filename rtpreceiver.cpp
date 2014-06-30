#include "rtpreceiver.h"

RtpReceiver::RtpReceiver(QObject *parent) :
    QObject(parent)
{
}

void RtpReceiver::bindSocket(PayloadType payloadType, quint16 *port)
{
    if (!m_udpSocket) m_udpSocket = new QUdpSocket();
    *port = 55555;
}
