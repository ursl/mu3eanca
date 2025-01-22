#ifndef TRGEN_H
#define TRGEN_H

#include <iostream>
#include <vector>

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

protected:

  // -- Cut values
  double PTLO, PTHI;
  int TYPE;

};


#endif
