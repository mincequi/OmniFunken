#include "devicecontrolfactory.h"

#include <QDebug>
#include <QMap>


/*
class AudioOutDummy : public AudioOutAbstract {
public:
    virtual const char *name() const { return "dummy"; }
    virtual void play(char *data, int samples) { Q_UNUSED(data) Q_UNUSED(samples) QThread::sleep(1); }
};
Q_GLOBAL_STATIC(AudioOutDummy, dummy)
*/

typedef QMap<QString, DeviceControlAbstract*> registryType;
Q_GLOBAL_STATIC(registryType, registry)


void DeviceControlFactory::registerDeviceControl(DeviceControlAbstract* deviceControl)
{
    qDebug() << __func__ << ": " << deviceControl->name();
    registry->insert(deviceControl->name(), deviceControl);
}

DeviceControlAbstract* DeviceControlFactory::createDeviceControl(const QString &key)
{
    return registry->value(key, NULL);
}

