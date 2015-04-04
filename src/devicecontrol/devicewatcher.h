#ifndef DEVICEWATCHER_H
#define DEVICEWATCHER_H

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
    class WorkerThread : public QThread
    {
    public:
        explicit WorkerThread(QObject *parent);
        void init(const QString &action, const DeviceWatcher::UDevProperties &properties);

    signals:
        void ready();

    private:
        void run() Q_DECL_OVERRIDE;

    private:
        QString m_action;
        DeviceWatcher::UDevProperties m_properties;
    };

    bool m_started;
}; // DeviceWatcher

#endif // DEVICEWATCHER_H
