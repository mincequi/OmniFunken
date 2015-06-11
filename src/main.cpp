#include <QCoreApplication>
#include <QCommandLineParser>

#include "daemon.h"
#include "signalhandler.h"
#include "airtunes/airtunesserviceconfig.h"
#include "core/core.h"
#include "service/service.h"

int main(int argc, char *argv[])
{
    // suppress avahi warning
    setenv("AVAHI_COMPAT_NOWARN", "1", 1);

    // install qt message handler
    //qInstallMsgHandler(logOutput);

    // init app
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("OmniFunken");
    QCoreApplication::setApplicationVersion("0.0.2");

    // command line options
    QCommandLineParser parser;
    parser.setApplicationDescription("OmniFunken aims to be a general purpose media render daemon.");
    parser.addVersionOption();
    parser.addHelpOption();
    //QCommandLineOption verboseOption(QStringList() << "v" << "verbose", "Set verbose mode.");
    //parser.addOption(verboseOption);
    QCommandLineOption daemonOption(QStringList() << "d" << "daemon", "Start as daemon.");
    parser.addOption(daemonOption);
    ofCore->parseCommandLine(parser);
    parser.process(a);

    // daemonize
    if (parser.isSet(daemonOption)) {
        daemonize();
    }

    // init signal handler
    initSignalHandler();

    // init service
    AirTunesServiceConfig airTunesServiceConfig;
    Service *service = new Service(airTunesServiceConfig, qApp);
    service->open();
    QObject::connect(&a, &QCoreApplication::aboutToQuit, service, &Service::close);

    return a.exec();
}
