#ifndef AUDIOOUTJACK_H
#define AUDIOOUTJACK_H

#include "audioout_abstract.h"
#include <airtunes/airtunesconstants.h>
#include <jack/types.h>
#include <jack/ringbuffer.h>
#include <QWaitCondition>


class AudioOutJack : public AudioOutAbstract
{
public:
    AudioOutJack();
    ~AudioOutJack();

    virtual const char *name() const Q_DECL_OVERRIDE;
    virtual bool init(const QSettings::SettingsMap &settings) Q_DECL_OVERRIDE;
    virtual void deinit() Q_DECL_OVERRIDE;
    virtual void stop() Q_DECL_OVERRIDE;
    virtual void play(char *data, int samples) Q_DECL_OVERRIDE;

private:
    void doStart();

    static int  onProcess(jack_nframes_t nframes, void *arg);
    static void onShutdown(void *arg);
    static void onError(const char *message);

    jack_client_t       *m_client;
    jack_port_t         *m_ports[airtunes::channels];
    jack_ringbuffer_t   *m_buffers[airtunes::channels];
    QStringList         m_destinationPorts;
    bool                m_portsConnected;

    QMutex          m_mutex;
    QWaitCondition  m_waitCondition;
};

#endif // AUDIOOUTJACK_H
