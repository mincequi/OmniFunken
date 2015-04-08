#include <QCoreApplication>
#include <QCommandLineParser>

#include "airtunes/airtunesserviceconfig.h"
#include "service/service.h"
#include "daemon.h"
#include "signalhandler.h"

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
