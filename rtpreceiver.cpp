#include "rtpreceiver.h"
#include "alac.h"

#include <assert.h>

#include <QtEndian>

RtpReceiver::RtpReceiver(RtpBuffer *rtpBuffer, QObject *parent) :
    QObject(parent),
    m_senderControlPort(0),
    m_alac(NULL),
    m_rtpBuffer(rtpBuffer)

{   
    connect(&m_udpSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
    connect(m_rtpBuffer, SIGNAL(missingSequence(quint16,quint16)), this, SLOT(requestRetransmit(quint16,quint16)));
}

void RtpReceiver::announce(const RtspMessage::Announcement &announcement)
{
    teardown();

    m_announcement = announcement;
    AES_set_decrypt_key(reinterpret_cast<const unsigned char*>(announcement.rsaAesKey.data()), 128, &m_aesKey);
    m_rtpBuffer->setPacketSize(352);
    initAlac(announcement.fmtp);
}

void RtpReceiver::setSenderSocket(RtpReceiver::PayloadType payloadType, quint16 controlPort)
{
    switch (payloadType) {
    case RetransmitRequest:
        m_senderControlPort = controlPort;
        break;
    default:
        break;
    }
}

void RtpReceiver::bindSocket(RtpReceiver::PayloadType payloadType, quint16 *port)
{
    Q_UNUSED(payloadType);

    if (!m_udpSocket.localPort()) {
        m_udpSocket.bind();
    }

    *port = m_udpSocket.localPort();
}

void RtpReceiver::teardown()
{
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
        QHostAddress sender;
        quint16 senderPort;
        m_udpSocket.readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        RtpHeader header;
        readHeader(datagram.data(), &header);
        const char* payload = (datagram.data()+12);
        int payloadSize = datagram.size()-12;
        if (payloadSize < 16) {
            return;
        }

        switch (header.payloadType) {
        case Sync:
            break;
        case RetransmitResponse: {
            header.sequenceNumber = qFromBigEndian(*((quint16*)(datagram.data()+6)));
            payload = payload+4;
            payloadSize = payloadSize-4;
        }
        case AudioData: {
            unsigned char packet[2048];
            decrypt(payload, packet, payloadSize);
            RtpPacket* bufferItem = m_rtpBuffer->obtainPacket(header.sequenceNumber);
            if (bufferItem) {
                if (bufferItem->twice) {
                    char* twice = new char[bufferItem->payloadSize];
                    alac_decode_frame(m_alac, packet, twice, &(bufferItem->payloadSize));
                    int result = memcmp(twice, bufferItem->payload, bufferItem->payloadSize);
                    qWarning() << __func__ << ": memcmp: " << result;
                    delete[] twice;
                    m_rtpBuffer->commitPacket();
                }
                alac_decode_frame(m_alac, packet, bufferItem->payload, &(bufferItem->payloadSize));
                m_rtpBuffer->commitPacket();
            }
            break;
        }
        default:
            qCritical("RtpReceiver::readPendingDatagrams: illegal payload type: %d", header.payloadType);
            break;
        }
    }
}

void RtpReceiver::requestRetransmit(quint16 first, quint16 num)
{
    char req[8];    // *not* a standard RTCP NACK
    req[0] = 0x80;
    req[1] = 0x55|0x80;  // Apple 'resend'
//    *(unsigned short *)(req+2) = htons(1);  // our seqnum
//    *(unsigned short *)(req+4) = htons(first);  // missed seqnum
//    *(unsigned short *)(req+6) = htons(num);  // count

    *(unsigned short *)(req+2) = qToBigEndian(1);  // our seqnum
    *(unsigned short *)(req+4) = qToBigEndian(first);  // missed seqnum
    *(unsigned short *)(req+6) = qToBigEndian(num);  // count

    m_udpSocket.writeDatagram(req, 8, m_announcement.senderAddress, m_senderControlPort);
}

void RtpReceiver::readHeader(const char* data, RtpHeader *header)
{
    header->version     = (data[0] >> 6) & 0x03;
    header->padding     = (data[0] >> 5) & 0x01;
    header->extension   = (data[0] >> 4) & 0x01;
    header->csrcCount   = (data[0] >> 0) & 0x0f;
    header->marker      = (data[1] >> 7) & 0x01;
    header->payloadType = static_cast<PayloadType>((data[1] >> 0) & 0x7f);

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



