#include "rtpreceiver.h"

RtpReceiver::RtpReceiver(QObject *parent) :
    QObject(parent)
{
}

void RtpReceiver::announce(const RtspMessage::Announcement &announcement)
{
}

void RtpReceiver::setSenderSocket()
{
}

void RtpReceiver::bindSocket(RtpReceiver::PayloadType payloadType, quint16 *port)
{
    if (!m_udpSocket) m_udpSocket = new QUdpSocket();
    *port = 55555;
}
