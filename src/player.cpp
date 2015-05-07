#include "player.h"

#include "audioout/audioout_abstract.h"
#include "rtp/rtpbuffer.h"
#include "rtp/rtppacket.h"

#include <QDebug>

Player::Player(RtpBuffer *rtpBuffer, AudioOutAbstract *audioOut, QObject *parent) :
    QObject(parent),
    m_rtpBuffer(rtpBuffer),
    m_audioOut(audioOut),
    m_volume(0.0f)
{
    connect(m_rtpBuffer, SIGNAL(ready()), this, SLOT(play()));

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
    qDebug() << __PRETTY_FUNCTION__;

    if (m_playWorker->isRunning()) {
        //m_playWorker->quit();
        m_playWorker->wait();
    }
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

Player::PlayWorker::PlayWorker(Player *player)
    : QThread(player),
      m_player(player)
{
}

void Player::PlayWorker::run()
{
    qDebug() << __func__;

    float prevVolume = 0.0;

    while(true) {
        const RtpPacket *packet = m_player->m_rtpBuffer->takePacket();
        if (!packet) {
            qWarning() << __PRETTY_FUNCTION__ << ": no packet from buffer.";
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
        if (prevVolume != volume && m_player->m_audioOut->hasVolumeControl()) {
            m_player->m_audioOut->setVolume(volume);
            prevVolume = volume;
        }
        m_player->m_audioOut->play(packet->payload, packet->payloadSize);
    } // while

    char *silence = NULL;
    int size;
    m_player->m_rtpBuffer->silence(&silence, &size);
    m_player->m_audioOut->play(silence, size);
    m_player->m_audioOutTimer->start(15000);
}
