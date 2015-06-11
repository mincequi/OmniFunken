#ifndef AUDIOOUT_ALSA_H
#define AUDIOOUT_ALSA_H

#include "audioout_abstract.h"

#include <alsa/asoundlib.h>


class AudioOutAlsa : public AudioOutAbstract
{
public:
    AudioOutAlsa();

private:
    virtual const char *name() const Q_DECL_OVERRIDE;
    virtual void setDevice(const QString &device) Q_DECL_OVERRIDE;
    virtual bool init(const QSettings::SettingsMap &settings) Q_DECL_OVERRIDE;
    virtual bool ready() Q_DECL_OVERRIDE;
    virtual void deinit() Q_DECL_OVERRIDE;
    virtual void start() Q_DECL_OVERRIDE;
    virtual void stop() Q_DECL_OVERRIDE;
    virtual void play(char *data, int bytes) Q_DECL_OVERRIDE;
    virtual bool hasVolumeControl() Q_DECL_OVERRIDE;
    virtual void setVolume(float volume) Q_DECL_OVERRIDE;

    bool probeNativeFormat();

    QString	m_deviceName;
    bool    m_ready;
    snd_pcm_t   *m_pcm;
    bool        m_block;

    float m_volume;
};

#endif // AUDIOOUT_ALSA_H
