#ifndef AUDIOOUTPIPE_H
#define AUDIOOUTPIPE_H

#include "audioout_abstract.h"

struct ao_device;
struct ao_option;


class AudioOutPipe : public AudioOutAbstract
{
public:
    AudioOutPipe();

private:
    virtual const char *name() const Q_DECL_OVERRIDE;
    virtual bool init(const QSettings::SettingsMap &settings) Q_DECL_OVERRIDE;
    virtual void deinit() Q_DECL_OVERRIDE;
    virtual void start() Q_DECL_OVERRIDE;
    virtual void stop() Q_DECL_OVERRIDE;
    virtual void play(char *data, int samples) Q_DECL_OVERRIDE;

    ao_device   *m_aoDevice;
    ao_option   *m_aoOptions;
    int         m_driverId;

    int         m_fd;
};

#endif // AUDIOOUTPIPE_H
