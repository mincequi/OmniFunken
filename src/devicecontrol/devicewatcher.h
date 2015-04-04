#ifndef DEVICEWATCHER_H
#define DEVICEWATCHER_H

#include <QMap>
#include <QObject>
#include <QThread>

class Worker;

class DeviceWatcher : public QObject
{
    Q_OBJECT
public:
    typedef QMap<QString,QString> UDevProperties;

    explicit DeviceWatcher(QObject *parent = 0);
    ~DeviceWatcher();

signals:
    void ready();

public slots:
    void start(const QString &action, const UDevProperties &properties);

private:
    bool m_started;
}; // DeviceWatcher

#endif // DEVICEWATCHER_H
