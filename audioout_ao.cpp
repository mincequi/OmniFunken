#include "audioout_ao.h"
#include "audiooutfactory.h"

AudioOutAo::AudioOutAo(QObject *parent) :
    AudioOutAbstract(parent),
    m_aoDevice(NULL),
    m_volume(1.0f)
{
    AudioOutFactory::registerAudioOut(this);
}

const char *AudioOutAo::name() const
{
    return "ao";
}

void AudioOutAo::init()
{
    if (!m_aoDevice) {
        ao_initialize();
        int driver = ao_default_driver_id();

        ao_sample_format format;
        memset(&format, 0, sizeof(format));

        format.bits = 16;
        format.rate = 44100;
        format.channels = 2;
        format.byte_format = AO_FMT_NATIVE;

        m_aoDevice = ao_open_live(driver, &format, NULL);
    }
}

void AudioOutAo::play(char *data, int samples)
{
    m_mutex.lock();
    int shift = abs(m_volume/5.625f);
    m_mutex.unlock();
    for (int i = 0; i < samples/2; ++i) {
        *(qint16 *)(data+(i*2)) >>= shift;
    }

    ao_play(m_aoDevice, data, samples);
}

void AudioOutAo::deinit()
{
    if (m_aoDevice) {
        ao_close(m_aoDevice);
        m_aoDevice = NULL;
        ao_shutdown();
    }
}

void AudioOutAo::setVolume(float volume)
{
    QMutexLocker locker(&m_mutex);
    m_volume = volume;
}

static AudioOutAo s_instance;
