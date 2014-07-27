#ifndef AUDIOOUT_AO_H
#define AUDIOOUT_AO_H

#include "audioout_abstract.h"

#include <ao/ao.h>

class AudioOutAo : public AudioOutAbstract
{
public:
    AudioOutAo();

    virtual const char *name() const;
    virtual void init();
    virtual void play(void *data, int samples);
    virtual void deinit();

private:
    ao_device *m_aoDevice;

};

#endif // AUDIOOUT_AO_H
