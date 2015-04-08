#ifndef SERVICE_H
#define SERVICE_H

#include <QObject>
#include "serviceconfig.h"

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
    virtual void initAudioOut();
    virtual void deinitAudioOut();
    virtual void initDeviceControl();
    virtual void deinitDeviceControl();
    virtual void initNetwork();
    virtual void deinitNetwork();
    virtual void initZeroconf();
    virtual void deinitZeroconf();

private:
    ServiceConfig m_config;

    DeviceControlAbstract *m_deviceControl;
};

#endif // SERVICE_H
