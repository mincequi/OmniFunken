#ifndef DEVICEWATCHER_H
#define DEVICEWATCHER_H

#include <QMap>
#include <QObject>
#include <QThread>


class DeviceWatcher : public QObject
{
    Q_OBJECT
public:
    typedef QMap<QString,QString> UDevProperties;

    explicit DeviceWatcher(QObject *parent = 0);
    ~DeviceWatcher();

    void setAction(const QString &action);
    void setUDevProperties(const UDevProperties &properties);

signals:
    void ready();

public slots:
    void start(const QString &action, const UDevProperties &properties);

private:
    bool            m_started;
    QString         m_action;
    UDevProperties  m_properties;
}; // DeviceWatcher


class WorkerThread : public QThread
{
    Q_OBJECT
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
}; // WorkerThread


#endif // DEVICEWATCHER_H
