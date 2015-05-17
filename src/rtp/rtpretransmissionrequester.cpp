#include "rtpretransmissionrequester.h"

#include <QtEndian>
#include <QTimer>
#include <QUdpSocket>

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

RtpRetransmissionRequester::RtpRetransmissionRequester(RtpBuffer *rtpBuffer, QUdpSocket* udpSocket, int retryInterval, QObject *parent) :
    QObject(parent),
    m_rtpBuffer(rtpBuffer),
    m_udpSocket(udpSocket),
    m_senderControlPort(0),
    m_retryInterval(retryInterval),
    m_socketDescriptor(-1)
{
    //m_udpSocket = new QUdpSocket(this);
    m_retryTimer = new QTimer(this);

    //connect(m_rtpBuffer, &RtpBuffer::stateChanged, this, &RtpRetransmissionRequester::onBufferStateChanged);
    //connect(m_rtpBuffer, &RtpBuffer::stateChanged, this, &RtpRetransmissionRequester::sendRequest);
    //connect(m_retryTimer, &QTimer::timeout, this, &RtpRetransmissionRequester::sendRequest);
}

void RtpRetransmissionRequester::announce(const RtspMessage::Announcement &announcement)
{
    m_announcement = announcement;
    qDebug()<<Q_FUNC_INFO<< announcement.senderAddress;
}

void RtpRetransmissionRequester::setSenderSocket(airtunes::PayloadType payloadType, quint16 controlPort)
{
    switch (payloadType) {
    case airtunes::RetransmitRequest:
        m_senderControlPort = controlPort;
        qDebug()<<Q_FUNC_INFO<< controlPort;
        break;
    default:
        break;
    }
}

void RtpRetransmissionRequester::onBufferStateChanged(RtpBuffer::State state)
{
    if (state == RtpBuffer::Empty) {
        m_retryTimer->stop();
        //closeSocket();
    } else if (!m_retryTimer->isActive()) {
        //createSocket();
        m_retryTimer->start(m_retryInterval);
    }
}

bool RtpRetransmissionRequester::createSocket()
{
    int protocol = AF_INET;
    int type = SOCK_DGRAM;

#if defined(SOCK_CLOEXEC) && defined(SOCK_NONBLOCK)
    //type |= SOCK_CLOEXEC;
#endif

    int socket = ::socket(protocol, type, IPPROTO_UDP);

    if (socket <= 0) {
        return false;
    }

    // Ensure that the socket is closed on exec*().
    ::fcntl(socket, F_SETFD, FD_CLOEXEC);

    m_socketDescriptor = socket;
    return true;
}

void RtpRetransmissionRequester::closeSocket()
{
    ::close(m_socketDescriptor);
    m_socketDescriptor = -1;
}

void RtpRetransmissionRequester::sendRequest()
{
    auto sequences = m_rtpBuffer->missingSequences();
    for (const RtpBuffer::Sequence& sequence : sequences) {
        qWarning()<<Q_FUNC_INFO<< sequence.first << sequence.count << " from " << m_announcement.senderAddress << m_senderControlPort;
        char req[8];    // *not* a standard RTCP NACK
        req[0] = 0x80;
        req[1] = 0x55|0x80;  // Apple 'resend'

        *(unsigned short *)(req+2) = qToBigEndian(1);  // our seqnum
        *(unsigned short *)(req+4) = qToBigEndian(sequence.first);  // missed seqnum
        *(unsigned short *)(req+6) = qToBigEndian(sequence.count);  // count

        // QT Bug!!
        m_udpSocket->writeDatagram(req, 8, m_announcement.senderAddress, m_senderControlPort);
        //m_udpSocket->writeDatagram(req, 8, QHostAddress("192.168.1.10"), m_senderControlPort);

        /*
        struct sockaddr_in sockAddrIPv4;
        struct sockaddr *sockAddrPtr = 0;
        socklen_t sockAddrSize = 0;

        memset(&sockAddrIPv4, 0, sizeof(sockAddrIPv4));
        sockAddrIPv4.sin_family = AF_INET;
        sockAddrIPv4.sin_port = htons(m_senderControlPort);
        sockAddrIPv4.sin_addr.s_addr = htonl(m_announcement.senderAddress.toIPv4Address());
        sockAddrSize = sizeof(sockAddrIPv4);
        sockAddrPtr = (struct sockaddr *)&sockAddrIPv4;

        ssize_t sentBytes = ::sendto(m_socketDescriptor, req, sizeof(req), 0, sockAddrPtr, sockAddrSize);
        if (sentBytes < 0) {
            qWarning()<<Q_FUNC_INFO<< "error sending request";
        }
        */
    }
}
