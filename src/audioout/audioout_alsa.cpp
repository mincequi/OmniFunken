#include "audioout_alsa.h"
#include "audiooutfactory.h"
#include "airtunes/airtunesconstants.h"

#include <QDebug>


AudioOutAlsa::AudioOutAlsa() :
    m_deviceName("hw:0"),
    m_pcm(0),
    m_block(true),
    m_format(SND_PCM_FORMAT_UNKNOWN),
    m_bitAccurate(true),
    m_conversionBuffer(NULL),
    m_volume(0.0)
{
    AudioOutFactory::registerAudioOut(this);
}

const char *AudioOutAlsa::name() const
{
    return "alsa";
}

bool AudioOutAlsa::init(const QSettings::SettingsMap &settings)
{
    Q_UNUSED(settings)

    if (!probeNativeFormat()) {
        return false;
    }

    // sample size in bytes
    int sampleSize = snd_pcm_format_width(m_format);
    if ((sampleSize%8) != 0) {
        return false;
    }
    if (sampleSize > Airtunes::sampleSize) {
        int size = Airtunes::framesPerPacket*Airtunes::channels*sampleSize/8;
        m_conversionBuffer = new char[size]();
    }
    return true;
}

void AudioOutAlsa::deinit()
{
    stop();
    if (m_conversionBuffer) {
        delete[] m_conversionBuffer;
    }
}

