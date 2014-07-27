#include "audioout_ao.h"
#include "audiooutfactory.h"

AudioOutAo::AudioOutAo() :
    m_aoDevice(NULL)
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

void AudioOutAo::play(void *data, int samples)
{
    ao_play(m_aoDevice, reinterpret_cast<char*>(data), samples);
}

void AudioOutAo::deinit()
{
    if (m_aoDevice) {
        ao_close(m_aoDevice);
        m_aoDevice = NULL;
        ao_shutdown();
    }
}

static AudioOutAo s_instance;
