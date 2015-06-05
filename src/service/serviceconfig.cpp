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
    //    QCommandLineOption audioOutOption(QStringList() << "ao" << "audioout", "Set audio backend.", "audioout", "ao");
    //    parser.addOption(audioOutOption);
    //    QCommandLineOption audioDeviceOption(QStringList() << "ad" << "audiodevice", "Set audio device.", "audiodevice", "hw:0");
    //    parser.addOption(audioDeviceOption);

    if (!parser.parse(QCoreApplication::arguments())) {
        if (errorMessage) *errorMessage = parser.errorText();
        return CommandLineError;
    }

    m_name = parser.value(nameOption);
    m_port = parser.value(portOption).toInt();
    m_latency = parser.value(latencyOption).toInt();
//    m_audioOut = parser.value(audioOutOption);
//    m_audioDevice = parser.value(audioDeviceOption);

    qDebug()<<Q_FUNC_INFO<<"name:"<<m_name<<"port:"<<m_port<<"latency:"<<m_latency;
//             << ", audioOut: " << m_audioOut
//             << ", audioDevice: " << m_audioDevice;

    return CommandLineOk;
}

QString ServiceConfig::name() const
{
    return m_name;
}

quint16 ServiceConfig::port() const
{
    return m_port;
}

quint16 ServiceConfig::latency() const
{
    return m_latency;
}

//QString ServiceConfig::audioOut() const
//{
//    return m_audioOut;
//}

//QString ServiceConfig::audioDevice() const
//{
//    return m_audioDevice;
//}

QString ServiceConfig::zeroconfType() const
{
    return "";
}
