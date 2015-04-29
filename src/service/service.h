#ifndef SERVICE_H
#define SERVICE_H

#include <QObject>
#include "serviceconfig.h"

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
    virtual void initNetwork();
    virtual void deinitNetwork();
    virtual void initZeroconf();
    virtual void deinitZeroconf();

private slots:
    void onAnnounce();

private:
    ServiceConfig m_config;
};

#endif // SERVICE_H
