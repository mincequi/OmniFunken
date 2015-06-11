#include "audioout_pipe.h"
#include "audiooutfactory.h"

#include <thread>

AudioOutPipe::AudioOutPipe()
{
    AudioOutFactory::registerAudioOut(this);
}

const char *AudioOutPipe::name() const
{
    return "pipe";
}

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
}

void AudioOutPipe::stop()
{
}

void AudioOutPipe::play(char *data, int bytes)
{
    Q_UNUSED(data)
    Q_UNUSED(bytes)

    //std::this_thread::sleep_for(std::chrono::nanoseconds(7981859));
    std::this_thread::sleep_for(std::chrono::microseconds(7982));
}

static AudioOutPipe s_instance;
