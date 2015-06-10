#include "rtpreceiverboost.h"

#include "rtpbufferalt.h"
#include "rtpheader.h"
#include "rtppacket.h"
#include "alac.h"

#include <assert.h>
#include <boost/bind.hpp>
#include <QElapsedTimer>
#include <QtEndian>

using boost::asio::ip::udp;
namespace ph = boost::asio::placeholders;

RtpReceiver::RtpReceiver(RtpBuffer *rtpBuffer, quint16 retryInterval, QObject *parent) :
    QObject(parent),
    m_senderControlPort(0),
    m_rtpBuffer(rtpBuffer),
    m_retryInterval(retryInterval),
    m_udpWorker(NULL)
{
}

void RtpReceiver::announce(const RtspMessage::Announcement &announcement)
{
    teardown();

    m_announcement = announcement;
}

void RtpReceiver::setSenderSocket(airtunes::PayloadType payloadType, quint16 controlPort)
{
    switch (payloadType) {
    case airtunes::RetransmitRequest:
        m_senderControlPort = controlPort;
        break;
    default:
        break;
    }
}

void RtpReceiver::bindSocket(airtunes::PayloadType payloadType, quint16 *port)
{
    Q_UNUSED(payloadType);

    if (!m_udpWorker) {
        m_udpWorker = new UdpWorker(m_announcement, m_rtpBuffer, m_senderControlPort, m_retryInterval);
    }

    m_udpWorker->start();
    *port = m_udpWorker->port();
}

void RtpReceiver::teardown()
{
    if (m_udpWorker) {
        m_udpWorker->stop();
        m_udpWorker->wait();
        delete m_udpWorker;
        m_udpWorker = NULL;
    }
}

RtpReceiver::UdpWorker::UdpWorker(const RtspMessage::Announcement &announcement, RtpBuffer *rtpBuffer, quint16 senderControlPort, quint16 retryInterval) :
    m_announcement(announcement),
    m_alac(NULL),
    m_rtpBuffer(rtpBuffer),
    m_senderControlPort(senderControlPort),
    m_retryInterval(retryInterval),
    m_retryTimer(m_ioService, boost::posix_time::milliseconds(m_retryInterval))
{
    m_socket = new udp::socket(m_ioService, udp::endpoint(udp::v4(), 0));
    initAlac(m_announcement.fmtp);
    AES_set_decrypt_key(reinterpret_cast<const unsigned char*>(m_announcement.rsaAesKey.data()), 128, &m_aesKey);

    m_retryEndpoint = udp::endpoint(boost::asio::ip::address::from_string(m_announcement.senderAddress.toString().toStdString()), m_senderControlPort);
}

RtpReceiver::UdpWorker::~UdpWorker()
{
    stop();

    if (m_socket) {
        delete m_socket;
    }

    if (m_alac) {
        alac_free(m_alac);
        m_alac = NULL;
    }
}

quint16 RtpReceiver::UdpWorker::port()
{
    return m_socket ? m_socket->local_endpoint().port() : 0;
}

void RtpReceiver::UdpWorker::stop()
{
    m_retryTimer.cancel();

    if (m_work) {
        delete m_work;
        m_work = NULL;
    }
    m_ioService.stop();
}

void RtpReceiver::UdpWorker::run()
{
    m_work = new boost::asio::io_service::work(m_ioService);

    m_retryTimer.async_wait(boost::bind(&UdpWorker::doRequestRetransmit, this));
    doReceive();
    m_ioService.run();
}

void RtpReceiver::UdpWorker::doReceive()
{
    m_socket->async_receive_from(boost::asio::buffer(m_receiveBuffer),
                                 m_remoteEndpoint,
                                 boost::bind(&RtpReceiver::UdpWorker::onReceive, this, ph::error, ph::bytes_transferred));
}

