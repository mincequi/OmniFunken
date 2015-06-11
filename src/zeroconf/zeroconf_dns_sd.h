#ifndef ZEROCONFDNSSD_H
#define ZEROCONFDNSSD_H

#include "zeroconf.h"
#include <dns_sd.h>

class ZeroconfDnsSd : public ZeroconfInterface
{
public:
    ZeroconfDnsSd();
    ~ZeroconfDnsSd();

    int registerService(const QString &name, quint16 port) Q_DECL_OVERRIDE;
    void unregisterService() Q_DECL_OVERRIDE;

private:
    QString         m_macAddress;
    DNSServiceRef   m_dnssref;
};

#endif // ZEROCONFDNSSD_H
