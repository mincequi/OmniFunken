#include "player.h"

#include <QDebug>

Player::Player(RtpBuffer *rtpBuffer, AudioOutAbstract *audioOut, QObject *parent) :
    QObject(parent),
    m_rtpBuffer(rtpBuffer),
    m_audioOut(audioOut),
    m_volume(0.0f)
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
    m_audioOut->start();
    m_playWorker->start();
}

void Player::teardown()
{
    m_audioOutTimer->stop();
    m_audioOut->stop();
}

void Player::setVolume(float volume)
{
    if (m_audioOut->hasVolumeControl()) {
        m_audioOut->setVolume(volume);
    } else {
        m_mutex.lock();
        m_volume = volume;
        m_mutex.unlock();
    }
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
    while(const RtpPacket *packet = m_player->m_rtpBuffer->takePacket()) {
        m_player->m_mutex.lock();
        int shift = abs(m_player->m_volume/5.625f);
        //float volume = qPow(10.0f, m_volume/20.0f);
        m_player->m_mutex.unlock();
        if (shift != 0) {
            for (int i = 0; i < packet->payloadSize/2; ++i) {
                *(qint16 *)(packet->payload+(i*2)) >>= shift;
                //*(qint16 *)(data+(i*2)) *= volume;
            }
        }

        m_player->m_audioOut->play(packet->payload, packet->payloadSize);
    } // while

//    char *silence = NULL;
//    int size;
//    m_player->m_rtpBuffer->silence(&silence, &size);
//    m_player->m_audioOut->play(silence, size);
//    m_player->m_audioOut->stop();
    m_player->m_audioOutTimer->start(15000);
}
