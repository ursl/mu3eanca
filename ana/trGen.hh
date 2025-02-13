#ifndef TRGEN_H
#define TRGEN_H

#include <iostream>
#include <vector>

#include "TVector3.h"

#include "trBase.hh"

class trGen: public trBase {
public:
  trGen(TChain *tree, std::string para);
  ~trGen();

  void       commonVar();
  void       printBranches();

  void       bookHist();
  void       readCuts(std::string filename, int dump = 1);

  void       startAnalysis();
  void       endAnalysis();
  void       eventProcessing();
  void       initVariables();
  void       fillHist();
  void       closeHistFile();

  // -- studies
  void       genStudy();
  void       overlapHitsInVertex();

  // -- creates a map between tid (unique trajectory ID) and hit_pixelid (in mu3e tree) associated with that tid
  void       mapTID2PixelID();
  TVector3   getHitLocation(uint32_t hit_pixelid);

protected:

  std::map<int, std::vector<int>> fMapTID2Hits;

  // -- Cut values
  double PTLO, PTHI;
  int TYPE;

};


#endif
