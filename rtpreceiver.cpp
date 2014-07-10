#include "rtpreceiver.h"
#include "alac.h"

#include <assert.h>

#include <QtEndian>

RtpReceiver::RtpReceiver(RtpBuffer *rtpBuffer, QObject *parent) :
    QObject(parent),
    m_alac(NULL),
    m_rtpBuffer(rtpBuffer)
{   
    connect(&m_udpSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
}

void RtpReceiver::announce(const RtspMessage::Announcement &announcement)
{
    m_announcement = announcement;
    AES_set_decrypt_key(reinterpret_cast<const unsigned char*>(announcement.rsaAesKey.data()), 128, &m_aesKey);
    initAlac(announcement.fmtp);
    m_rtpBuffer->setPacketSize(352);
}

void RtpReceiver::setSenderSocket()
{
}

void RtpReceiver::bindSocket(RtpReceiver::PayloadType payloadType, quint16 *port)
{
    Q_UNUSED(payloadType);

    if (!m_udpSocket.localPort())
        m_udpSocket.bind();

    *port = m_udpSocket.localPort();
}

void RtpReceiver::teardown()
{
    m_udpSocket.close();

    if (m_alac)
    {
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
        decodeHeader(datagram.data(), &header);
        const char* payload = (datagram.data()+12);
        int payloadSize = datagram.size()-12;

        switch (header.payloadType)
        {
        case Sync:
            break;
        case RetransmitResponse:
            break;
        case AudioData:
        {
            unsigned char packet[2048];
            unsigned char dest[1408];
            int outsize;
            decrypt(payload, packet, payloadSize);

            RtpBuffer::RtpPacket* bufferItem = m_rtpBuffer->putPacket(header.sequenceNumber);
            alac_decode_frame(m_alac, packet, bufferItem->payload, &(bufferItem->payloadSize));
            //alac_decode_frame(m_alac, packet, dest, &outsize);
            //play(reinterpret_cast<char*>(dest), 352);
            break;
        }
        default:
            qCritical("RtpReceiver::readPendingDatagrams: illegal payload type: %d", header.payloadType);
            break;
        }
    }
}

void RtpReceiver::decodeHeader(const char* data, RtpHeader *header)
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

    if (!m_alac)
    {
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



