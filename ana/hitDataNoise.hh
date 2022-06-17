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
#include <TTimeStamp.h>

#include "hitDataBase.hh"
#include "sensor.hh"
#include "util/util.hh"

#define DR      57.29577951

// ----------------------------------------------------------------------
class hitDataNoise : public hitDataBase {
public:
  hitDataNoise(TChain *tree, std::string para);
  ~hitDataNoise();

  void   bookHist(int runnumber);
  void   eventProcessing();
  void   readJSON(std::string filename, std::string dir = "."); 
  int    getValInt(std::string line);
  float  getValFloat(std::string line);
  std::vector<std::string> readEntry(std::vector<std::string> lines, int &iLine);
  struct sensor fillEntry(std::vector<std::string> lines);

private:
  
  std::map<int, struct sensor> fDetectorChips;

};


#endif
