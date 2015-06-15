#include "audioout_pipe.h"
#include "audiooutfactory.h"

#define _GNU_SOURCE

#include <fcntl.h>
#include <stdio.h>
#include <ao/ao.h>

#ifndef F_LINUX_SPECIFIC_BASE
#define F_LINUX_SPECIFIC_BASE       1024
#endif
#ifndef F_SETPIPE_SZ
#define F_SETPIPE_SZ	(F_LINUX_SPECIFIC_BASE + 7)
#endif

AudioOutPipe::AudioOutPipe() :
    m_aoDevice(NULL),
    m_aoOptions(NULL),
    m_fd(-1)
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

//bool AudioOutPipe::init(const QSettings::SettingsMap &settings)
//{
//    Q_UNUSED(settings)

//    ao_initialize();

//// MACOS wants the audio device to be opened from the main thread
//#ifdef Q_OS_MAC
//    if (!m_aoDevice) {
//        ao_sample_format format;
//        memset(&format, 0, sizeof(format));

//        format.bits = 16;
//        format.rate = 44100;
//        format.channels = 2;
//        format.byte_format = AO_FMT_NATIVE;

//        m_aoDevice = ao_open_file(m_driverId, "-" /*stdout*/, 1 /*overwrite*/, &format, m_aoOptions);
//    }
//#endif

//    return true;
//}

//void AudioOutPipe::deinit()
//{
//    if (m_aoDevice) {
//        ao_close(m_aoDevice);
//        m_aoDevice = NULL;
//    }

//    ao_shutdown();
//}

//void AudioOutPipe::start()
//{
//#ifndef Q_OS_MAC
//    if (!m_aoDevice) {
//        ao_sample_format format;
//        memset(&format, 0, sizeof(format));

//        format.bits = 16;
//        format.rate = 44100;
//        format.channels = 2;
//        format.byte_format = AO_FMT_NATIVE;

//        m_aoDevice = ao_open_file(m_driverId, "-" /*stdout*/, 1 /*overwrite*/, &format, m_aoOptions);
//    }
//#endif
//}

//void AudioOutPipe::stop()
//{
//#ifndef Q_OS_MAC
//    if (m_aoDevice) {
//        ao_close(m_aoDevice);
//        m_aoDevice = NULL;
//    }
//#endif
//}

//void AudioOutPipe::play(char *data, int bytes)
//{
//    ao_play(m_aoDevice, data, bytes);
//}

bool AudioOutPipe::init(const QSettings::SettingsMap &settings)
{
    Q_UNUSED(settings)

    if (m_fd >= 0) {
        stop();
    }

    m_fd = open("/dev/stdout", O_WRONLY | O_NONBLOCK);
    if ((m_fd < 0) && (errno != ENXIO)) {
        qFatal("error opening pipe");
    }

    // The other end is ready, reopen with blocking
    if (m_fd >= 0) {
        close(m_fd);
        m_fd = open("/dev/stdout", O_WRONLY);
        fcntl(F_SETPIPE_SZ, 4096);
    }

    return true;
}

void AudioOutPipe::deinit()
{
    if (m_fd >= 0) {
        close(m_fd);
        m_fd = -1;
    }
}

void AudioOutPipe::start()
{
}

void AudioOutPipe::stop()
{
}

void AudioOutPipe::play(char *data, int bytes)
{
    write(m_fd, data, bytes);
}

static AudioOutPipe s_instance;
