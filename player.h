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
        //Q_OBJECT
    public:
        explicit PlayWorker(RtpBuffer *rtpBuffer, ao_device *dev, QObject *parent = 0);

    private:
        void run() Q_DECL_OVERRIDE;
        RtpBuffer *m_rtpBuffer;
        ao_device *m_aoDevice;
    };

    int init();
    void deinit();

private:
    QTimer      *m_timeoutTimer;
    QTimer      *m_pullTimer;
    QThread     m_thread;
    AudioOutAbstract *m_audioOut;
    PlayWorker  *m_playWorker;
    ao_device   *m_aoDevice;
    RtpBuffer   *m_rtpBuffer;
};

#endif // PLAYER_H
