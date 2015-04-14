#ifndef SERVICE_H
#define SERVICE_H

#include <QObject>
#include "serviceconfig.h"

class AudioOutAbstract;
class DeviceControlAbstract;

class Service : public QObject
{
    Q_OBJECT
public:
    explicit Service(const ServiceConfig &serviceConfig, QObject *parent = 0);
    ~Service();

signals:

public slots:
    void open();
    void close();

protected:
    ServiceConfig config() const;
    virtual void initDeviceControl();
    virtual void deinitDeviceControl();
    virtual void initNetwork();
    virtual void deinitNetwork();
    virtual void initZeroconf();
    virtual void deinitZeroconf();

private slots:
    void onAnnounce();

private:
    ServiceConfig m_config;
    AudioOutAbstract *m_audioOut;
    DeviceControlAbstract *m_deviceControl;
};

#endif // SERVICE_H
