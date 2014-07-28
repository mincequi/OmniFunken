#ifndef AUDIOOUT_ALSA_H
#define AUDIOOUT_ALSA_H

#include "audioout_abstract.h"

class AudioOutAlsa : public AudioOutAbstract
{
public:
    AudioOutAlsa();

    virtual const char *name() const;
    virtual void init();
    virtual void play(char *data, int samples);
    virtual void deinit();
};

#endif // AUDIOOUT_ALSA_H
