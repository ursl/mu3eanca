#include "cntEvents.h"

#include <iostream>

#include "HitVectorFlowEvent.h"

using namespace std;

// ----------------------------------------------------------------------    
cntEvents::cntEvents(TARunInfo* runinfo):
  TARunObject(runinfo){
  fModuleName = "cntEvents";
}

// ----------------------------------------------------------------------    
cntEvents::~cntEvents(){};

// ----------------------------------------------------------------------    
void cntEvents::BeginRun(TARunInfo* runinfo) {
  printf("cntEvents::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
}


// ----------------------------------------------------------------------    
void cntEvents::EndRun(TARunInfo* runinfo)  {
  std::cout << "cntEvents::EndRun, cnt = " << fEvtCounter << std::endl;
}

// ----------------------------------------------------------------------    
TAFlowEvent* cntEvents::AnalyzeFlowEvent(TARunInfo*, TAFlags*, TAFlowEvent* flow) {
  if (!flow)
    return flow;

  ++fEvtCounter;
  if (0 == fEvtCounter%500000) cout << "cntEvents> Event counter " << fEvtCounter << endl;
  return flow;
}

// ----------------------------------------------------------------------    
void cntEventsFactory::Init(const std::vector<std::string> &args) {

}
    
// ----------------------------------------------------------------------    
void cntEventsFactory::Finish() {

}
