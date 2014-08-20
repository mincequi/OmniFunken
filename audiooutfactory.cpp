#include "audiooutfactory.h"

#include <QDebug>
#include <QMap>
#include <QThread>


class AudioOutDummy : public AudioOutAbstract {
public:
    virtual const char *name() const { return "dummy"; }
    virtual void play(char *data, int samples) { Q_UNUSED(data) Q_UNUSED(samples) QThread::sleep(1); }
};
Q_GLOBAL_STATIC(AudioOutDummy, dummy)

typedef QMap<QString, AudioOutAbstract*> registryType;
Q_GLOBAL_STATIC(registryType, registry)


void AudioOutFactory::registerAudioOut(AudioOutAbstract* audioOut)
{
    qDebug() << __func__ << ": " << audioOut->name();
    registry->insert(audioOut->name(), audioOut);
}

AudioOutAbstract* AudioOutFactory::createAudioOut(const QString &key)
{
    return registry->value(key, dummy);
}
