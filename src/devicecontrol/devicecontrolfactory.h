#ifndef DEVICECONTROLFACTORY_H
#define DEVICECONTROLFACTORY_H

#include <QString>

class DeviceControlAbstract;

class DeviceControlFactory
{
public:
    static DeviceControlAbstract* createDeviceControl(const QString &key);
    static void registerDeviceControl(DeviceControlAbstract* audioOut);
};


#endif // DEVICECONTROLFACTORY_H
