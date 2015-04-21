#ifndef ZEROCONFDNSSD_H
#define ZEROCONFDNSSD_H

#include <QObject>

#include "zeroconf.h"

#include <dns_sd.h>

class ZeroconfDnsSd : public QObject, public ZeroconfInterface
{
    Q_OBJECT
public:
    explicit ZeroconfDnsSd(const QString &macAddress, QObject *parent = 0);

    int registerService(const QString &name, quint16 port) Q_DECL_OVERRIDE;
    void unregisterService() Q_DECL_OVERRIDE;

signals:

public slots:

private:
    QString         m_macAddress;
    DNSServiceRef   m_dnssref;
};

#endif // ZEROCONFDNSSD_H
