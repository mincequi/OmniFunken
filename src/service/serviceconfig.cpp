#include "serviceconfig.h"

ServiceConfig::ServiceConfig()
{
}

ServiceConfig::~ServiceConfig()
{
}

QString ServiceConfig::zeroconfName() const
{
    return "OmniFunken";
}

QString ServiceConfig::zeroconfType() const
{
    return "";
}
