#include "airtunesserviceconfig.h"

AirTunesServiceConfig::AirTunesServiceConfig()
{
}

AirTunesServiceConfig::~AirTunesServiceConfig()
{
}

QString AirTunesServiceConfig::zeroconfType() const
{
    return "_raop._tcp";
}
