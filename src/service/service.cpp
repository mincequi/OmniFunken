#include "service.h"

#include "audioout/audioout_abstract.h"
#include "audioout/audiooutfactory.h"
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

void Service::initAudioOut()
{
    // init audio driver
    AudioOutAbstract *m_audioOut = AudioOutFactory::createAudioOut(config().audioOut(),
                                                                 config().audioDevice(),
                                                                 audioSettings);

    // init rtsp/rtp components
    RtspServer  *rtspServer = new RtspServer(macAddress);
    RtpBuffer   *rtpBuffer = new RtpBuffer(config().latency());
    RtpReceiver *rtpReceiver = new RtpReceiver(rtpBuffer);

    // init player
    Player      *player = new Player(rtpBuffer, m_audioOut);

    // init device control
    DeviceControlAbstract *deviceControl = DeviceControlFactory::createDeviceControl(&settings);

    // wire components
    //QObject::connect(rtspServer, SIGNAL(announce(RtspMessage::Announcement)), rtpReceiver, SLOT(announce(RtspMessage::Announcement)));
    QObject::connect(rtspServer, &RtspServer::senderSocketAvailable, rtpReceiver, &RtpReceiver::setSenderSocket);
    QObject::connect(rtspServer, &RtspServer::receiverSocketRequired, rtpReceiver, &RtpReceiver::bindSocket);
    QObject::connect(rtspServer, SIGNAL(record(quint16)), rtpBuffer, SLOT(flush(quint16)));
    QObject::connect(rtspServer, SIGNAL(flush(quint16)), rtpBuffer, SLOT(flush(quint16)));
    QObject::connect(rtspServer, &RtspServer::teardown, rtpBuffer, &RtpBuffer::teardown);
    QObject::connect(rtspServer, &RtspServer::teardown, rtpReceiver, &RtpReceiver::teardown);
    QObject::connect(rtspServer, &RtspServer::teardown, player, &Player::teardown);

    if (deviceControl) {
        QObject::connect(&a, &QCoreApplication::aboutToQuit, [deviceControl]() { deviceControl->deinit(); } );
        QObject::connect(rtspServer, &RtspServer::announce, [deviceControl]() {
            qDebug("open deviceControl");
            deviceControl->open();
            qDebug("deviceControl opened");
            deviceControl->powerOn();
            //deviceControl->setInput();
        });
        QObject::connect(rtspServer, &RtspServer::volume, deviceControl, &DeviceControlAbstract::setVolume);
    } else {
        QObject::connect(rtspServer, &RtspServer::volume, player, &Player::setVolume);
    }

    // QObject::connect(rtspServer, &RtspServer::announce, [rtpReceiver, m_audioOut](const RtspMessage::Announcement& announcement) {
    //      onAnnounce(rtpReceiver, m_audioOut, announcement);
    // });
    QObject::connect(rtspServer, SIGNAL(announce(RtspMessage::Announcement)), rtpReceiver, SLOT(announce(RtspMessage::Announcement)));

    // startup
    rtspServer->listen(QHostAddress::AnyIPv4, parser.value(portOption).toInt());
}

void Service::deinitAudioOut()
{
}

void Service::initDeviceControl()
{

}

void Service::deinitDeviceControl()
{

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
    ZeroconfDnsSd *dnsSd = new ZeroconfDnsSd(macAddress);
    dnsSd->registerService(parser.value(nameOption).toLatin1(), parser.value(portOption).toInt());
}

void Service::deinitZeroconf()
{

}
