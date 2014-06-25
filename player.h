#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QTimer>

#include "audio_out.h"

class Player : public QObject
{
    Q_OBJECT
public:
    explicit Player(QObject *parent = 0);

signals:

public slots:
    void play();
    void stop();

    void setVolume(qint8 volume);

    void putPacket();

private:
    void initGui();

private:
    QTimer* m_pullTimer;
    AudioOutAbstract* m_audioOut;

};

#endif // PLAYER_H
