#ifndef RTPSTAT_H
#define RTPSTAT_H

#include <stdint.h>

struct RtpStat {
    uint32_t put;
    uint32_t take;
    uint32_t duplicates;
    uint32_t expected;
    uint32_t early;
    uint32_t late;
    uint32_t lost;
    uint32_t flush;
    uint32_t overflow;
    uint32_t underrun;
    
    void init() {
      put = 0;
      take = 0;
      duplicates = 0;
      expected = 0;
      early = 0;
      late = 0;
      lost = 0;
      flush = 0;
      overflow = 0;
      underrun = 0;
    }
    
    RtpStat() { init(); }
};

#endif // RTPSTAT_H
