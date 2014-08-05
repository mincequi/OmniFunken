#ifndef AUDIOOUT_ALSA_H
#define AUDIOOUT_ALSA_H

#include "audioout_abstract.h"

#include <alsa/asoundlib.h>


class AudioOutAlsa : public AudioOutAbstract
{
public:
    AudioOutAlsa();

    virtual const char *name() const;
    virtual void init(const char *deviceName = NULL) override;
    virtual void play(char *data, int samples);
    virtual void deinit();

private:
    snd_pcm_t   *m_pcm;
    bool        m_block;
};

#endif // AUDIOOUT_ALSA_H
