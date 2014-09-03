#ifndef AUDIOOUT_ALSA_H
#define AUDIOOUT_ALSA_H

#include "audioout_abstract.h"

#include <alsa/asoundlib.h>


class AudioOutAlsa : public AudioOutAbstract
{
public:
    AudioOutAlsa();

    virtual const char *name() const override;
    virtual bool init(const QSettings::SettingsMap &settings) override;
    virtual void deinit() override;
    virtual void start() override;
    virtual void stop() override;
    virtual void play(char *data, int bytes) override;
    virtual bool hasVolumeControl() override;
    virtual void setVolume(float volume) override;

private:
    bool probeNativeFormat();

    char        *m_deviceName;
    snd_pcm_t   *m_pcm;
    bool        m_block;
    snd_pcm_format_t    m_format;
    unsigned int        m_rate;
    
    bool    m_bitAccurate;
};

#endif // AUDIOOUT_ALSA_H
