#ifndef AUDIOOUT_AO_H
#define AUDIOOUT_AO_H

#include "audioout_abstract.h"

#include <ao/ao.h>


class AudioOutAo : public AudioOutAbstract
{
public:
    AudioOutAo();

    virtual const char *name() const override;
    virtual bool init(const QSettings::SettingsMap &settings) override;
    virtual void deinit() override;
    virtual void start() override;
    virtual void stop() override;
    virtual void play(char *data, int samples) override;

private:
    int         m_driverId;
    ao_device   *m_aoDevice;
    ao_option   *m_aoOptions;
};

#endif // AUDIOOUT_AO_H
