#include "player.h"

#include <QDebug>

Player::Player(RtpBuffer* rtpBuffer, QObject *parent) :
    QObject(parent),
    m_rtpBuffer(rtpBuffer),
    m_aoDevice(NULL)
{
    connect(m_rtpBuffer, SIGNAL(ready()), this, SLOT(play()));
    connect(m_rtpBuffer, SIGNAL(notify(quint16)), this, SLOT(updateRateControl(quint16)));

    m_timeoutTimer = new QTimer(this);
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout, [this]() { deinit(); emit timeout(); } );

    m_playWorker = new PlayWorker(this);
}

void Player::play()
{
    m_timeoutTimer->stop();
    init();
    m_playWorker->start();
}

void Player::updateRateControl(quint16 size)
{
    qDebug() << __func__ << ": buffer size: " << size;
}

void Player::init()
{
    if (!m_aoDevice) {
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
    }
}

void Player::deinit()
{
    if (m_aoDevice) {
        ao_close(m_aoDevice);
        m_aoDevice = NULL;
        ao_shutdown();
    }
}

Player::PlayWorker::PlayWorker(Player *player)
    : QThread(player),
      m_player(player)
{
}

void Player::PlayWorker::run()
{
    while(const RtpBuffer::RtpPacket *packet = m_player->m_rtpBuffer->takePacket()) {
        ao_play(m_player->m_aoDevice, packet->payload, packet->payloadSize);
    }

    char *silence = NULL;
    int size;
    m_player->m_rtpBuffer->silence(&silence, &size);
    ao_play(m_player->m_aoDevice, silence, size);

    m_player->m_timeoutTimer->start(5000);
}
