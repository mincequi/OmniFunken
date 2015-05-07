#ifndef COMMON_H
#define COMMON_H

#include <QtGlobal>

namespace airtunes
{
    const quint16 framesPerPacket = 352;
    const quint32 sampleRate = 44100;
    const quint16 sampleSize = 16;
    const quint8  channels = 2;

    enum PayloadType {
        TimingRequest       = 82,
        TimingResponse      = 83,
        Sync                = 84,
        RetransmitRequest   = 85,
        RetransmitResponse  = 86,
        AudioData           = 96,
    };
} // namespace airtunes

#endif // COMMON_H
