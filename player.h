#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QTimer>
#include <QThread>

#include <ao/ao.h>

#include "audio_out.h"
#include "rtpbuffer.h"

class Player : public QObject
{
    Q_OBJECT
    friend class PlayWorker;
public:
    explicit Player(RtpBuffer *rtpBuffer, QObject *parent = 0);

signals:
    void timeout();

public slots:
    void play();

private slots:
    void updateRateControl(quint16 size);

private:
    class PlayWorker : public QThread
    {
    public:
        explicit PlayWorker(Player *player);
    private:
        void run() Q_DECL_OVERRIDE;
        Player *m_player;
    };

    void init();
    void deinit();

private:
    QTimer      *m_timeoutTimer;
    QTimer      *m_pullTimer;
    QThread     m_thread;
    AudioOutAbstract *m_audioOut;
    RtpBuffer   *m_rtpBuffer;
    ao_device   *m_aoDevice;
    PlayWorker  *m_playWorker;
};

#endif // PLAYER_H
