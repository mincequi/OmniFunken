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
#include "rtp/rtpreceiver.h"
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
    initDeviceControl();
    initNetwork();
    initZeroconf();
}

void Service::close()
{
    deinitZeroconf();
    deinitNetwork();
    deinitDeviceControl();

    ofCore->audioOut()->deinit();
}

ServiceConfig Service::config() const
{
    return m_config;
}

void Service::initDeviceControl()
{
}

void Service::deinitDeviceControl()
{
    if (m_deviceControl) {
        m_deviceControl->deinit();
    }
}

void Service::initNetwork()
{
    // init rtsp/rtp components
    RtspServer  *rtspServer = new RtspServer(Util::getMacAddress());
    RtpBuffer   *rtpBuffer = new RtpBuffer(config().latency());
    RtpReceiver *rtpReceiver = new RtpReceiver(rtpBuffer);

    // init player
    Player      *player = new Player(rtpBuffer, ofCore->audioOut());

    // init device control
    m_deviceControl = DeviceControlFactory::createDeviceControl(ofCore->settings());

    // wire components
    //QObject::connect(rtspServer, SIGNAL(announce(RtspMessage::Announcement)), rtpReceiver, SLOT(announce(RtspMessage::Announcement)));
    QObject::connect(rtspServer, &RtspServer::senderSocketAvailable, rtpReceiver, &RtpReceiver::setSenderSocket);
    QObject::connect(rtspServer, &RtspServer::receiverSocketRequired, rtpReceiver, &RtpReceiver::bindSocket);
    QObject::connect(rtspServer, SIGNAL(record(quint16)), rtpBuffer, SLOT(flush(quint16)));
    QObject::connect(rtspServer, SIGNAL(flush(quint16)), rtpBuffer, SLOT(flush(quint16)));
    QObject::connect(rtspServer, &RtspServer::teardown, rtpBuffer, &RtpBuffer::teardown);
    QObject::connect(rtspServer, &RtspServer::teardown, rtpReceiver, &RtpReceiver::teardown);
    QObject::connect(rtspServer, &RtspServer::teardown, player, &Player::teardown);

    if (m_deviceControl) {
        QObject::connect(rtspServer, &RtspServer::volume, m_deviceControl, &DeviceControlAbstract::setVolume);
    } else {
        QObject::connect(rtspServer, &RtspServer::volume, player, &Player::setVolume);
    }

    QObject::connect(rtspServer, &RtspServer::announce, this, &Service::onAnnounce, Qt::DirectConnection);
    QObject::connect(rtspServer, &RtspServer::announce, rtpReceiver, &RtpReceiver::announce);

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

void Service::onAnnounce()
{
    qDebug() << __PRETTY_FUNCTION__;
    ofCore->powerOnDevice();
}
