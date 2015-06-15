#include "audioout_pipe.h"
#include "audiooutfactory.h"

#include <ao/ao.h>

AudioOutPipe::AudioOutPipe() :
    m_aoDevice(NULL),
    m_aoOptions(NULL)
{
    m_driverId = ao_driver_id("raw");

    // stdout is expected to be little-endian generally.
    ao_append_option(&m_aoOptions, "byteorder", "little");

    AudioOutFactory::registerAudioOut(this);
}

const char *AudioOutPipe::name() const
{
    return "pipe";
}

bool AudioOutPipe::init(const QSettings::SettingsMap &settings)
{
    Q_UNUSED(settings)

    ao_initialize();

// MACOS wants the audio device to be opened from the main thread
#ifdef Q_OS_MAC
    if (!m_aoDevice) {
        ao_sample_format format;
        memset(&format, 0, sizeof(format));

        format.bits = 16;
        format.rate = 44100;
        format.channels = 2;
        format.byte_format = AO_FMT_NATIVE;

        m_aoDevice = ao_open_file(m_driverId, "-" /*stdout*/, 1 /*overwrite*/, &format, m_aoOptions);
    }
#endif

    return true;
}

void AudioOutPipe::deinit()
{
    if (m_aoDevice) {
        ao_close(m_aoDevice);
        m_aoDevice = NULL;
    }

    ao_shutdown();
}

void AudioOutPipe::start()
{
#ifndef Q_OS_MAC
    if (!m_aoDevice) {
        ao_sample_format format;
        memset(&format, 0, sizeof(format));

        format.bits = 16;
        format.rate = 44100;
        format.channels = 2;
        format.byte_format = AO_FMT_NATIVE;

        m_aoDevice = ao_open_file(m_driverId, "-" /*stdout*/, 1 /*overwrite*/, &format, m_aoOptions);
    }
#endif
}

void AudioOutPipe::stop()
{
#ifndef Q_OS_MAC
    if (m_aoDevice) {
        ao_close(m_aoDevice);
        m_aoDevice = NULL;
    }
#endif
}

void AudioOutPipe::play(char *data, int bytes)
{
    ao_play(m_aoDevice, data, bytes);
}

/*
bool AudioOutPipe::init(const QSettings::SettingsMap &settings)
{
    Q_UNUSED(settings)

    return true;
}

void AudioOutPipe::deinit()
{
}

void AudioOutPipe::start()
{
    PipeOutput *pd = (PipeOutput *)ao;

    pd->fh = popen(pd->cmd.c_str(), "w");
    if (pd->fh == nullptr) {
        qFatal("Error opening pipe \"%s\"", pd->cmd.c_str());
    }
}

void AudioOutPipe::stop()
{
    PipeOutput *pd = (PipeOutput *)ao;

    pclose(pd->fh);
}

void AudioOutPipe::play(char *data, int bytes)
{
    PipeOutput *pd = (PipeOutput *)ao;
    size_t ret;

    ret = fwrite(chunk, 1, size, pd->fh);
    if (ret == 0) {
        qWarning("Write error on pipe");
    }
}
*/

static AudioOutPipe s_instance;
