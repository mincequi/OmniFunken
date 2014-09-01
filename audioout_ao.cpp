#include "audioout_ao.h"
#include "audiooutfactory.h"

#include <QtMath>

AudioOutAo::AudioOutAo() :
    m_driverId(-1),
    m_aoDevice(NULL),
    m_aoOptions(NULL)
{
    AudioOutFactory::registerAudioOut(this);
}

const char *AudioOutAo::name() const
{
    return "ao";
}

bool AudioOutAo::init(const QSettings::SettingsMap &settings)
{
    ao_initialize();

    auto it = settings.find("driver");
    if (it != settings.end()) {
        const char *driverName = it.value().toByteArray();
        if (driverName) {
            m_driverId = ao_driver_id(driverName);
        } else {
            m_driverId = ao_default_driver_id();
        }
    } else {
        m_driverId = ao_default_driver_id();
    }

    for (auto it = settings.constBegin(); it != settings.constEnd(); ++it) {
        ao_append_option(&m_aoOptions, it.key().toLatin1(), it.value().toByteArray());
    }

// MACOS wants the audio device to be opened from the main thread
#ifdef Q_OS_MAC
    if (!m_aoDevice) {
        ao_sample_format format;
        memset(&format, 0, sizeof(format));

        format.bits = 16;
        format.rate = 44100;
        format.channels = 2;
        format.byte_format = AO_FMT_NATIVE;

        m_aoDevice = ao_open_live(m_driverId, &format, m_aoOptions);
    }
#endif

    return true;
}

void AudioOutAo::deinit()
{
#ifdef Q_OS_MAC
    if (m_aoDevice) {
        ao_close(m_aoDevice);
        m_aoDevice = NULL;
    }
#endif
    ao_shutdown();
}

void AudioOutAo::start()
{
#ifndef Q_OS_MAC
    if (!m_aoDevice) {
        ao_sample_format format;
        memset(&format, 0, sizeof(format));

        format.bits = 16;
        format.rate = 44100;
        format.channels = 2;
        format.byte_format = AO_FMT_NATIVE;

        m_aoDevice = ao_open_live(m_driverId, &format, m_aoOptions);
    }
#endif
}

void AudioOutAo::stop()
{
#ifndef Q_OS_MAC
    if (m_aoDevice) {
        ao_close(m_aoDevice);
        m_aoDevice = NULL;
    }
#endif
}

void AudioOutAo::play(char *data, int bytes)
{
    //bytes *= 1000;
    char *samples = new char[bytes];

    for(int i = 0; i < bytes/4; ++i) {
        *(char*)(samples+(i*4)) = 0; //*(char*)(data+(i*4)); //32750 * sin( (2.f*float(M_PI)*440)/44100 * i*2 );
        *(char*)(samples+(i*4+1)) = *(char*)(data+(i*4+1));
        *(char*)(samples+(i*4+2)) = 0; //*(char*)(data+(i*4+2));//*(samples+(i*4));
        *(char*)(samples+(i*4+3)) = 0;//*(char*)(data+(i*4+3));
    }

    ao_play(m_aoDevice, samples, bytes);

    delete[] samples;
}

static AudioOutAo s_instance;
