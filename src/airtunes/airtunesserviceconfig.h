#ifndef AIRTUNESSERVICECONFIG_H
#define AIRTUNESSERVICECONFIG_H

#include "service/serviceconfig.h"

class AirTunesServiceConfig : public ServiceConfig
{
public:
    AirTunesServiceConfig();
    ~AirTunesServiceConfig();

    virtual QString zeroconfType() const override;
};

#endif // AIRTUNESSERVICECONFIG_H
