#ifndef DEVICEWATCHER_H
#define DEVICEWATCHER_H

#include <QObject>
#include <QThread>

class DeviceWatcher : public QObject
{
    Q_OBJECT
    typedef QMap<QString,QString> UDevProperties;

public:
    explicit DeviceWatcher(QObject *parent = 0);
    ~DeviceWatcher();

signals:
    void ready();

public slots:
    void start(const QString &action, const UDevProperties &properties);

private:
    class Worker : public QObject
    {
        Q_OBJECT
    public slots:
        void doStart(const QString &action, const DeviceWatcher::UDevProperties &properties);

    signals:
        void ready();
    };

    bool m_started;
    Worker *m_worker;
    QThread m_workerThread;
}; // DeviceWatcher

#endif // DEVICEWATCHER_H
