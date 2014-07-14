#ifndef RTPRECEIVER_H
#define RTPRECEIVER_H

#include "rtspmessage.h"
#include "rtpbuffer.h"

#include "alac.h"

#include <openssl/aes.h>

#include <QFile>
#include <QList>
#include <QObject>
#include <QUdpSocket>


class RtpReceiver : public QObject
{
    Q_OBJECT
public:
    enum PayloadType {
        TimingRequest       = 82,
        TimingResponse      = 83,
        Sync                = 84,
        RetransmitRequest   = 85,
        RetransmitResponse  = 86,
        AudioData           = 96,
    };

    #pragma pack(1)
    struct RtpHeaderBitfield {
        //#if Q_BYTE_ORDER == Q_BIG_ENDIAN
        unsigned int csrcCount : 4;
        unsigned int extension : 1;
        unsigned int padding : 1;
        unsigned int version : 2;
        unsigned int payloadType : 7;
        unsigned int marker : 1;
        //#else
        //#endif
        quint16 sequenceNumber;
        quint32 timestamp;
        quint32 ssrc;
    };
    #pragma pack()


//{
//#if defined(PJ_IS_BIG_ENDIAN) && (PJ_IS_BIG_ENDIAN!=0)
//    pj_uint16_t v:2;		/**< packet type/version	    */
//    pj_uint16_t p:1;		/**< padding flag		    */
//    pj_uint16_t x:1;		/**< extension flag		    */
//    pj_uint16_t cc:4;		/**< CSRC count			    */
//    pj_uint16_t m:1;		/**< marker bit			    */
//    pj_uint16_t pt:7;		/**< payload type		    */
//#else
//    pj_uint16_t cc:4;		/**< CSRC count			    */
//    pj_uint16_t x:1;		/**< header extension flag	    */
//    pj_uint16_t p:1;		/**< padding flag		    */
//    pj_uint16_t v:2;		/**< packet type/version	    */
//    pj_uint16_t pt:7;		/**< payload type		    */
//    pj_uint16_t m:1;		/**< marker bit			    */
//#endif
//    pj_uint16_t seq;		/**< sequence number		    */
//    pj_uint32_t ts;		/**< timestamp			    */
//    pj_uint32_t ssrc;		/**< synchronization source	    */
//};

    struct RtpHeader {
        quint8  version;
        bool    padding;
        bool    extension;
        quint8  csrcCount;
        bool    marker;
        PayloadType payloadType;
        quint16 sequenceNumber;
        quint32 timestamp;
        quint32 ssrc;
    };



    explicit RtpReceiver(RtpBuffer *rtpBuffer, QObject *parent = 0);

signals:


public slots:
    void announce(const RtspMessage::Announcement &announcement);
    void setSenderSocket(RtpReceiver::PayloadType payloadType, quint16 port);
    void bindSocket(RtpReceiver::PayloadType payloadType, quint16 *port);
    void teardown();

private slots:
    void readPendingDatagrams();
    void requestRetransmit(quint16 first, quint16 num);

private:
    void readHeader(const char* data, RtpHeader *header);
    void writeHeader(const RtpHeader *header, char* data);
    void readHeader(const char* data, RtpHeaderBitfield *header);
    void decrypt(const char *in, unsigned char *out, int length);

    void initAlac(const QByteArray &fmtp);
    void decodeAlac();


    quint16     m_senderControlPort;

    QUdpSocket  m_udpSocket;
    AES_KEY     m_aesKey;
    alac_file   *m_alac;
    RtspMessage::Announcement m_announcement;

    RtpBuffer   *m_rtpBuffer;
};

#endif // RTPRECEIVER_H
