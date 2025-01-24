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

  // -- utilities
  int        pixelID(uint32_t hit_pixelid) {return ((hit_pixelid & (0xffff << 16)) >> 16);}
  int        pixelRow(uint32_t hit_pixelid) {return (hit_pixelid & 0x000000ff);}
  int        pixelCol(uint32_t hit_pixelid) {return ((hit_pixelid & 0x0000ff00) >> 8);}
  
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
