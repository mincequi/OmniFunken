#include "player.h"

#include <QDebug>

Player::Player(RtpBuffer *rtpBuffer, AudioOutAbstract *audioOut, QObject *parent) :
    QObject(parent),
    m_rtpBuffer(rtpBuffer),
    m_audioOut(audioOut)
{
    connect(m_rtpBuffer, SIGNAL(ready()), this, SLOT(play()));
    connect(m_rtpBuffer, SIGNAL(notify(quint16)), this, SLOT(updateRateControl(quint16)));

    m_audioOutTimer = new QTimer(this);
    m_audioOutTimer->setSingleShot(true);
    connect(m_audioOutTimer, &QTimer::timeout, [this]() { teardown(); } );

    m_playWorker = new PlayWorker(this);
}

void Player::play()
{
    m_audioOutTimer->stop();
    m_audioOut->init();
    m_playWorker->start();
}

void Player::teardown()
{
    m_audioOutTimer->stop();
    m_audioOut->deinit();
}

void Player::updateRateControl(quint16 size)
{
    qDebug() << __func__ << ": buffer size: " << size;
}

Player::PlayWorker::PlayWorker(Player *player)
    : QThread(player),
      m_player(player)
{
}

void Player::PlayWorker::run()
{
    while(const RtpBuffer::RtpPacket *packet = m_player->m_rtpBuffer->takePacket()) {
        m_player->m_audioOut->play(packet->payload, packet->payloadSize);
    }

    char *silence = NULL;
    int size;
    m_player->m_rtpBuffer->silence(&silence, &size);
    m_player->m_audioOut->play(silence, size);
    m_player->m_audioOutTimer->start(15000);
}
