#ifndef AUDIOOUT_ABSTRACT_H
#define AUDIOOUT_ABSTRACT_H

#include <QSettings>

class AudioOutAbstract
{
friend class Core;

public:
    AudioOutAbstract() {}
    virtual ~AudioOutAbstract() { deinit(); }

    // name of output plugin
    virtual const char *name() const = 0;

    // set device
    virtual void setDevice(const QString &device) { Q_UNUSED(device) }

    // called before playing
    virtual void start() {}
    // called after playing
    virtual void stop() {}
    // play samples
    virtual void play(char *data, int bytes) = 0;

    // if no volume control available, we apply soft volume
    virtual bool hasVolumeControl() { return false; }
    virtual void setVolume(float volume) { Q_UNUSED(volume) }

protected:
    // called at startup
    virtual bool init(const QSettings::SettingsMap &settings) { Q_UNUSED(settings) return true; }
    // Returns true if device is ready/init succeeded.
    virtual bool ready() { return true; }
    // called at shutdown
    virtual void deinit() {}
};


#endif // AUDIOOUT_ABSTRACT_H
