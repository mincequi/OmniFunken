#ifndef AUDIOOUT_AO_H
#define AUDIOOUT_AO_H

#include "audioout_abstract.h"

#include <QMutex>

#include <ao/ao.h>


class AudioOutAo : public AudioOutAbstract
{
public:
    AudioOutAo();

    virtual const char *name() const;
    virtual void init();
    virtual void play(char *data, int samples);
    virtual void deinit();
    virtual void setVolume(float volume);

private:
    ao_device   *m_aoDevice;
    float       m_volume;
    QMutex      m_mutex;
};

#endif // AUDIOOUT_AO_H
