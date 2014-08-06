#ifndef AUDIOOUT_ALSA_H
#define AUDIOOUT_ALSA_H

#include "audioout_abstract.h"

#include <alsa/asoundlib.h>


class AudioOutAlsa : public AudioOutAbstract
{
public:
    AudioOutAlsa();

    virtual const char *name() const override;
    virtual void init(const QSettings::SettingsMap &settings) override;
    virtual void deinit() override;
    virtual void start() override;
    virtual void stop() override;
    virtual void play(char *data, int samples) override;

private:
    char        *m_deviceName;
    snd_pcm_t   *m_pcm;
    bool        m_block;
};

#endif // AUDIOOUT_ALSA_H
