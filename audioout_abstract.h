#ifndef AUDIOOUT_ABSTRACT_H
#define AUDIOOUT_ABSTRACT_H

#include <QObject>
#include <QSettings>

class AudioOutAbstract : public QObject
{
    Q_OBJECT
public:
    AudioOutAbstract() : QObject() {}

    // name of output plugin
    virtual const char *name() const = 0;

    // called at startup
    virtual void init(const QSettings::SettingsMap &settings) { Q_UNUSED(settings) }
    // called at shutdown
    virtual void deinit() {}

    // called before playing
    virtual void start() {}
    // called after playing
    virtual void stop() {}
    // play samples
    virtual void play(char *data, int samples) = 0;

    // if no volume control available, we apply soft volume
    virtual bool hasVolumeControl() { return false; }

public slots:
    virtual void setVolume(float volume) { Q_UNUSED(volume) }
};


#endif // AUDIOOUT_ABSTRACT_H
