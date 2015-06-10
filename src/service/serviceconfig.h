#ifndef SERVICECONFIG_H
#define SERVICECONFIG_H

#include <QCommandLineParser>

class ServiceConfig
{
public:
    ServiceConfig();
    ~ServiceConfig();

    virtual QString zeroconfType() const;
};

#endif // SERVICECONFIG_H
