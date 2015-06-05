#include "audioout_ao.h"
#include "audiooutfactory.h"

#include <QtMath>

AudioOutAo::AudioOutAo() :
    m_driverId(-1),
    m_aoDevice(NULL),
    m_aoOptions(NULL)
{
    AudioOutFactory::registerAudioOut(this);
    ao_append_option(&m_aoOptions, "buffer_time", "125");
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
        QByteArray driverName = it.value().toByteArray();
        if (!driverName.isEmpty()) {
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
    if (m_aoDevice) {
        ao_close(m_aoDevice);
        m_aoDevice = NULL;
    }

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
    ao_play(m_aoDevice, data, bytes);
}

static AudioOutAo s_instance;
