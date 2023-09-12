#ifndef CNTEVENTS_H
#define CNTEVENTS_H

#include "manalyzer.h"

class cntEvents: public TARunObject {
public:
  cntEvents(TARunInfo* runinfo);
  ~cntEvents();
  void BeginRun(TARunInfo* runinfo);
  void EndRun(TARunInfo* runinfo);
  TAFlowEvent* Analyze(TARunInfo*, TMEvent*, TAFlags* flags, TAFlowEvent* flow) {
    // This function doesn't analyze anything, so we use flags 
    // to have the profiler ignore it
    *flags |= TAFlag_SKIP_PROFILE;
    return flow;
  };
  TAFlowEvent* AnalyzeFlowEvent(TARunInfo*, TAFlags* flags, TAFlowEvent* flow);

protected:
    
  TFile* dataFile{};
  int fEvtCounter{0};
};

// Because we have the equipment in the constructor, 
// we have to create a non-defaultfactory for this analyzer module

class cntEventsFactory: public TAFactory {
public:

  void Usage(){}

  void Init(const std::vector<std::string> &args);

  void Finish();
   
  cntEvents* NewRunObject(TARunInfo* runinfo)  {
    return new cntEvents(runinfo);
  }
};


#endif
