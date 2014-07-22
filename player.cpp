#include "player.h"

#include <QDebug>

Player::Player(RtpBuffer* rtpBuffer, QObject *parent) :
    QObject(parent),
    m_rtpBuffer(rtpBuffer)
{
    connect(m_rtpBuffer, SIGNAL(ready()), this, SLOT(play()));
    connect(m_rtpBuffer, SIGNAL(notify(quint16)), this, SLOT(updateRateControl(quint16)));
    init();
}

void Player::play()
{
    while(const RtpBuffer::RtpPacket *packet = m_rtpBuffer->takePacket()) {
        ao_play(m_aoDevice, packet->payload, packet->payloadSize);
    }

    char *silence = NULL;
    int size;
    m_rtpBuffer->silence(&silence, &size);
    ao_play(m_aoDevice, silence, size);
}

void Player::stop()
{
}

void Player::updateRateControl(quint16 size)
{
    qDebug() << __func__ << ": buffer size: " << size;
}

int Player::init()
{
    ao_initialize();
    int driver = ao_default_driver_id();
    ao_option *ao_opts = NULL;

    ao_sample_format fmt;
    memset(&fmt, 0, sizeof(fmt));

    fmt.bits = 16;
    fmt.rate = 44100;
    fmt.channels = 2;
    fmt.byte_format = AO_FMT_NATIVE;

    m_aoDevice = ao_open_live(driver, &fmt, ao_opts);

    return m_aoDevice ? 0 : 1;
}

void Player::deinit()
{
    if (m_aoDevice)
        ao_close(m_aoDevice);
    m_aoDevice = NULL;
    ao_shutdown();
}
