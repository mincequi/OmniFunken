#ifndef RTPPACKET_H
#define RTPPACKET_H


#include <QtGlobal>


struct RtpPacket {
    RtpPacket() :
        sequenceNumber(0),
        status(PacketFree),
        flush(false),
        twice(false),
        requestCount(0),
        payloadSize(0),
        payload(NULL) {}
    void init() {
        sequenceNumber = 0;
        status = PacketFree;
        flush = false;
        twice = false;
        requestCount = 0;
    }

    quint16         sequenceNumber;
    enum Status {
        PacketFree,
        PacketOk,
        PacketMissing
    }               status;
    bool            flush;
    bool            twice;
    quint16         requestCount;
    int             payloadSize;
    char            *payload;
};
    
#endif // RTPPACKET_H    
