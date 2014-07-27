#ifndef AUDIOOUT_ABSTRACT_H
#define AUDIOOUT_ABSTRACT_H


class AudioOutAbstract
{
public:
    AudioOutAbstract() {}
    virtual ~AudioOutAbstract() {}

    virtual const char *name() const = 0;
    virtual void init() = 0;
    virtual void play(void *data, int samples) = 0;
    virtual void deinit() = 0;
};


#endif // AUDIOOUT_ABSTRACT_H
