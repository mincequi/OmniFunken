#ifndef PLAYER_H
#define PLAYER_H

#include <QMutex>
#include <QObject>
#include <QTimer>
#include <QThread>

class RtpBuffer;

class Player : public QObject
{
    Q_OBJECT
    friend class PlayWorker;
public:
    explicit Player(RtpBuffer *rtpBuffer, QObject *parent = 0);

public slots:
    void play();
    void teardown();
    void setVolume(float volume);

private:
    class PlayWorker : public QThread
    {
    public:
        explicit PlayWorker(Player *player);
    private:
        void run() Q_DECL_OVERRIDE;
        Player *m_player;
    };

private:
    RtpBuffer   *m_rtpBuffer;
    PlayWorker  *m_playWorker;
    float       m_volume;
    QMutex      m_mutex;
};

#endif // PLAYER_H
