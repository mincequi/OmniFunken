#ifndef AUDIO_OUT_H
#define AUDIO_OUT_H

#include <QObject>

class AudioOutAbstract : public QObject
{
    Q_OBJECT

public:
    virtual ~AudioOutAbstract() {}

    virtual void play(void *data, int samples) = 0;


};

#endif // AUDIO_OUT_H
