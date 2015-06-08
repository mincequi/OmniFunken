#include "devicecontrolfactory.h"

#include "devicecontrolabstract.h"

#include <QDebug>
#include <QMap>


typedef QMap<QString, DeviceControlAbstract*> registryType;
Q_GLOBAL_STATIC(registryType, registry)


void DeviceControlFactory::registerDeviceControl(DeviceControlAbstract* deviceControl)
{
    qDebug()<<Q_FUNC_INFO<<deviceControl->name();
    registry->insert(deviceControl->name(), deviceControl);
}

DeviceControlAbstract* DeviceControlFactory::createDeviceControl(QSettings *settings)
{
    settings->beginGroup("device_control");
    QString type = settings->value("type", "").toString();
    QString device = settings->value("device", "/dev/ttyUSB0").toString();
    QString config = settings->value("config", "").toString();
    settings->endGroup();

    if (type.isEmpty() || config.isEmpty()) {
        return NULL;
    }

    QString configGroup = "device_control_";
    configGroup += config;

    DeviceControlAbstract* deviceControl = registry->value(type, NULL);
    if (!deviceControl || !deviceControl->init(device, configGroup, settings)) {
        return NULL;
    } else {
        return deviceControl;
    }
}

