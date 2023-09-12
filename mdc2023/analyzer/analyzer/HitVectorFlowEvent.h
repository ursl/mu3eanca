#ifndef HITVECTORFLOWEVENT_H
#define HITVECTORFLOWEVENT_H

#include "manalyzer.h"
#include <vector>

#include "hits.h"

// flow event creating vectors for the hits 
class HitVectorFlowEvent : public TAFlowEvent
{  public:
    HitVectorFlowEvent(TAFlowEvent* flow, 
            uint64_t * pixelstart, uint32_t npixel, 
            uint64_t * fibrestart, uint32_t nfibre, 
            uint64_t * tilestart, uint32_t ntile);
    
    std::vector<pixelhit> pixelhits;
    std::vector<fibrehit> fibrehits;
    std::vector<tilehit> tilehits;
};


#endif