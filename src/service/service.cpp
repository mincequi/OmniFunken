#include "service.h"

Service::Service(const ServiceConfig &serviceConfig, QObject *parent) :
    QObject(parent),
    m_serviceConfig(serviceConfig)
{

}

Service::~Service()
{

}

void Service::open()
{
    initAudioOut();
    initDeviceControl();
    initNetwork();
    initZeroconf();
}

void Service::close()
{
    deinitZeroconf();
    deinitNetwork();
    deinitDeviceControl();
    deinitAudioOut();
}

void Service::initAudioOut()
{

}

void Service::deinitAudioOut()
{

}

void Service::initDeviceControl()
{

}

void Service::deinitDeviceControl()
{

}

void Service::initNetwork()
{

}

void Service::deinitNetwork()
{

}

void Service::initZeroconf()
{

}

void Service::deinitZeroconf()
{

}
