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
    initAudioOut();
    initDeviceControl();
    initNetwork();
    initZeroconf();
}

void Service::close()
{
    deinitZeroconf();
    deinitNetwork();
    deinitDeviceControl();
    deinitAudioOut();
}

ServiceConfig Service::config() const
{
    return m_config;
}

void Service::initAudioOut()
{
    // init audio driver
    QSettings::SettingsMap settings;
    m_audioOut = AudioOutFactory::createAudioOut(config().audioOut(), config().audioDevice(), settings);

    // init rtsp/rtp components
    RtspServer  *rtspServer = new RtspServer(Util::getMacAddress());
    RtpBuffer   *rtpBuffer = new RtpBuffer(config().latency());
    RtpReceiver *rtpReceiver = new RtpReceiver(rtpBuffer);

    // init player
    Player      *player = new Player(rtpBuffer, m_audioOut);

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
        QObject::connect(rtspServer, &RtspServer::announce, [this]() {
            //qDebug("open deviceControl");
            m_deviceControl->open();
            //qDebug("deviceControl opened");
            m_deviceControl->powerOn();
            //deviceControl->setInput();
        });
        QObject::connect(rtspServer, &RtspServer::volume, m_deviceControl, &DeviceControlAbstract::setVolume);
    } else {
        QObject::connect(rtspServer, &RtspServer::volume, player, &Player::setVolume);
    }

    QObject::connect(rtspServer, &RtspServer::announce, this, &Service::onAnnounce);
    QObject::connect(rtspServer, &RtspServer::announce, rtpReceiver, &RtpReceiver::announce);

    // startup
    rtspServer->listen(QHostAddress::AnyIPv4, config().port());
}

void Service::deinitAudioOut()
{
    if (m_audioOut) {
        m_audioOut->deinit();
    }
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
    // If audio device is not yet ready
    if (!m_audioOut->ready()) {
        DeviceWatcher *deviceWatcher = new DeviceWatcher();
        DeviceWatcher::UDevProperties properties;
        properties["ID_MODEL"] = "Primare_I22_v1.0";

        QEventLoop loop;
        deviceWatcher->start("add", properties);
        QObject::connect(deviceWatcher, &DeviceWatcher::ready, []() { qDebug() << "device ready"; });
        QObject::connect(deviceWatcher, &DeviceWatcher::ready, &loop, &QEventLoop::quit);
        QObject::connect(deviceWatcher, &DeviceWatcher::ready, deviceWatcher, &QObject::deleteLater);
        loop.exec();

        QSettings::SettingsMap settings;
        m_audioOut->init(settings);

        if (m_deviceControl) {
            m_deviceControl->setInput();
        }
    }
}
