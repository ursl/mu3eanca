#ifndef ANALADDER_H
#define ANALADDER_H

#include <string>
#include <fstream>
#include <TTree.h>

struct noise_scan {
  std::vector<int> ThHigh;
  std::vector<int> NoisyPixels;
  std::vector<int> NoisyHits;
  std::vector<int> Iterations;
  std::vector<int> NotMaskablePixels;
  std::vector<int> Errorrate_link_A;
  std::vector<int> Errorrate_link_B;
  std::vector<int> Errorrate_link_C;
};

struct check_contact {
  std::vector<int> LVCurrent;
};


// ----------------------------------------------------------------------
class anaLadder  {
public:
  anaLadder();
  anaLadder(std::string dirname, std::string ladderPN);
  ~anaLadder();

  void   parseFiles();
  void   parseNoiseScans();
  void   parseCheckContact();
  void   printAll();
  void   bookHist();
  
private:
  std::string fDirectory, fLadderPN;
  std::vector<std::string> fHalves = {"US", "DS"};
 

  std::map<std::string, struct noise_scan> fNoiseScan;
  std::map<std::string, struct check_contact> fCheckContact;
};

#endif
