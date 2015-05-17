#ifndef RTPHEADER_H
#define RTPHEADER_H

#include "airtunes/airtunesconstants.h"

struct RtpHeader {
    quint8  version;
    bool    padding;
    bool    extension;
    quint8  csrcCount;
    bool    marker;
    airtunes::PayloadType payloadType;
    quint16 sequenceNumber;
    quint32 timestamp;
    quint32 ssrc;
};

void readHeader(const char* data, RtpHeader *header);

#endif // RTPHEADER_H
