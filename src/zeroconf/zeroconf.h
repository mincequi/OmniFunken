#ifndef ZEROCONF_H
#define ZEROCONF_H

#include <QString>

#define MDNS_RECORD  "tp=UDP", "sm=false", "ek=1", "et=0,1", "cn=0,1", "ch=2", \
                "ss=16", "sr=44100", "vn=3", "txtvers=1", "pw=false"

class ZeroconfInterface
{
public:
    virtual ~ZeroconfInterface() {}

    virtual int registerService(const QString &name, quint16 port) = 0;
    virtual void unregisterService() = 0;
};

#endif // ZEROCONF_H
