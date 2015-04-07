#ifndef SERVICECONFIG_H
#define SERVICECONFIG_H

#include <QString>

class ServiceConfig
{
public:
    ServiceConfig();
    ~ServiceConfig();

    virtual QString zeroconfName() const;
    virtual QString zeroconfType() const;
};

#endif // SERVICECONFIG_H
