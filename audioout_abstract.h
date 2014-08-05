#ifndef AUDIOOUT_ABSTRACT_H
#define AUDIOOUT_ABSTRACT_H

#include <QObject>

class AudioOutAbstract : public QObject
{
    Q_OBJECT
public:
    AudioOutAbstract() : QObject() {}

    virtual const char *name() const = 0;
    virtual void init(const char *deviceName = NULL) = 0;
    virtual void play(char *data, int samples) = 0;
    virtual void deinit() = 0;

public slots:
    virtual void setVolume(float volume) { Q_UNUSED(volume) }
};


#endif // AUDIOOUT_ABSTRACT_H
