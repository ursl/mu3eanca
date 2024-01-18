#ifndef ANAFILLHITS_H
#define ANAFILLHITS_H

#include "manalyzer.h"

class AnaFillHits: public TARunObject {
    public:
    AnaFillHits(TARunInfo* runinfo);
    ~AnaFillHits();
    void BeginRun(TARunInfo* runinfo);
    void EndRun(TARunInfo* runinfo);
    TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* event, 
        TAFlags* flags, TAFlowEvent* flow);
     TAFlowEvent* AnalyzeFlowEvent(TARunInfo*, 
        TAFlags* flags, TAFlowEvent* flow) {
            // This function doesn't analyze anything, so we use flags 
            // to have the profiler ignore it
            *flags |= TAFlag_SKIP_PROFILE;
            return flow;
        }    
};



#endif
