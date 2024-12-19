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

  // -- input and preparation
  void   parseFiles();
  void   parseNoiseScans();
  void   parseCheckContact();
  //void parseHitmaps()  FIXME
  //void parseMasks()    FIXME
  void   printAll();
  void   bookHist();

  // -- analysis
  void   anaErrorRate(int stableLinkCut = 500);
  
private:
  std::string fDirectory, fLadderPN, fLadderInformation;
  std::vector<std::string> fHalves = {"US", "DS"};
 
  // -- data from files
  std::map<std::string, struct noise_scan> fNoiseScan;
  std::map<std::string, struct check_contact> fCheckContact;

  // -- analysis results
  struct errorRate {
    int thr;
    int nerror;
    std::vector<int> linkErrors; // int gives the MINIMUM error rate/link. -99 means all > maxErr(1e5)
  };
  
  std::map<std::string, struct errorRate> fAnaErrorRate;
  std::map<std::string, int> fAnaLastStableThreshold;

};

#endif
