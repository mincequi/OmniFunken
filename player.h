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

public slots:
    void play();
    void stop();

private:
    int init();
    void deinit();

private:
    QTimer           *m_pullTimer;
    QThread m_thread;
    AudioOutAbstract *m_audioOut;
    RtpBuffer *m_rtpBuffer;
    ao_device *m_aoDevice;
};

#endif // PLAYER_H
