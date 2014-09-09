#include <QCoreApplication>
#include <QCommandLineParser>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QSettings>

#include "audioout/audiooutfactory.h"
#include "audioout/audioout_abstract.h"
#include "daemon.h"
#include "devicecontrol/devicecontrolfactory.h"
#include "player.h"
#include "rtp/rtpbuffer.h"
#include "rtspserver.h"
#include "util.h"
#include "zeroconf/zeroconf_dns_sd.h"

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
    QString macAddress;
    foreach(QNetworkInterface networkInterface, QNetworkInterface::allInterfaces()) {
        // Return only the first non-loopback MAC Address
        if (!(networkInterface.flags() & QNetworkInterface::IsLoopBack)) {
            macAddress = networkInterface.hardwareAddress();
            if (isValidMacAddress(macAddress)) {
                break;
            }
        }
    }
    // TODO: mac address might be "00:00:00:00:00:00", which is illegal
    qDebug() << "MAC address: " << macAddress;

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
    AudioOutAbstract *audioOut = AudioOutFactory::createAudioOut(parser.value(audioOutOption).toLatin1(), audioSettings);
    QObject::connect(&a, &QCoreApplication::aboutToQuit, [audioOut]() { audioOut->deinit(); } );

    // init player
    Player      *player = new Player(rtpBuffer, audioOut);

    // wire components
    QObject::connect(rtspServer, SIGNAL(announce(RtspMessage::Announcement)), rtpReceiver, SLOT(announce(RtspMessage::Announcement)));
    QObject::connect(rtspServer, SIGNAL(senderSocketAvailable(RtpReceiver::PayloadType, quint16)), rtpReceiver, SLOT(setSenderSocket(RtpReceiver::PayloadType, quint16)));
    QObject::connect(rtspServer, SIGNAL(receiverSocketRequired(RtpReceiver::PayloadType, quint16*)), rtpReceiver, SLOT(bindSocket(RtpReceiver::PayloadType, quint16*)));
    QObject::connect(rtspServer, SIGNAL(record(quint16)), rtpBuffer, SLOT(flush(quint16)));
    QObject::connect(rtspServer, SIGNAL(flush(quint16)), rtpBuffer, SLOT(flush(quint16)));
    QObject::connect(rtspServer, &RtspServer::teardown, rtpBuffer, &RtpBuffer::teardown);
    QObject::connect(rtspServer, &RtspServer::teardown, rtpReceiver, &RtpReceiver::teardown);
    QObject::connect(rtspServer, &RtspServer::teardown, player, &Player::teardown);
    QObject::connect(rtspServer, &RtspServer::volume, player, &Player::setVolume);

    // init device control
    //DeviceControlAbstract *deviceControl = DeviceControlFactory::createDeviceControl("rs232");
    //deviceControl->init(deviceControlSettings);
    //QObject::connect(&a, &QCoreApplication::aboutToQuit, [deviceControl]() { deviceControl->stop(); deviceControl->deinit(); } );

    // startup
    rtspServer->listen(QHostAddress::AnyIPv4, parser.value(portOption).toInt());

    // register service
    ZeroconfDnsSd *dnsSd = new ZeroconfDnsSd(macAddress);
    dnsSd->registerService(parser.value(nameOption).toLatin1(), parser.value(portOption).toInt());

    return a.exec();
}
