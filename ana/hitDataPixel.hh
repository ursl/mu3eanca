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
#include "util/util.hh"

#define DR      57.29577951

// ----------------------------------------------------------------------
struct sensor {
  int layer, localLadder, simLadder,  confLadder, simChip, runChip, ladderChip,  direction; 
  TVector3 v;
};

// ----------------------------------------------------------------------
class hitDataPixel : public hitDataBase {
public:
  hitDataPixel(TChain *tree, std::string para);
  ~hitDataPixel();

  void   bookHist(int runnumber);
  void   eventProcessing();
  void   readJSON(std::string filename, std::string dir = "."); 
  int    getValInt(std::string line);
  float  getValFloat(std::string line);
  std::vector<std::string> readEntry(std::vector<std::string> lines, int &iLine);
  struct sensor fillEntry(std::vector<std::string> lines);

  int    countChipHits(int chipid);
  int    getLayer(int chipid);

private:
  int fChipID, fcol, frow, ftot, ftot2, fqual, flayer; 
  int fChipHits, fEvtHits; 
  
  std::map<int, struct sensor> fDetectorChips;

};


#endif
