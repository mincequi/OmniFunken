#ifndef RTPPACKET_H
#define RTPPACKET_H


#include <QtGlobal>


struct RtpPacket {
    RtpPacket() :
        sequenceNumber(0),
        status(PacketFree),
        flush(false),
        payloadSize(0),
        payload(NULL) {}
    void init() {
        sequenceNumber = 0;
        status = PacketFree;
        flush = false;
    }

    quint16         sequenceNumber;
    quint32         timestamp;
    enum Status {
        PacketFree,
        PacketOk,
        PacketMissing
    }               status;
    bool            flush;
    int             payloadSize;
    char            *payload;
};
    
#endif // RTPPACKET_H    
