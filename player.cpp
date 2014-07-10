#include "player.h"

Player::Player(RtpBuffer* rtpBuffer, QObject *parent) :
    QObject(parent),
    m_rtpBuffer(rtpBuffer)
{
    //this->moveToThread(&m_thread);
    connect(m_rtpBuffer, SIGNAL(ready()), this, SLOT(play()));
    init();
}

void Player::play()
{
    while(const RtpBuffer::RtpPacket *packet = m_rtpBuffer->takePacket())
        ao_play(m_aoDevice, packet->payload, packet->payloadSize);
}

void Player::stop()
{
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
