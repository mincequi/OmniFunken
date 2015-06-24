#include "audioout_jack.h"
#include "audiooutfactory.h"

#include <jack/jack.h>
#include <jack/ringbuffer.h>
#include <QDebug>

typedef jack_default_audio_sample_t sample_t;

AudioOutJack::AudioOutJack() :
    m_client(NULL),
    m_portsConnected(false)
{
    for (uint i = 0; i < airtunes::channels; ++i) {
        m_ports[i] = NULL;
        m_buffers[i] = NULL;
    }
    AudioOutFactory::registerAudioOut(this);
}

AudioOutJack::~AudioOutJack()
{
}

const char *AudioOutJack::name() const
{
    return "jack";
}

bool AudioOutJack::init(const QSettings::SettingsMap &settings)
{
    qDebug()<<Q_FUNC_INFO;

    m_destinationPorts = settings["device"].toString().split(",");

    if (m_client) {
        return true;
    }

    // set error callback
    jack_set_error_function(onError);

    const char *clientName = "omnifunken";
    jack_options_t options = JackNullOption;
    jack_status_t status;

    // open a client connection to the JACK server
    m_client = jack_client_open(clientName, options, &status, NULL);
    if (m_client == NULL) {
        qWarning()<<Q_FUNC_INFO<<"failed opening jack client: "<<status;
        return false;
    }

    // set callbacks
    jack_set_process_callback(m_client, onProcess, this);
    jack_on_shutdown(m_client, onShutdown, this);

    // register our ports, reserve buffers
    for (uint i = 0; i < airtunes::channels; ++i) {
        m_ports[i] = jack_port_register(m_client, QString("output_%1").arg(i+1).toLatin1(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
        // We want to keep 1024 samples with 4 bytes each. We reserve four times as much.
        m_buffers[i] = jack_ringbuffer_create(16384);
    }

    return true;
}

void AudioOutJack::deinit()
{
    if (!m_client) {
        return;
    }

    qDebug()<<Q_FUNC_INFO;

    for (uint i = 0; i < airtunes::channels; ++i) {
        jack_port_unregister(m_client, m_ports[i]);
        jack_ringbuffer_free(m_buffers[i]);
        m_ports[i] = NULL;
        m_buffers[i] = NULL;
    }

    jack_client_close(m_client);
    m_client = NULL;
}

void AudioOutJack::stop()
{
    qDebug()<<Q_FUNC_INFO;

    if (jack_deactivate(m_client)) {
        qWarning()<<Q_FUNC_INFO<<"cannot deactivate client";
    }
    m_portsConnected = false;
}

void AudioOutJack::play(char *data, int bytes)
{
    // we always expect int16 samples
    int16_t *inSamples = (int16_t*)data;
    size_t  numSamples = bytes/2;
    size_t  bytesPerChannel = numSamples*sizeof(sample_t)/airtunes::channels;

    // find minimum available size
    size_t availableWrite = SIZE_MAX;
    for (uint i = 0; i < airtunes::channels; ++i) {
        availableWrite = std::min(jack_ringbuffer_write_space(m_buffers[i]), availableWrite);
    }

    // buffer is full. start consumer and wait until buffer has space again.
    if (availableWrite < bytesPerChannel) {
        doStart();
        m_mutex.lock();
        m_waitCondition.wait(&m_mutex);
        m_mutex.unlock();
    }

    // Deinterleave and convert to float32
    float outSamples[airtunes::channels][numSamples/airtunes::channels];
    for (uint i = 0; i < numSamples; i += airtunes::channels) {
        for (uint j = 0; j < airtunes::channels; ++j) {
            outSamples[j][i/airtunes::channels] = (float)inSamples[i+j]/32768.0f;
        }
    }

    // Write to jack ringbuffer
    for (uint i = 0; i < airtunes::channels; ++i) {
        size_t written = jack_ringbuffer_write(m_buffers[i], (char*)outSamples[i], bytesPerChannel);
        if (written != bytesPerChannel) {
            qWarning()<<Q_FUNC_INFO<<"error writing entire packet. written ="<<written;
        }
    }
}

void AudioOutJack::doStart()
{
    // Activate our client if jack_ringbuffer is ready
    if (jack_activate(m_client)) {
        qWarning()<<Q_FUNC_INFO<<"cannot activate client";
    }

    if (!m_portsConnected) {
        // if output port count does not match our channel count, do not connect.
        if (m_destinationPorts.size() != airtunes::channels) {
            qWarning()<<Q_FUNC_INFO<<"destination port count does not match:"<<m_destinationPorts.size();
            m_portsConnected = true; // set to true anyway, since we do not want to retry.
            return;
        }

        for (uint i = 0; i < airtunes::channels; ++i) {
            if (jack_connect(m_client, QString("omnifunken:output_%1").arg(i+1).toLatin1(), m_destinationPorts[i].toLatin1())) {
                qWarning()<<Q_FUNC_INFO<<"cannot connect to destination port:"<<m_destinationPorts[i];
            }
        }

        m_portsConnected = true;
    }
}

// The process callback called in a realtime thread.
int AudioOutJack::onProcess(jack_nframes_t nframes, void *arg)
{
    if (nframes <= 0) {
        return 0;
    }

    AudioOutJack *instance = static_cast<AudioOutJack*>(arg);

    for (uint i = 0; i < airtunes::channels; ++i) {
        sample_t *out = static_cast<sample_t*>(jack_port_get_buffer(instance->m_ports[i], nframes));
        // we request nframes*sizeof(sample_t) bytes
        size_t readBytes = jack_ringbuffer_read(instance->m_buffers[i], (char*)out, nframes*sizeof(sample_t));
        size_t readFrames = readBytes/sizeof(sample_t);

        // Fill remaining frames with silence
        if (readFrames < nframes) {
            qDebug()<<Q_FUNC_INFO<<"buffer ran empty, stuffing with silence";
            for (size_t j = readFrames; j < nframes; j++) {
                out[j] = 0.0f;
            }
        }
    }

    instance->m_waitCondition.wakeAll();

    return 0;
}

void AudioOutJack::onShutdown(void *arg)
{
    qWarning()<<Q_FUNC_INFO;

    AudioOutJack *instance = static_cast<AudioOutJack*>(arg);
    instance->deinit();
}

void AudioOutJack::onError(const char *message)
{
    qWarning()<<Q_FUNC_INFO<<message;
}

static AudioOutJack s_instance;
