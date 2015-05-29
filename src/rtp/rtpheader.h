#ifndef RTPHEADER_H
#define RTPHEADER_H

#include "airtunes/airtunesconstants.h"

#include <stdint.h>

struct RtpHeader {
    uint8_t version;
    bool    padding;
    bool    extension;
    uint8_t csrcCount;
    //bool    marker;
    airtunes::PayloadType payloadType;
    uint16_t sequenceNumber;
    uint32_t timestamp;
    uint32_t ssrc;
};

void readHeader(const char* data, RtpHeader *header);

#endif // RTPHEADER_H
