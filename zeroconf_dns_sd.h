#ifndef ZEROCONFDNSSD_H
#define ZEROCONFDNSSD_H

#include <QObject>

#include "zeroconf.h"

#include <dns_sd.h>


class ZeroconfDnsSd : public QObject, public ZeroconfInterface
{
    Q_OBJECT
public:
    explicit ZeroconfDnsSd(QObject *parent = 0);

    int registerService(const char *name, uint16_t port);
    void unregisterService();

signals:

public slots:

private:
    DNSServiceRef m_dnssref;

};

#endif // ZEROCONFDNSSD_H
