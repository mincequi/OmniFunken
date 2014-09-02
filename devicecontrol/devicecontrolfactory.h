#ifndef DEVICECONTROLFACTORY_H
#define DEVICECONTROLFACTORY_H


#include "devicecontrolabstract.h"


class DeviceControlFactory
{
protected:
    DeviceControlFactory() {}
    ~DeviceControlFactory() {}

public:
    static DeviceControlAbstract *createDeviceControl(const QString &key);
    static void registerDeviceControl(DeviceControlAbstract* audioOut);
};


#endif // DEVICECONTROLFACTORY_H
