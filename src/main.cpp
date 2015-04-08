#include <QCoreApplication>
#include <QCommandLineParser>
#include <QHostInfo>
#include <QSettings>

#include "airtunes/airtunesserviceconfig.h"
#include "audioout/audiooutfactory.h"
#include "audioout/audioout_abstract.h"
#include "core/core.h"
#include "devicecontrol/devicecontrolabstract.h"
#include "devicecontrol/devicecontrolfactory.h"
#include "devicecontrol/devicewatcher.h"
#include "rtp/rtpbuffer.h"
#include "rtp/rtpreceiver.h"
#include "rtsp/rtspserver.h"
#include "service/service.h"
#include "zeroconf/zeroconf_dns_sd.h"
#include "daemon.h"
#include "player.h"
#include "signalhandler.h"
#include "util.h"

#include <unistd.h>
#include <sys/stat.h>

void onAnnounce(RtpReceiver* rtpReceiver, AudioOutAbstract* audioOut, const RtspMessage::Announcement& announcement)
{
    if (!audioOut->ready()) {
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
        audioOut->init(settings);
    }
    rtpReceiver->announce(announcement);
}

int main(int argc, char *argv[])
{
    // suppress avahi warning
    setenv("AVAHI_COMPAT_NOWARN", "1", 1);

    //qInstallMsgHandler(logOutput);
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("OmniFunken");
    QCoreApplication::setApplicationVersion("0.0.1");

    // command line options
    QCommandLineParser parser;
    parser.setApplicationDescription("OmniFunken aims to be a general purpose media render daemon.");
    parser.addVersionOption();
    parser.addHelpOption();
    //QCommandLineOption verboseOption(QStringList() << "v" << "verbose", "Set verbose mode.");
    //parser.addOption(verboseOption);
    QCommandLineOption daemonOption(QStringList() << "d" << "daemon", "Start as daemon.");
    parser.addOption(daemonOption);

    AirTunesServiceConfig airTunesServiceConfig;
    airTunesServiceConfig.parseCommandLine(parser);

    parser.process(a);

    if (parser.isSet(daemonOption)) {
        daemonize();
    }

    // init signal handler
    initSignalHandler();

    // init service
    Service *service = new Service(airTunesServiceConfig, qApp);
    service->open();
    QObject::connect(&a, &QCoreApplication::aboutToQuit, service, &Service::close);

    return a.exec();
}
