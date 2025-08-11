#ifndef HITDATANOISE_H
#define HITDATANOISE_H

#include <iostream>
#include <vector>

#include <TROOT.h>
#include <TBranch.h>
#include <TVector3.h>
#include <TChain.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TTimeStamp.h>

#include "hitDataBase.hh"
#include "sensor.hh"
#include "util/util.hh"

// ----------------------------------------------------------------------
class hitDataNoise : public hitDataBase {
public:
  hitDataNoise(TChain *tree, std::string para);
  ~hitDataNoise();

  void   bookHist(int runnumber);
  void   eventProcessing();

  void   runEndAnalysis(int runnumber);
  void   writeNoiseMaskFile(std::vector<uint8_t> noise, int runnumber, int chipID,
                            std::string name, std::string dir);
 
  
private:
 
  std::vector<TH2F *> fhitmaps;
  std::vector<TH1F *> fnoisemaps;
 
  std::vector<int> funique_chipIDs;
  TH1F* fhErrors, *fhTotal, *fhRatio; 
 
  int         fModeNoiseLimit;
  double      fNoiseLevel;
};

#endif
