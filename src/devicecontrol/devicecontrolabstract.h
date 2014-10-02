#ifndef DEVICECONTROLABSTRACT_H
#define DEVICECONTROLABSTRACT_H

#include <QObject>
#include <QSettings>

class DeviceControlAbstract : public QObject
{
    Q_OBJECT
public:
    DeviceControlAbstract() : QObject() {}

    // name of control plugin
    virtual const char *name() const = 0;

    // called at startup
    virtual bool init(const QString &device, const QString &settingsGroup, QSettings *settings) = 0;
    // called at shutdown
    virtual void deinit() {}

public slots:
    // open device
    virtual void open() {}
    // close device
    virtual void close() {}

    virtual void powerOn() {}
    virtual void powerOff() {}
    virtual void setInput() {}
    virtual void setVolume(float volume) { Q_UNUSED(volume) }

};

#endif // DEVICECONTROLABSTRACT_H
