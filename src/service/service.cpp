#include "service.h"

#include "player.h"
#include "util.h"
#include "audioout/audioout_abstract.h"
#include "audioout/audiooutfactory.h"
#include "core/core.h"
#include "devicecontrol/devicecontrolabstract.h"
#include "devicecontrol/devicecontrolfactory.h"
#include "devicecontrol/devicewatcher.h"
#include "rtsp/rtspserver.h"
#include "rtp/rtpbuffer.h"
//#include "rtp/rtpreceiver.h"
#include "rtp/rtpreceiverboost.h"
#include "rtp/rtpretransmissionrequester.h"
#include "zeroconf/zeroconf_dns_sd.h"

Service::Service(const ServiceConfig &serviceConfig, QObject *parent) :
    QObject(parent),
    m_config(serviceConfig)
{
}

Service::~Service()
{
}

void Service::open()
{
    initNetwork();
    initZeroconf();
}

void Service::close()
{
    deinitZeroconf();
    deinitNetwork();
}

ServiceConfig Service::config() const
{
    return m_config;
}

void Service::initNetwork()
{
    // init rtsp/rtp components
    RtspServer  *rtspServer = new RtspServer(Util::getMacAddress(), this);
    RtpBuffer   *rtpBuffer = new RtpBuffer(airtunes::framesPerPacket, config().latency(), this);
    RtpReceiver *rtpReceiver = new RtpReceiver(rtpBuffer, config().latency()/10, this);

    // init player
    Player      *player = new Player(rtpBuffer, ofCore->audioOut(), this);

    // wire components
    //QObject::connect(rtspServer, SIGNAL(announce(RtspMessage::Announcement)), rtpReceiver, SLOT(announce(RtspMessage::Announcement)));
    QObject::connect(rtspServer, &RtspServer::senderSocketAvailable, rtpReceiver, &RtpReceiver::setSenderSocket);
    //QObject::connect(rtspServer, &RtspServer::senderSocketAvailable, rtpRetransmissionRequester, &RtpRetransmissionRequester::setSenderSocket);
    QObject::connect(rtspServer, &RtspServer::receiverSocketRequired, rtpReceiver, &RtpReceiver::bindSocket);
    QObject::connect(rtspServer, SIGNAL(record(quint16)), rtpBuffer, SLOT(flush(quint16)));
    QObject::connect(rtspServer, SIGNAL(flush(quint16)), rtpBuffer, SLOT(flush(quint16)));
    QObject::connect(rtspServer, &RtspServer::teardown, rtpBuffer, &RtpBuffer::teardown);
    QObject::connect(rtspServer, &RtspServer::teardown, rtpReceiver, &RtpReceiver::teardown);
    QObject::connect(rtspServer, &RtspServer::teardown, player, &Player::teardown);

    if (ofCore->deviceControl()) {
        QObject::connect(rtspServer, &RtspServer::volume, ofCore->deviceControl(), &DeviceControlAbstract::setVolume);
    } else {
        QObject::connect(rtspServer, &RtspServer::volume, player, &Player::setVolume);
    }

    QObject::connect(rtspServer, &RtspServer::announce, rtpReceiver, &RtpReceiver::announce);
    //QObject::connect(rtspServer, &RtspServer::announce, rtpRetransmissionRequester, &RtpRetransmissionRequester::announce);

    // startup
    rtspServer->listen(QHostAddress::AnyIPv4, config().port());
}

void Service::deinitNetwork()
{
}

void Service::initZeroconf()
{
    // register service
    ZeroconfDnsSd *dnsSd = new ZeroconfDnsSd(Util::getMacAddress());
    dnsSd->registerService(config().name().toLatin1(), config().port());
}

void Service::deinitZeroconf()
{
}

