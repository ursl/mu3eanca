#ifndef HITDATAPIXEL_H
#define HITDATAPIXEL_H

#include <iostream>
#include <vector>

#include <TROOT.h>
#include <TBranch.h>
#include <TVector3.h>
#include <TChain.h>
#include <TFile.h>
#include <TTree.h>
#include <TTimeStamp.h>

#include "hitDataBase.hh"

#define DR      57.29577951

class hitDataPixel : public hitDataBase {
public:
  hitDataPixel(TChain *tree, std::string para);
  ~hitDataPixel();

  void        bookHist(int runnumber);
  void        eventProcessing();
  int         cntChipHits(int chipid);

private:
  int fChipID, fcol, frow, ftot, fqual; 
  int fChipHits, fEvtHits; 
};


#endif
