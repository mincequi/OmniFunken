#ifndef RTPRECEIVERBOOST_H
#define RTPRECEIVERBOOST_H

#include "alac.h"
#include "airtunes/airtunesconstants.h"
#include "rtsp/rtspmessage.h"

#include <memory>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <openssl/aes.h>

#include <QObject>
#include <QThread>

class RtpBuffer;
class QElapsedTimer;

class RtpReceiver : public QObject
{
    Q_OBJECT
public:
    explicit RtpReceiver(RtpBuffer *rtpBuffer, quint16 retryInterval = 25, QObject *parent = 0);

public slots:
    void announce(const RtspMessage::Announcement &announcement);
    void setSenderSocket(airtunes::PayloadType payloadType, quint16 port);
    void bindSocket(airtunes::PayloadType payloadType, quint16 *port);
    void teardown();

private:
    class UdpWorker : public QThread
    {
    public:
        UdpWorker(const RtspMessage::Announcement& announcement, RtpBuffer *rtpBuffer, quint16 senderControlPort, quint16 retryInterval = 25);
        ~UdpWorker();
        quint16 port();
        void stop();

    private:
        void run() Q_DECL_OVERRIDE;
        void doReceive();
        void onReceive(const boost::system::error_code& error, std::size_t bytesTransferred);

        void initAlac(const QByteArray &fmtp);
        void decrypt(const char *in, unsigned char *out, int length);

        boost::asio::io_service m_ioService;
        boost::asio::io_service::work   *m_work;
        boost::asio::ip::udp::socket    *m_socket;
        boost::asio::ip::udp::endpoint  m_remoteEndpoint;
        boost::array<char, 1536>        m_receiveBuffer;

        RtspMessage::Announcement m_announcement;

        alac_file   *m_alac;
        AES_KEY     m_aesKey;

        RtpBuffer   *m_rtpBuffer;

        //retry members
        void doRequestRetransmit();
        void onRequestRetransmit(const boost::system::error_code& error, std::size_t bytesTransferred);
        quint16         m_senderControlPort;
        boost::asio::ip::udp::endpoint  m_retryEndpoint;
        quint16         m_retryInterval;
        boost::asio::deadline_timer m_retryTimer;
    };

private:
    quint16     m_senderControlPort;

    RtspMessage::Announcement m_announcement;

    RtpBuffer   *m_rtpBuffer;
    quint16     m_retryInterval;

    UdpWorker   *m_udpWorker;
};

#endif // RTPRECEIVERBOOST_H
