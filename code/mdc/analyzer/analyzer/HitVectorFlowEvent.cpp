#include "HitVectorFlowEvent.h"

HitVectorFlowEvent::HitVectorFlowEvent(TAFlowEvent* flow, uint64_t * pixelstart, uint32_t npixel, uint64_t * fibrestart, uint32_t nfibre, uint64_t * tilestart, uint32_t ntile)
    :  TAFlowEvent(flow), pixelhits(pixelstart, pixelstart+npixel), fibrehits(fibrestart, fibrestart+nfibre), tilehits(tilestart,tilestart+ntile)
    // Note that this does perform copies. In place construction would mess up ownership, so C++ does not really allow it. 
    // Advantage: We can do type conversion to our structs as we go along
    {

    };

    