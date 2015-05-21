#ifndef AUDIOOUT_AO_H
#define AUDIOOUT_AO_H

#include "audioout_abstract.h"

#include <ao/ao.h>


class AudioOutAo : public AudioOutAbstract
{
public:
    AudioOutAo();

    virtual const char *name() const Q_DECL_OVERRIDE;
    virtual bool init(const QSettings::SettingsMap &settings) Q_DECL_OVERRIDE;
    virtual void deinit() Q_DECL_OVERRIDE;
    virtual void start() Q_DECL_OVERRIDE;
    virtual void stop() Q_DECL_OVERRIDE;
    virtual void play(char *data, int samples) Q_DECL_OVERRIDE;

private:
    int         m_driverId;
    ao_device   *m_aoDevice;
    ao_option   *m_aoOptions;
};

#endif // AUDIOOUT_AO_H
