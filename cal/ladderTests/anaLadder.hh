#ifndef ANALADDER_H
#define ANALADDER_H

#include <string>
#include <fstream>
#include <TTree.h>

struct noise_scan {
  std::vector<int> NoisyPixels;
  std::vector<int> NoisyHits;
  std::vector<int> Iterations;
  std::vector<int> NotMaskablePixels;
  std::vector<int> Errorrate_link_A;
  std::vector<int> Errorrate_link_B;
  std::vector<int> Errorrate_link_C;
};

// ----------------------------------------------------------------------
class anaLadder  {
public:
  anaLadder();
  anaLadder(std::string dirname, std::string ladderPN);
  ~anaLadder();

  void   parseFiles();
  void   printAll();
  void   bookHist();
  
private:
  std::string fDirectory, fLadderPN;
  std::map<std::string, struct noise_scan> fNoiseScan;
};

#endif
