#ifndef AUDIOOUTFACTORY_H
#define AUDIOOUTFACTORY_H

#include "audioout_abstract.h"


class AudioOutFactory
{
protected:
    AudioOutFactory() {}
    ~AudioOutFactory() {}

public:
    static AudioOutAbstract *createAudioOut(const QString &key, const QSettings::SettingsMap &settings);
    static void registerAudioOut(AudioOutAbstract* audioOut);
};

#endif // AUDIOOUTFACTORY_H
