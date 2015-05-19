#ifndef COMMON_H
#define COMMON_H

#include <sys/types.h>

namespace airtunes
{
    const uint framesPerPacket = 352;
    const uint sampleRate = 44100;
    const uint sampleSize = 16;
    const uint channels = 2;

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
