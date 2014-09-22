#include "devicecontrolfactory.h"

#include "devicecontrolabstract.h"

#include <QDebug>
#include <QMap>


typedef QMap<QString, DeviceControlAbstract*> registryType;
Q_GLOBAL_STATIC(registryType, registry)


void DeviceControlFactory::registerDeviceControl(DeviceControlAbstract* deviceControl)
{
    qDebug() << __func__ << ": " << deviceControl->name();
    registry->insert(deviceControl->name(), deviceControl);
}

DeviceControlAbstract* DeviceControlFactory::createDeviceControl(const QString &key, const QSettings::SettingsMap &settings)
{
    DeviceControlAbstract* deviceControl = registry->value(key, NULL);
    if (!deviceControl || !deviceControl->init(settings)) {
        return NULL;
    } else {
        return deviceControl;
    }
}

