#ifndef AUDIOFILTERABSTRACT_H
#define AUDIOFILTERABSTRACT_H

#include <QSettings>

class AudioFilterAbstract
{
public:
    AudioFilterAbstract() {}
    virtual ~AudioFilterAbstract() {}

    // name of output plugin
    virtual const char *name() const = 0;

    // called at startup
    virtual bool init(const QString &settingsGroup, QSettings *settings)
    {
        Q_UNUSED(settingsGroup)
        Q_UNUSED(settings)
        return true;
    }
    // called at shutdown
    virtual void deinit() {}

    // called before playing
    virtual void start() {}
    // called after playing
    virtual void stop() {}
    // process samples
    virtual void process(char *data, int bytes) = 0;
};

#endif // AUDIOFILTERABSTRACT_H
