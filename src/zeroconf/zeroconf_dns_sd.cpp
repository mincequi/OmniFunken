#include "zeroconf_dns_sd.h"

#include <dns_sd.h>

#include <QtEndian>


ZeroconfDnsSd::ZeroconfDnsSd(const QString &macAddress, QObject *parent) :
    QObject(parent),
    m_macAddress(macAddress),
    m_dnssref(0)
{
    m_macAddress.remove(QChar(':'));
}

int ZeroconfDnsSd::registerService(const char *name, uint16_t port)
{
    QByteArray mdnsName(m_macAddress.toLatin1());
    mdnsName.append("@");
    mdnsName.append(name);

    const char *record[] = { MDNS_RECORD, NULL };
    uint16_t length = 0;
    const char **field;

    // Concatenate string contained i record into buf.
    for (field = record; *field; ++field) {
        length += strlen(*field) + 1; // One byte for length each time
    }

    char *buf = new char[length * sizeof(char)];
    if (buf == NULL) {
        qWarning("dns_sd: buffer record allocation failed");
        return -1;
    }

    char *p = buf;

    for (field = record; *field; ++field) {
        char * newp = stpcpy(p + 1, *field);
        *p = newp - p - 1;
        p = newp;
    }

    DNSServiceErrorType error;
    error = DNSServiceRegister(&m_dnssref,
                               0,
                               kDNSServiceInterfaceIndexAny,
                               mdnsName.constData(),
                               "_raop._tcp",
                               "",
                               NULL,
                               qToBigEndian(port),
                               length,
                               buf,
                               NULL,
                               NULL);

    free(buf);

    if (error == kDNSServiceErr_NoError) {
        return 0;
    } else {
        qWarning("dns-sd: DNSServiceRegister error %d", error);
        return -1;
    }
}

void ZeroconfDnsSd::unregisterService()
{
}