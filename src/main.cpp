#include <QCoreApplication>
#include <QCommandLineParser>
#include <QHostInfo>
#include <QSettings>

#include "audioout/audiooutfactory.h"
#include "audioout/audioout_abstract.h"
#include "devicecontrol/devicecontrolabstract.h"
#include "devicecontrol/devicecontrolfactory.h"
#include "rtp/rtpbuffer.h"
#include "rtp/rtpreceiver.h"
#include "rtsp/rtspserver.h"
#include "zeroconf/zeroconf_dns_sd.h"
#include "daemon.h"
#include "player.h"
#include "util.h"

#include <unistd.h>
#include <sys/stat.h>


int main(int argc, char *argv[])
{
    // suppress avahi warning
    setenv("AVAHI_COMPAT_NOWARN", "1", 1);

    //qInstallMsgHandler(logOutput);
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("OmniFunken");
    QCoreApplication::setApplicationVersion("0.0.1");

    // MAC address
    QString macAddress = Util::getMacAddress();

    // settings
    QSettings settings("/etc/omnifunken.conf", QSettings::IniFormat);

    // audio settings
    QSettings::SettingsMap audioSettings;
    settings.beginGroup("audio_out");
    QStringList keys = settings.childKeys();
    for (const QString &key : keys) {
        audioSettings.insert(key, settings.value(key));
    }
    settings.endGroup();

    // command line options
    QString defaultName("OmniFunken@"); defaultName.append(QHostInfo::localHostName());
    QCommandLineParser parser;
    parser.setApplicationDescription("OmniFunken aims to be a general purpose media render daemon.");
    parser.addVersionOption();
    parser.addHelpOption();
    //QCommandLineOption verboseOption(QStringList() << "v" << "verbose", "Set verbose mode.");
    //parser.addOption(verboseOption);
    QCommandLineOption nameOption(QStringList() << "n" << "name", "Set propagated name.", "name", defaultName);
    parser.addOption(nameOption);
    QCommandLineOption portOption(QStringList() << "p" << "port", "Set RTSP port.", "port", "5002");
    parser.addOption(portOption);
    QCommandLineOption latencyOption(QStringList() << "l" << "latency", "Set latency in milliseconds.", "latency", "500");
    parser.addOption(latencyOption);
    QCommandLineOption audioOutOption(QStringList() << "a" << "audio", "Set audio backend.", "audio", "ao");
    parser.addOption(audioOutOption);
    QCommandLineOption audioDeviceOption(QStringList() << "ad" << "audiodevice", "Set audio device.", "audiodevice", "hw:0");
    parser.addOption(audioDeviceOption);
    QCommandLineOption daemonOption(QStringList() << "d" << "daemon", "Start as daemon.");
    parser.addOption(daemonOption);
    parser.process(a);

    if (parser.isSet(daemonOption)) {
        daemon_init();
    }

    // init rtsp/rtp components
    RtspServer  *rtspServer = new RtspServer(macAddress);
    RtpBuffer   *rtpBuffer = new RtpBuffer(parser.value(latencyOption).toInt());
    RtpReceiver *rtpReceiver = new RtpReceiver(rtpBuffer);

    // init audio driver
    AudioOutAbstract *audioOut = AudioOutFactory::createAudioOut(parser.value(audioOutOption).toLatin1(),
                                                                 parser.value(audioDeviceOption).toLatin1(),
                                                                 audioSettings);
    QObject::connect(&a, &QCoreApplication::aboutToQuit, [audioOut]() { audioOut->deinit(); } );

    // init player
    Player      *player = new Player(rtpBuffer, audioOut);

    // init device control
    DeviceControlAbstract *deviceControl = DeviceControlFactory::createDeviceControl(&settings);

    // wire components
    QObject::connect(rtspServer, SIGNAL(announce(RtspMessage::Announcement)), rtpReceiver, SLOT(announce(RtspMessage::Announcement)));
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
            //deviceControl->powerOn();
            //deviceControl->setInput();
        });
        QObject::connect(rtspServer, &RtspServer::volume, deviceControl, &DeviceControlAbstract::setVolume);
    } else {
        QObject::connect(rtspServer, &RtspServer::volume, player, &Player::setVolume);
    }

    // startup
    rtspServer->listen(QHostAddress::AnyIPv4, parser.value(portOption).toInt());

    // register service
    ZeroconfDnsSd *dnsSd = new ZeroconfDnsSd(macAddress);
    dnsSd->registerService(parser.value(nameOption).toLatin1(), parser.value(portOption).toInt());

    return a.exec();
}
