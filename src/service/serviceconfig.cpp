#include "serviceconfig.h"

#include <QHostInfo>

ServiceConfig::ServiceConfig() :
    m_port(0),
    m_latency(0)
{
}

ServiceConfig::~ServiceConfig()
{
}

ServiceConfig::CommandLineParseResult ServiceConfig::parseCommandLine(QCommandLineParser &parser, QString *errorMessage)
{
    QString defaultName("OmniFunken@"); defaultName.append(QHostInfo::localHostName());
    QCommandLineOption nameOption(QStringList() << "n" << "name", "Set propagated name.", "name", defaultName);
    parser.addOption(nameOption);
    QCommandLineOption portOption(QStringList() << "p" << "port", "Set RTSP port.", "port", "5002");
    parser.addOption(portOption);
    QCommandLineOption latencyOption(QStringList() << "l" << "latency", "Set latency in milliseconds.", "latency", "500");
    parser.addOption(latencyOption);
    QCommandLineOption audioOutOption(QStringList() << "ao" << "audioout", "Set audio backend.", "audioout", "ao");
    parser.addOption(audioOutOption);
    QCommandLineOption audioDeviceOption(QStringList() << "ad" << "audiodevice", "Set audio device.", "audiodevice", "hw:0");
    parser.addOption(audioDeviceOption);

    m_name = parser.value(nameOption);
    m_port = parser.value(portOption).toInt();
    m_latency = parser.value(latencyOption).toInt();
    m_audioOut = parser.value(audioOutOption);
    m_audioDevice = parser.value(audioDeviceOption);
}

QString ServiceConfig::name() const
{
    return "OmniFunken";
}

quint16 ServiceConfig::port() const
{
}

quint16 ServiceConfig::latency() const
{
}

QString ServiceConfig::audioOut() const
{
}

QString ServiceConfig::audioDevice() const
{
}

QString ServiceConfig::zeroconfType() const
{
    return "";
}
