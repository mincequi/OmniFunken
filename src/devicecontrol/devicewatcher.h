#ifndef DEVICEWATCHER_H
#define DEVICEWATCHER_H

#include <QObject>

class devicewatcher : public QObject
{
    Q_OBJECT
public:
    explicit devicemonitor(QObject *parent = 0);
    ~devicemonitor();

signals:
    void start();
    void ready();

public slots:
};

#endif // DEVICEWATCHER_H
