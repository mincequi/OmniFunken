#include "rtpheader.h"

#include <QtEndian>

void readHeader(const char* data, RtpHeader *header)
{
    header->version     = (data[0] >> 6) & 0x03;
    header->padding     = (data[0] >> 5) & 0x01;
    header->extension   = (data[0] >> 4) & 0x01;
    header->csrcCount   = (data[0] >> 0) & 0x0f;
    header->marker      = (data[1] >> 7) & 0x01;
    header->payloadType = static_cast<airtunes::PayloadType>((data[1] >> 0) & 0x7f);

    header->sequenceNumber  = qFromBigEndian(*((quint16*)(data+2)));
    header->timestamp       = qFromBigEndian(*((quint32*)(data+4)));
    header->ssrc            = qFromBigEndian(*((quint32*)(data+8)));
}
