#ifndef ANAPIXELHISTOS_H
#define ANAPIXELHISTOS_H

#include "manalyzer.h"

class TH1F;
class TH1D;
class TH2F;
class TH2D;

class AnaPixelHistos: public TARunObject {
public:
  AnaPixelHistos(TARunInfo* runinfo);
  ~AnaPixelHistos();
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
  TH1F * chipID{};
  std::map<unsigned int, TH2F*> hPixelHitMap{};
  int fEvtCounter{0};
};

// Because we have the equipment in the constructor, 
// we have to create a non-defaultfactory for this analyzer module

class AnaPixelHistoFactory: public TAFactory {
public:

  void Usage(){}

  void Init(const std::vector<std::string> &args);

  void Finish();
   
  AnaPixelHistos* NewRunObject(TARunInfo* runinfo)  {
    return new AnaPixelHistos(runinfo);
  }
};


#endif