void AudioOutAlsa::start()
{
    if (m_pcm) {
        return;
    }

    qDebug() << __func__;

    snd_pcm_hw_params_t *hw_params;

    int error = 0;
    if ((error = snd_pcm_open(&m_pcm, m_deviceName, SND_PCM_STREAM_PLAYBACK, m_block ? 0 : SND_PCM_NONBLOCK) < 0)) {
        qCritical("cannot open audio device %s (%s)\n", m_deviceName, snd_strerror(error));
        return;
    }
    if ((error = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
        qCritical("cannot allocate hardware parameter structure (%s)\n", snd_strerror(error));
        return;
    }
    if ((error = snd_pcm_hw_params_any(m_pcm, hw_params)) < 0) {
        qCritical("cannot initialize hardware parameter structure (%s)\n", snd_strerror(error));
        return;
    }
    if ((error = snd_pcm_hw_params_set_access(m_pcm, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        qCritical("cannot set access type (%s)\n", snd_strerror(error));
        return;
    }
    if ((error = snd_pcm_hw_params_set_format(m_pcm, hw_params, m_format)) < 0) {
        qCritical("cannot set sample format (%s)\n", snd_strerror(error));
        return;
    }
    if ((error = snd_pcm_hw_params_set_rate(m_pcm, hw_params, Airtunes::sampleRate, 0)) < 0) {
        qCritical("cannot set sample rate (%s)\n", snd_strerror(error));
        return;
    }
    if ((error = snd_pcm_hw_params_set_channels(m_pcm, hw_params, 2)) < 0) {
        qCritical("cannot set channel count (%s)\n", snd_strerror(error));
        return;
    }
    if ((error = snd_pcm_hw_params_set_period_size(m_pcm, hw_params, Airtunes::framesPerPacket, 0)) < 0) {
        qCritical("cannot set period size (%s)\n", snd_strerror(error));
        return;
    }
    if ((error = snd_pcm_hw_params_set_buffer_size(m_pcm, hw_params, 16*Airtunes::framesPerPacket)) < 0) {
        qCritical("cannot set buffer size (%s)\n", snd_strerror(error));
        return;
    }
    if ((error = snd_pcm_hw_params(m_pcm, hw_params)) < 0) {
        qCritical("cannot set parameters (%s)\n", snd_strerror(error));
        return;
    }

    snd_pcm_hw_params_free(hw_params);

    if ((error = snd_pcm_prepare(m_pcm)) < 0) {
        qCritical("cannot prepare audio interface for use (%s)\n", snd_strerror(error));
        return;
    }
}

void AudioOutAlsa::stop()
{
    if (m_pcm) {
        qDebug() << __func__;
        snd_pcm_drain(m_pcm);
        snd_pcm_close(m_pcm);
        m_pcm = 0;
    }
}

void AudioOutAlsa::play(char *data, int bytes)
{
    int error = snd_pcm_writei(m_pcm, convertSamplesToNativeFormat(data, bytes/4), bytes/4);
    if (error < 0) {
        error = snd_pcm_recover(m_pcm, error, 1);
    }
    if (error < 0) {
        qFatal("write to audio interface failed (%s)\n", snd_strerror(error));
    } else {
        //error = snd_pcm_writei(m_pcm, samples, bytes/4);
    }
}

bool AudioOutAlsa::hasVolumeControl()
{
    return true;
}

void AudioOutAlsa::setVolume(float volume)
{
    m_volume = volume;
}

bool AudioOutAlsa::probeNativeFormat()
{
#define ARRAY_SIZE(a) (sizeof(a) / sizeof *(a))

    static const snd_pcm_format_t formats[] = {
        SND_PCM_FORMAT_S16_LE, /** Signed 16 bit Little Endian */
        //SND_PCM_FORMAT_S16_BE, /** Signed 16 bit Big Endian */
        //SND_PCM_FORMAT_U16_LE, /** Unsigned 16 bit Little Endian */
        //SND_PCM_FORMAT_U16_BE, /** Unsigned 16 bit Big Endian */
        SND_PCM_FORMAT_S24_LE, /** Signed 24 bit Little Endian using low three bytes in 32-bit word */
        //SND_PCM_FORMAT_S24_BE, /** Signed 24 bit Big Endian */
        //SND_PCM_FORMAT_U24_LE, /** Unsigned 24 bit Little Endian */
        //SND_PCM_FORMAT_U24_BE, /** Unsigned 24 bit Big Endian */
        SND_PCM_FORMAT_S24_3LE, /** Signed 24bit Little Endian in 3bytes format */
        //SND_PCM_FORMAT_S24_3BE, /** Signed 24bit Big Endian in 3bytes format */
        //SND_PCM_FORMAT_U24_3LE, /** Unsigned 24bit Little Endian in 3bytes format */
        //SND_PCM_FORMAT_U24_3BE, /** Unsigned 24bit Big Endian in 3bytes format */
        SND_PCM_FORMAT_S32_LE, /** Signed 32 bit Little Endian */
        //SND_PCM_FORMAT_S32_BE, /** Signed 32 bit Big Endian */
        //SND_PCM_FORMAT_U32_LE, /** Unsigned 32 bit Little Endian */
        //SND_PCM_FORMAT_U32_BE, /** Unsigned 32 bit Big Endian */
    };

    snd_pcm_t *pcm;
    snd_pcm_hw_params_t *hw_params;
    int error;

    error = snd_pcm_open(&pcm, m_deviceName, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
    if (error < 0) {
        qWarning("cannot open device '%s': %s\n", m_deviceName, snd_strerror(error));
        return false;
    }

    snd_pcm_hw_params_alloca(&hw_params);
    error = snd_pcm_hw_params_any(pcm, hw_params);
    if (error < 0) {
        qWarning("cannot get hardware parameters: %s\n", snd_strerror(error));
        snd_pcm_close(pcm);
        return false;
    }

    qDebug("Device: %s (type: %s)\n", m_deviceName, snd_pcm_type_name(snd_pcm_type(pcm)));

    qDebug("Formats:");
    for (int i = ARRAY_SIZE(formats)-1; i >= 0; --i) {
        if (!snd_pcm_hw_params_test_format(pcm, hw_params, formats[i])) {
            qDebug(" %s", snd_pcm_format_name(formats[i]));
            m_format = formats[i];
            break;
        }
    }

    if ((error = snd_pcm_hw_params_test_rate(pcm, hw_params, Airtunes::sampleRate, 0)) < 0) {
        qWarning("cannot set sample rate (%s)\n", snd_strerror(error));
        return false;
    }
    if ((error = snd_pcm_hw_params_test_channels(pcm, hw_params, Airtunes::channels)) < 0) {
        qWarning("cannot set channel count (%s)\n", snd_strerror(error));
        return false;
    }
    if ((error = snd_pcm_hw_params_test_period_size(pcm, hw_params, Airtunes::framesPerPacket, 0)) < 0) {
        qWarning("cannot set period size (%s)\n", snd_strerror(error));
        return false;
    }
    if ((error = snd_pcm_hw_params_test_buffer_size(pcm, hw_params, 16*Airtunes::framesPerPacket)) < 0) {
        qWarning("cannot set buffer size (%s)\n", snd_strerror(error));
        return false;
    }

    snd_pcm_close(pcm);
    return true;
}

const char* AudioOutAlsa::convertSamplesToNativeFormat(char *frames, snd_pcm_uframes_t size)
{
    if (m_conversionBuffer) {
        for(snd_pcm_uframes_t i = 0; i < size; ++i) {
            qint32 left = 0;
            qint32 right = 0;

            ((qint16*)&left)[1] = ((qint16*)frames)[i*2];
            ((qint16*)&right)[1] = ((qint16*)frames)[i*2+1];

            // apply volume
            int shift = abs(m_volume/3.75f);
            if (shift >= 16) {
                shift = 16;
            }
            if (shift) {
                left >>= shift;
                right >>= shift;
            }

            // Always the higher 3 bytes/24 bits are relevant
            switch (m_format) {
            case SND_PCM_FORMAT_S24_3LE:
                m_conversionBuffer[i*6] = ((qint8*)&left)[1];
                m_conversionBuffer[i*6+1] = ((qint8*)&left)[2];
                m_conversionBuffer[i*6+2] = ((qint8*)&left)[3];
                m_conversionBuffer[i*6+3] = ((qint8*)&right)[1];
                m_conversionBuffer[i*6+4] = ((qint8*)&right)[2];
                m_conversionBuffer[i*6+5] = ((qint8*)&right)[3];
                break;
            case SND_PCM_FORMAT_S24_LE:
                m_conversionBuffer[i*8] = ((qint8*)&left)[1];
                m_conversionBuffer[i*8+1] = ((qint8*)&left)[2];
                m_conversionBuffer[i*8+2] = ((qint8*)&left)[3];
                m_conversionBuffer[i*8+4] = ((qint8*)&right)[1];
                m_conversionBuffer[i*8+5] = ((qint8*)&right)[2];
                m_conversionBuffer[i*8+6] = ((qint8*)&right)[3];
                break;
            case SND_PCM_FORMAT_S32_LE:
                m_conversionBuffer[i*8] = ((qint8*)&left)[0];
                m_conversionBuffer[i*8+1] = ((qint8*)&left)[1];
                m_conversionBuffer[i*8+2] = ((qint8*)&left)[2];
                m_conversionBuffer[i*8+3] = ((qint8*)&left)[3];
                m_conversionBuffer[i*8+4] = ((qint8*)&right)[0];
                m_conversionBuffer[i*8+5] = ((qint8*)&right)[1];
                m_conversionBuffer[i*8+6] = ((qint8*)&right)[2];
                m_conversionBuffer[i*8+7] = ((qint8*)&right)[3];
                break;
            default:
                return frames;
                break;
            }

            /*
            *(m_conversionBuffer+(i*6)) = *(((qint8*)&left)+1);
            *(m_conversionBuffer+(i*6+3)) = *(((qint8*)&right)+1);

            *((qint16*)(m_conversionBuffer+(i*6+1))) = *(((qint16*)&left)+1);
            *((qint16*)(m_conversionBuffer+(i*6+4))) = *(((qint16*)&right)+1);
            */
        }
        return m_conversionBuffer;
    }
    return frames;
}

static AudioOutAlsa s_instance;

/*
#include <QtCore/qcoreapplication.h>
#include <QtMultimedia/private/qaudiohelpers_p.h>
#include "qalsaaudiooutput.h"
#include "qalsaaudiodeviceinfo.h"


QAlsaAudioOutput::QAlsaAudioOutput(const QByteArray &device)
{
    bytesAvailable = 0;
    handle = 0;
    ahandler = 0;
    access = SND_PCM_ACCESS_RW_INTERLEAVED;
    pcmformat = SND_PCM_FORMAT_S16;
    buffer_frames = 0;
    period_frames = 0;
    buffer_size = 0;
    period_size = 0;
    buffer_time = 100000;
    period_time = 20000;
    totalTimeValue = 0;
    intervalTime = 1000;
    audioBuffer = 0;
    errorState = QAudio::NoError;
    deviceState = QAudio::StoppedState;
    audioSource = 0;
    pullMode = true;
    resuming = false;
    opened = false;

    m_volume = 1.0f;

    m_device = device;

    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),SLOT(userFeed()));
}

QAlsaAudioOutput::~QAlsaAudioOutput()
{
    close();
    disconnect(timer, SIGNAL(timeout()));
    QCoreApplication::processEvents();
    delete timer;
}

void QAlsaAudioOutput::async_callback(snd_async_handler_t *ahandler)
{
    QAlsaAudioOutput* audioOut;

    audioOut = static_cast<QAlsaAudioOutput*>
        (snd_async_handler_get_callback_private(ahandler));

    if (audioOut && (audioOut->deviceState == QAudio::ActiveState || audioOut->resuming))
        audioOut->feedback();
}

int QAlsaAudioOutput::xrun_recovery(int err)
{
    int  count = 0;
    bool reset = false;

    if(error == -EPIPE) {
        errorState = QAudio::UnderrunError;
        emit errorChanged(errorState);
        error = snd_pcm_prepare(handle);
        if(error < 0)
            reset = true;

    } else if((error == -ESTRPIPE)||(error == -EIO)) {
        errorState = QAudio::IOError;
        emit errorChanged(errorState);
        while((error = snd_pcm_resume(handle)) == -EAGAIN){
            usleep(100);
            count++;
            if(count > 5) {
                reset = true;
                break;
            }
        }
        if(error < 0) {
            error = snd_pcm_prepare(handle);
            if(error < 0)
                reset = true;
        }
    }
    if(reset) {
        close();
        open();
        snd_pcm_prepare(handle);
        return 0;
    }
    return err;
}

int QAlsaAudioOutput::setFormat()
{
    snd_pcm_format_t pcmformat = SND_PCM_FORMAT_UNKNOWN;

    if(settings.sampleSize() == 8) {
        pcmformat = SND_PCM_FORMAT_U8;

    } else if(settings.sampleSize() == 16) {
        if(settings.sampleType() == QAudioFormat::SignedInt) {
            if(settings.byteOrder() == QAudioFormat::LittleEndian)
                pcmformat = SND_PCM_FORMAT_S16_LE;
            else
                pcmformat = SND_PCM_FORMAT_S16_BE;
        } else if(settings.sampleType() == QAudioFormat::UnSignedInt) {
            if(settings.byteOrder() == QAudioFormat::LittleEndian)
                pcmformat = SND_PCM_FORMAT_U16_LE;
            else
                pcmformat = SND_PCM_FORMAT_U16_BE;
        }
    } else if(settings.sampleSize() == 24) {
        if(settings.sampleType() == QAudioFormat::SignedInt) {
            if(settings.byteOrder() == QAudioFormat::LittleEndian)
                pcmformat = SND_PCM_FORMAT_S24_LE;
            else
                pcmformat = SND_PCM_FORMAT_S24_BE;
        } else if(settings.sampleType() == QAudioFormat::UnSignedInt) {
            if(settings.byteOrder() == QAudioFormat::LittleEndian)
                pcmformat = SND_PCM_FORMAT_U24_LE;
            else
                pcmformat = SND_PCM_FORMAT_U24_BE;
        }
    } else if(settings.sampleSize() == 32) {
        if(settings.sampleType() == QAudioFormat::SignedInt) {
            if(settings.byteOrder() == QAudioFormat::LittleEndian)
                pcmformat = SND_PCM_FORMAT_S32_LE;
            else
                pcmformat = SND_PCM_FORMAT_S32_BE;
        } else if(settings.sampleType() == QAudioFormat::UnSignedInt) {
            if(settings.byteOrder() == QAudioFormat::LittleEndian)
                pcmformat = SND_PCM_FORMAT_U32_LE;
            else
                pcmformat = SND_PCM_FORMAT_U32_BE;
        } else if(settings.sampleType() == QAudioFormat::Float) {
            if(settings.byteOrder() == QAudioFormat::LittleEndian)
                pcmformat = SND_PCM_FORMAT_FLOAT_LE;
            else
                pcmformat = SND_PCM_FORMAT_FLOAT_BE;
        }
    } else if(settings.sampleSize() == 64) {
        if(settings.byteOrder() == QAudioFormat::LittleEndian)
            pcmformat = SND_PCM_FORMAT_FLOAT64_LE;
        else
            pcmformat = SND_PCM_FORMAT_FLOAT64_BE;
    }

    return pcmformat != SND_PCM_FORMAT_UNKNOWN
            ? snd_pcm_hw_params_set_format( handle, hwparams, pcmformat)
            : -1;
}



QIODevice* QAlsaAudioOutput::start()
{
    if(deviceState != QAudio::StoppedState)
        deviceState = QAudio::StoppedState;

    errorState = QAudio::NoError;

    // Handle change of mode
    if(audioSource && !pullMode) {
        delete audioSource;
        audioSource = 0;
    }

    close();

    audioSource = new OutputPrivate(this);
    audioSource->open(QIODevice::WriteOnly|QIODevice::Unbuffered);
    pullMode = false;

    deviceState = QAudio::IdleState;

    open();

    emit stateChanged(deviceState);

    return audioSource;
}

void QAlsaAudioOutput::stop()
{
    if(deviceState == QAudio::StoppedState)
        return;
    errorState = QAudio::NoError;
    deviceState = QAudio::StoppedState;
    close();
    emit stateChanged(deviceState);
}

bool QAlsaAudioOutput::open()
{
    timeStamp.restart();
    elapsedTimeOffset = 0;

    int dir;
    int error = 0;
    int count=0;
    unsigned int sampleRate=settings.sampleRate();

    if (error == 0) {
        errorState = QAudio::OpenError;
        deviceState = QAudio::StoppedState;
        emit errorChanged(errorState);
        return false;
    }

    QString dev = QString(QLatin1String(m_device.constData()));
    QList<QByteArray> devices = QAlsaAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
    if(dev.compare(QLatin1String("default")) == 0) {
        if (devices.size() > 0)
            dev = QLatin1String(devices.first());
        else
            return false;
    } else {
        dev = QLatin1String(m_device);
    }

    // Step 1: try and open the device
    while((count < 5) && (error < 0)) {
        err=snd_pcm_open(&handle,dev.toLocal8Bit().constData(),SND_PCM_STREAM_PLAYBACK,0);
        if(error < 0)
            count++;
    }
    if (( error < 0)||(handle == 0)) {
        errorState = QAudio::OpenError;
        emit errorChanged(errorState);
        deviceState = QAudio::StoppedState;
        return false;
    }
    snd_pcm_nonblock( handle, 0 );

    // Step 2: Set the desired HW parameters.
    snd_pcm_hw_params_alloca( &hwparams );

    bool fatal = false;
    QString errMessage;
    unsigned int chunks = 8;

    error = snd_pcm_hw_params_any( handle, hwparams );
    if ( error < 0 ) {
        fatal = true;
        errMessage = QString::fromLatin1("QAudioOutput: snd_pcm_hw_params_any: error = %1").arg(error);
    }
    if ( !fatal ) {
        error = snd_pcm_hw_params_set_rate_resample( handle, hwparams, 1 );
        if ( error < 0 ) {
            fatal = true;
            errMessage = QString::fromLatin1("QAudioOutput: snd_pcm_hw_params_set_rate_resample: error = %1").arg(error);
        }
    }
    if ( !fatal ) {
        error = snd_pcm_hw_params_set_access( handle, hwparams, access );
        if ( error < 0 ) {
            fatal = true;
            errMessage = QString::fromLatin1("QAudioOutput: snd_pcm_hw_params_set_access: error = %1").arg(error);
        }
    }
    if ( !fatal ) {
        error = setFormat();
        if ( error < 0 ) {
            fatal = true;
            errMessage = QString::fromLatin1("QAudioOutput: snd_pcm_hw_params_set_format: error = %1").arg(error);
        }
    }
    if ( !fatal ) {
        error = snd_pcm_hw_params_set_channels( handle, hwparams, (unsigned int)settings.channelCount() );
        if ( error < 0 ) {
            fatal = true;
            errMessage = QString::fromLatin1("QAudioOutput: snd_pcm_hw_params_set_channels: error = %1").arg(error);
        }
    }
    if ( !fatal ) {
        error = snd_pcm_hw_params_set_rate_near( handle, hwparams, &sampleRate, 0 );
        if ( error < 0 ) {
            fatal = true;
            errMessage = QString::fromLatin1("QAudioOutput: snd_pcm_hw_params_set_rate_near: error = %1").arg(error);
        }
    }
    if ( !fatal ) {
        unsigned int maxBufferTime = 0;
        unsigned int minBufferTime = 0;
        unsigned int maxPeriodTime = 0;
        unsigned int minPeriodTime = 0;

        error = snd_pcm_hw_params_get_buffer_time_max(hwparams, &maxBufferTime, &dir);
        if ( error >= 0)
            error = snd_pcm_hw_params_get_buffer_time_min(hwparams, &minBufferTime, &dir);
        if ( error >= 0)
            error = snd_pcm_hw_params_get_period_time_max(hwparams, &maxPeriodTime, &dir);
        if ( error >= 0)
            error = snd_pcm_hw_params_get_period_time_min(hwparams, &minPeriodTime, &dir);

        if ( error < 0 ) {
            fatal = true;
            errMessage = QString::fromLatin1("QAudioOutput: buffer/period min and max: error = %1").arg(error);
        } else {
            if (maxBufferTime < buffer_time || buffer_time < minBufferTime || maxPeriodTime < period_time || minPeriodTime > period_time) {
                period_time = minPeriodTime;
                if (period_time*4 <= maxBufferTime) {
                    // Use 4 periods if possible
                    buffer_time = period_time*4;
                    chunks = 4;
                } else if (period_time*2 <= maxBufferTime) {
                    // Use 2 periods if possible
                    buffer_time = period_time*2;
                    chunks = 2;
                } else {
                    qWarning()<<"QAudioOutput: alsa only supports single period!";
                    fatal = true;
                }
            }
        }
    }
    if ( !fatal ) {
        error = snd_pcm_hw_params_set_buffer_time_near(handle, hwparams, &buffer_time, &dir);
        if ( error < 0 ) {
            fatal = true;
            errMessage = QString::fromLatin1("QAudioOutput: snd_pcm_hw_params_set_buffer_time_near: error = %1").arg(error);
        }
    }
    if ( !fatal ) {
        error = snd_pcm_hw_params_set_period_time_near(handle, hwparams, &period_time, &dir);
        if ( error < 0 ) {
            fatal = true;
            errMessage = QString::fromLatin1("QAudioOutput: snd_pcm_hw_params_set_period_time_near: error = %1").arg(error);
        }
    }
    if ( !fatal ) {
        error = snd_pcm_hw_params_set_periods_near(handle, hwparams, &chunks, &dir);
        if ( error < 0 ) {
            fatal = true;
            errMessage = QString::fromLatin1("QAudioOutput: snd_pcm_hw_params_set_periods_near: error = %1").arg(error);
        }
    }
    if ( !fatal ) {
        error = snd_pcm_hw_params(handle, hwparams);
        if ( error < 0 ) {
            fatal = true;
            errMessage = QString::fromLatin1("QAudioOutput: snd_pcm_hw_params: error = %1").arg(error);
        }
    }
    if( error < 0) {
        qWarning()<<errMessage;
        errorState = QAudio::OpenError;
        emit errorChanged(errorState);
        deviceState = QAudio::StoppedState;
        return false;
    }
    snd_pcm_hw_params_get_buffer_size(hwparams,&buffer_frames);
    buffer_size = snd_pcm_frames_to_bytes(handle,buffer_frames);
    snd_pcm_hw_params_get_period_size(hwparams,&period_frames, &dir);
    period_size = snd_pcm_frames_to_bytes(handle,period_frames);
    snd_pcm_hw_params_get_buffer_time(hwparams,&buffer_time, &dir);
    snd_pcm_hw_params_get_period_time(hwparams,&period_time, &dir);

    // Step 3: Set the desired SW parameters.
    snd_pcm_sw_params_t *swparams;
    snd_pcm_sw_params_alloca(&swparams);
    snd_pcm_sw_params_current(handle, swparams);
    snd_pcm_sw_params_set_start_threshold(handle,swparams,period_frames);
    snd_pcm_sw_params_set_stop_threshold(handle,swparams,buffer_frames);
    snd_pcm_sw_params_set_avail_min(handle, swparams,period_frames);
    snd_pcm_sw_params(handle, swparams);

    // Step 4: Prepare audio
    if(audioBuffer == 0)
        audioBuffer = new char[snd_pcm_frames_to_bytes(handle,buffer_frames)];
    snd_pcm_prepare( handle );
    snd_pcm_start(handle);

    // Step 5: Setup callback and timer fallback
    snd_async_add_pcm_handler(&ahandler, handle, async_callback, this);
    bytesAvailable = bytesFree();

    // Step 6: Start audio processing
    timer->start(period_time/1000);

    clockStamp.restart();
    timeStamp.restart();
    elapsedTimeOffset = 0;
    errorState  = QAudio::NoError;
    totalTimeValue = 0;
    opened = true;

    return true;
}

void QAlsaAudioOutput::close()
{
    timer->stop();

    if ( handle ) {
        snd_pcm_drain( handle );
        snd_pcm_close( handle );
        handle = 0;
        delete [] audioBuffer;
        audioBuffer=0;
    }
    if(!pullMode && audioSource) {
        delete audioSource;
        audioSource = 0;
    }
}

int QAlsaAudioOutput::bytesFree() const
{
    if(resuming)
        return period_size;

    if(deviceState != QAudio::ActiveState && deviceState != QAudio::IdleState)
        return 0;

    int frames = snd_pcm_avail_update(handle);
    if (frames == -EPIPE) {
        // Try and handle buffer underrun
        int error = snd_pcm_recover(handle, frames, 0);
        if (error < 0)
            return 0;
        else
            frames = snd_pcm_avail_update(handle);
    } else if (frames < 0) {
        return 0;
    }

    if ((int)frames > (int)buffer_frames)
        frames = buffer_frames;

    return snd_pcm_frames_to_bytes(handle, frames);
}

qint64 QAlsaAudioOutput::write( const char *data, qint64 len )
{
    int frames, err;
    int space = bytesFree();

    if (!space)
        return 0;

    if (len < space)
        space = len;

    frames = snd_pcm_bytes_to_frames(handle, space);

    if (m_volume < 1.0f) {
        char out[space];
        QAudioHelperInternal::qMultiplySamples(m_volume, settings, data, out, space);
        error = snd_pcm_writei(handle, out, frames);
    } else {
        error = snd_pcm_writei(handle, data, frames);
    }

    if(error > 0) {
        totalTimeValue += err;
        resuming = false;
        errorState = QAudio::NoError;
        if (deviceState != QAudio::ActiveState) {
            deviceState = QAudio::ActiveState;
            emit stateChanged(deviceState);
        }
        return snd_pcm_frames_to_bytes( handle, error );
    } else
        error = xrun_recovery(error);

    if(error < 0) {
        close();
        errorState = QAudio::FatalError;
        emit errorChanged(errorState);
        deviceState = QAudio::StoppedState;
        emit stateChanged(deviceState);
    }
    return 0;
}

int QAlsaAudioOutput::periodSize() const
{
    return period_size;
}

void QAlsaAudioOutput::setBufferSize(int value)
{
    if(deviceState == QAudio::StoppedState)
        buffer_size = value;
}

int QAlsaAudioOutput::bufferSize() const
{
    return buffer_size;
}

void QAlsaAudioOutput::setNotifyInterval(int ms)
{
    intervalTime = qMax(0, ms);
}

int QAlsaAudioOutput::notifyInterval() const
{
    return intervalTime;
}

qint64 QAlsaAudioOutput::processedUSecs() const
{
    return qint64(1000000) * totalTimeValue / settings.sampleRate();
}

void QAlsaAudioOutput::resume()
{
    if(deviceState == QAudio::SuspendedState) {
        int error = 0;

        if(handle) {
            error = snd_pcm_prepare( handle );
            if(error < 0)
                xrun_recovery(error);

            error = snd_pcm_start(handle);
            if(error < 0)
                xrun_recovery(error);

            bytesAvailable = (int)snd_pcm_frames_to_bytes(handle, buffer_frames);
        }
        resuming = true;

        deviceState = QAudio::ActiveState;

        errorState = QAudio::NoError;
        timer->start(period_time/1000);
        emit stateChanged(deviceState);
    }
}

void QAlsaAudioOutput::setFormat(const QAudioFormat& fmt)
{
    if (deviceState == QAudio::StoppedState)
        settings = fmt;
}

QAudioFormat QAlsaAudioOutput::format() const
{
    return settings;
}

void QAlsaAudioOutput::suspend()
{
    if(deviceState == QAudio::ActiveState || deviceState == QAudio::IdleState || resuming) {
        timer->stop();
        deviceState = QAudio::SuspendedState;
        errorState = QAudio::NoError;
        emit stateChanged(deviceState);
    }
}

void QAlsaAudioOutput::userFeed()
{
    if(deviceState == QAudio::StoppedState || deviceState == QAudio::SuspendedState)
        return;

    if(deviceState ==  QAudio::IdleState)
        bytesAvailable = bytesFree();

    deviceReady();
}

void QAlsaAudioOutput::feedback()
{
    updateAvailable();
}


void QAlsaAudioOutput::updateAvailable()
{
    bytesAvailable = bytesFree();
}

bool QAlsaAudioOutput::deviceReady()
{
    bytesAvailable = bytesFree();
    if(bytesAvailable > snd_pcm_frames_to_bytes(handle, buffer_frames-period_frames)) {
        // Underrun
        if (deviceState != QAudio::IdleState) {
            errorState = QAudio::UnderrunError;
            emit errorChanged(errorState);
            deviceState = QAudio::IdleState;
            emit stateChanged(deviceState);
        }
    }

    if(deviceState != QAudio::ActiveState)
        return true;

    if(intervalTime && (timeStamp.elapsed() + elapsedTimeOffset) > intervalTime) {
        emit notify();
        elapsedTimeOffset = timeStamp.elapsed() + elapsedTimeOffset - intervalTime;
        timeStamp.restart();
    }
    return true;
}

qint64 QAlsaAudioOutput::elapsedUSecs() const
{
    if (deviceState == QAudio::StoppedState)
        return 0;

    return clockStamp.elapsed()*1000;
}

void QAlsaAudioOutput::reset()
{
    if(handle)
        snd_pcm_reset(handle);

    stop();
}

OutputPrivate::OutputPrivate(QAlsaAudioOutput* audio)
{
    audioDevice = qobject_cast<QAlsaAudioOutput*>(audio);
}

OutputPrivate::~OutputPrivate() {}

qint64 OutputPrivate::readData( char* data, qint64 len)
{
    Q_UNUSED(data)
    Q_UNUSED(len)

    return 0;
}

qint64 OutputPrivate::writeData(const char* data, qint64 len)
{
    int retry = 0;
    qint64 written = 0;
    if((audioDevice->deviceState == QAudio::ActiveState)
            ||(audioDevice->deviceState == QAudio::IdleState)) {
        while(written < len) {
            int chunk = audioDevice->write(data+written,(len-written));
            if(chunk <= 0)
                retry++;
            written+=chunk;
            if(retry > 10)
                return written;
        }
    }
    return written;
}
*/
