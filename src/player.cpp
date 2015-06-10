#include "player.h"

#include <audioout/audioout_abstract.h>
#include "core/core.h"
#include <rtp/rtpbuffer.h>
#include <rtp/rtppacket.h>

#include <QDebug>

Player::Player(RtpBuffer *rtpBuffer, QObject *parent) :
    QObject(parent),
    m_rtpBuffer(rtpBuffer),
    m_volume(0.0f)
{
    // Start playing when buffer is ready
    connect(m_rtpBuffer, SIGNAL(ready()), this, SLOT(play()));

    m_playWorker = new PlayWorker(this);
}

void Player::play()
{
    ofCore->audioOut()->start();
    m_playWorker->start();
}

void Player::teardown()
{
    qDebug()<<Q_FUNC_INFO;

    if (m_playWorker->isRunning()) {
        m_playWorker->wait();
    }
    ofCore->audioOut()->stop();
}

void Player::setVolume(float volume)
{
    if (ofCore->audioOut()->hasVolumeControl()) {
        ofCore->audioOut()->setVolume(volume);
    } else {
        m_mutex.lock();
        m_volume = volume;
        m_mutex.unlock();
    }
}

Player::PlayWorker::PlayWorker(Player *player)
    : QThread(player),
      m_player(player)
{
}

void Player::PlayWorker::run()
{
    qDebug()<<Q_FUNC_INFO<<"enter";

    float prevVolume = 0.0;

    while(true) {
        const RtpPacket *packet = m_player->m_rtpBuffer->takePacket();
        if (!packet) {
            qWarning()<<Q_FUNC_INFO<< "no packet from buffer. Stopping playback.";
            break;
        }
        m_player->m_mutex.lock();
        float volume = m_player->m_volume;
        m_player->m_mutex.unlock();
        int shift = abs(volume/5.625f);
        //float volume = qPow(10.0f, m_volume/20.0f);
        if (shift != 0) {
            for (int i = 0; i < packet->payloadSize/2; ++i) {
                *(qint16 *)(packet->payload+(i*2)) >>= shift;
                //*(qint16 *)(data+(i*2)) *= volume;
            }
        }
        if (prevVolume != volume && ofCore->audioOut()->hasVolumeControl()) {
            ofCore->audioOut()->setVolume(volume);
            prevVolume = volume;
        }
        ofCore->audioOut()->play(packet->payload, packet->payloadSize);
    } // while

    qDebug()<<Q_FUNC_INFO<< "exit";
}
