#ifndef AUDIO_BUFFER_H
#define AUDIO_BUFFER_H

#include <QObject>

class AudioBuffer : public QObject
{
    Q_OBJECT
public:
    explicit AudioBuffer(QObject *parent = 0);

signals:

public slots:

};

#endif // AUDIO_BUFFER_H
