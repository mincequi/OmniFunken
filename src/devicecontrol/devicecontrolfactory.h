#ifndef DEVICECONTROLFACTORY_H
#define DEVICECONTROLFACTORY_H

#include <QString>
#include <QSettings>

class DeviceControlAbstract;

class DeviceControlFactory
{
public:
    static DeviceControlAbstract* createDeviceControl(QSettings *settings);
    static void registerDeviceControl(DeviceControlAbstract* deviceControl);
};

#endif // DEVICECONTROLFACTORY_H
