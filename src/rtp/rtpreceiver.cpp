#include "rtpreceiver.h"
#include "rtp/rtpbuffer.h"
#include "rtp/rtppacket.h"
#include "alac.h"

#include <assert.h>

#include <QtEndian>

RtpReceiver::RtpReceiver(RtpBuffer *rtpBuffer, quint16 retryInterval, QObject *parent) :
    QObject(parent),
    m_senderControlPort(0),
    m_alac(NULL),
    m_rtpBuffer(rtpBuffer),
    m_retryInterval(retryInterval)

{   
    connect(&m_udpSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
    //connect(m_rtpBuffer, SIGNAL(missingSequence(quint16,quint16)), this, SLOT(requestRetransmit(quint16,quint16)));
    connect(&m_retryTimer, &QTimer::timeout, this, &RtpReceiver::requestRetransmit);
}

void RtpReceiver::announce(const RtspMessage::Announcement &announcement)
{
    qDebug() << __PRETTY_FUNCTION__;
    teardown();

    m_announcement = announcement;
    AES_set_decrypt_key(reinterpret_cast<const unsigned char*>(announcement.rsaAesKey.data()), 128, &m_aesKey);
    initAlac(announcement.fmtp);

    m_retryTimer.start(m_retryInterval);
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

    if (!m_udpSocket.localPort()) {
        m_udpSocket.bind();
    }

    *port = m_udpSocket.localPort();
}

void RtpReceiver::teardown()
{
    m_retryTimer.stop();
    m_udpSocket.close();

    if (m_alac) {
        alac_free(m_alac);
        m_alac = NULL;
    }
}

void RtpReceiver::readPendingDatagrams()
{
    while (m_udpSocket.hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_udpSocket.pendingDatagramSize());
        m_udpSocket.readDatagram(datagram.data(), datagram.size());
        RtpHeader header;
        readHeader(datagram.data(), &header);
        const char* payload = (datagram.data()+12);
        int payloadSize = datagram.size()-12;
        if (payloadSize < 16) {
            //qDebug() << Q_FUNC_INFO << ": illegal payload size";
            return;
        }

        switch (header.payloadType) {
        case airtunes::Sync:
            break;
        case airtunes::RetransmitResponse: {
            header.sequenceNumber = qFromBigEndian(*((quint16*)(datagram.data()+6)));
            payload = payload+4;
            payloadSize = payloadSize-4;
        }
        case airtunes::AudioData: {
            unsigned char packet[2048];
            decrypt(payload, packet, payloadSize);
            RtpPacket* rtpPacket = m_rtpBuffer->obtainPacket(header.sequenceNumber);
            if (rtpPacket) {
                alac_decode_frame(m_alac, packet, rtpPacket->payload, &(rtpPacket->payloadSize));
            }
            m_rtpBuffer->commitPacket();
            break;
        }
        default:
            qCritical("RtpReceiver::readPendingDatagrams: illegal payload type: %d", header.payloadType);
            break;
        }
    }
}

void RtpReceiver::requestRetransmit()
{
    auto sequences = m_rtpBuffer->missingSequences();
    for (const RtpBuffer::Sequence& sequence : sequences) {
        qWarning() << __PRETTY_FUNCTION__ << sequence.first << sequence.count;

        char req[8];    // *not* a standard RTCP NACK
        req[0] = 0x80;
        req[1] = 0x55|0x80;  // Apple 'resend'

        *(unsigned short *)(req+2) = qToBigEndian(1);  // our seqnum
        *(unsigned short *)(req+4) = qToBigEndian(sequence.first);  // missed seqnum
        *(unsigned short *)(req+6) = qToBigEndian(sequence.count);  // count

        m_udpSocket.writeDatagram(req, 8, m_announcement.senderAddress, m_senderControlPort);
    }
}

void RtpReceiver::readHeader(const char* data, RtpHeader *header)
{
    header->version     = (data[0] >> 6) & 0x03;
    header->padding     = (data[0] >> 5) & 0x01;
    header->extension   = (data[0] >> 4) & 0x01;
    header->csrcCount   = (data[0] >> 0) & 0x0f;
    header->marker      = (data[1] >> 7) & 0x01;
    header->payloadType = static_cast<airtunes::PayloadType>((data[1] >> 0) & 0x7f);

    header->sequenceNumber  = qFromBigEndian(*((quint16*)(data+2)));
    header->timestamp       = qFromBigEndian(*((quint32*)(data+4)));
    header->ssrc            = qFromBigEndian(*((quint32*)(data+8)));
}

void RtpReceiver::initAlac(const QByteArray &fmtp)
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

void RtpReceiver::decrypt(const char *in, unsigned char *out, int length)
{
    unsigned char iv[16];
    int encryptedSize = length & ~0xf;
    memcpy(iv, m_announcement.aesIv.data(), sizeof(iv));
    AES_cbc_encrypt(reinterpret_cast<const unsigned char*>(in), out, encryptedSize, &m_aesKey, iv, AES_DECRYPT);
    memcpy(out+encryptedSize, in+encryptedSize, length-encryptedSize);
}