void RtpReceiver::UdpWorker::onReceive(const boost::system::error_code& error, std::size_t bytesTransferred)
{
    if (error) {
        qWarning()<<Q_FUNC_INFO<<" error occurred: "<<error;
    } else {
        RtpHeader header;
        readHeader(m_receiveBuffer.data(), &header);
        const char* payload = (m_receiveBuffer.data()+12);
        int payloadSize = bytesTransferred-12;

        switch (header.payloadType) {
        case airtunes::Sync:
            break;
        case airtunes::RetransmitResponse: {
            header.sequenceNumber = qFromBigEndian(*((quint16*)(m_receiveBuffer.data()+6)));
            payload = payload+4;
            payloadSize = payloadSize-4;
            // need to check payloadSize, since we get broken payloads from time to time
            if (payloadSize < 0) {
                break;
            }
        }
        case airtunes::AudioData: {
//            unsigned char packet[2048];
//            decrypt(payload, packet, payloadSize);
            RtpPacket* rtpPacket = m_rtpBuffer->obtainPacket(header);
            if (rtpPacket) {
                unsigned char packet[2048];
                decrypt(payload, packet, payloadSize);
                alac_decode_frame(m_alac, packet, rtpPacket->payload, &(rtpPacket->payloadSize));
                m_rtpBuffer->commitPacket(rtpPacket);
            }
            break;
        }
        default:
            qCritical("RtpReceiver::readPendingDatagrams: illegal payload type: %d", header.payloadType);
            break;
        }
    }

    doReceive();
}

void RtpReceiver::UdpWorker::initAlac(const QByteArray &fmtp)
{
    QList<QByteArray> fmtpList = fmtp.split(' ');

    if (!m_alac) {
        m_alac = alac_create(16, 2);
        m_alac->setinfo_max_samples_per_frame   = fmtpList.at(1).toUInt();
        m_alac->setinfo_7a                      = fmtpList.at(2).toUInt();
        m_alac->setinfo_sample_size             = fmtpList.at(3).toUInt();
        m_alac->setinfo_rice_historymult        = fmtpList.at(4).toUInt();
        m_alac->setinfo_rice_initialhistory     = fmtpList.at(5).toUInt();
        m_alac->setinfo_rice_kmodifier          = fmtpList.at(6).toUInt();
        m_alac->setinfo_7f                      = fmtpList.at(7).toUInt();
        m_alac->setinfo_80                      = fmtpList.at(8).toUInt();
        m_alac->setinfo_82                      = fmtpList.at(9).toUInt();
        m_alac->setinfo_86                      = fmtpList.at(10).toUInt();
        m_alac->setinfo_8a_rate                 = fmtpList.at(11).toUInt();
        alac_allocate_buffers(m_alac);
    }
}

void RtpReceiver::UdpWorker::decrypt(const char *in, unsigned char *out, int length)
{
    unsigned char iv[16];
    int encryptedSize = length & ~0xf;
    memcpy(iv, m_announcement.aesIv.data(), sizeof(iv));
    AES_cbc_encrypt(reinterpret_cast<const unsigned char*>(in), out, encryptedSize, &m_aesKey, iv, AES_DECRYPT);
    memcpy(out+encryptedSize, in+encryptedSize, length-encryptedSize);
}

void RtpReceiver::UdpWorker::doRequestRetransmit()
{
    auto sequences = m_rtpBuffer->missingSequences();
    for (const RtpBuffer::Sequence& sequence : sequences) {
        qDebug()<<Q_FUNC_INFO<<"first:"<<sequence.first<<"count:"<<sequence.count;

        char req[8];    // *not* a standard RTCP NACK
        req[0] = 0x80;
        req[1] = 0x55|0x80;  // Apple 'resend'

        *(unsigned short *)(req+2) = qToBigEndian(1);  // our seqnum
        *(unsigned short *)(req+4) = qToBigEndian(sequence.first);  // missed seqnum
        *(unsigned short *)(req+6) = qToBigEndian(sequence.count);  // count

        m_socket->async_send_to(boost::asio::buffer(req, 8),
                                m_retryEndpoint,
                                boost::bind(&RtpReceiver::UdpWorker::onRequestRetransmit, this, ph::error, ph::bytes_transferred));
    }

    m_retryTimer.expires_at(m_retryTimer.expires_at() + boost::posix_time::milliseconds(m_retryInterval));
    m_retryTimer.async_wait(boost::bind(&UdpWorker::doRequestRetransmit, this));
}

void RtpReceiver::UdpWorker::onRequestRetransmit(const boost::system::error_code& error, std::size_t /*bytesTransferred*/)
{
    if (error) {
        qWarning()<<Q_FUNC_INFO<<"error occurred:"<<error;
    }
}
