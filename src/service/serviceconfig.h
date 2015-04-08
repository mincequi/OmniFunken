#ifndef SERVICECONFIG_H
#define SERVICECONFIG_H

#include <QCommandLineParser>
//#include <QString>

class ServiceConfig
{
public:
    enum CommandLineParseResult
    {
        CommandLineOk,
        CommandLineError
    };

public:
    ServiceConfig();
    ~ServiceConfig();

    CommandLineParseResult parseCommandLine(QCommandLineParser &parser, QString *errorMessage);

    QString name() const;
    quint16 port() const;
    quint16 latency() const;
    QString audioOut() const;
    QString audioDevice() const;

    virtual QString zeroconfType() const;

private:
    QString m_name;
    quint16 m_port;
    quint16 m_latency;
    QString m_audioOut;
    QString m_audioDevice;
};

#endif // SERVICECONFIG_H
