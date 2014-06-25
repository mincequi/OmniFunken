#ifndef AUDIO_OUT_ALSA_H
#define AUDIO_OUT_ALSA_H

#include <QObject>
#include "audio_out.h"

class AudioOutAlsa : public QObject//, public AudioOutAbstract
{
    Q_OBJECT
public:
    explicit AudioOutAlsa(QObject *parent = 0);

signals:

public slots:
    void play(void *data, int samples);

};

#endif // AUDIO_OUT_ALSA_H
