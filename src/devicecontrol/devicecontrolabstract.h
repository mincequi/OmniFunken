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
    virtual bool init(const QSettings::SettingsMap &settings) { Q_UNUSED(settings) return true; }
    // called at shutdown
    virtual void deinit() {}

    // open device
    virtual void open() {}
    // close device
    virtual void close() {}

public slots:
    virtual void powerOn() {}
    virtual void powerOff() {}
    virtual void setInput() {}
    virtual void setVolume(float volume) { Q_UNUSED(volume) }

};

#endif // DEVICECONTROLABSTRACT_H
