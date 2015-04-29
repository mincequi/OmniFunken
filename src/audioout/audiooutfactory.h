#ifndef AUDIOOUTFACTORY_H
#define AUDIOOUTFACTORY_H

#include <QString>
#include <QSettings>

class AudioOutAbstract;

class AudioOutFactory
{
public:
    static AudioOutAbstract *createAudioOut(const QString &key);
    static void registerAudioOut(AudioOutAbstract* audioOut);
};

#endif // AUDIOOUTFACTORY_H
