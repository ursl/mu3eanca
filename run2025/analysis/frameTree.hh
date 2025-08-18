#ifndef PIXELHISTOGRAMS_H
#define PIXELHISTOGRAMS_H

#include <string>
#include <vector>

#include <TH1D.h>
#include <TFile.h>
#include <TTree.h>

#include <map>

#include "pixelHit.hh"

// ----------------------------------------------------------------------
class frameTree {
public:
  static frameTree* instance(int mode = -1, std::string filename = "frameTree");

  void  setRun(int run) {fRun = run;}
  void  setOutDir(std::string outdir) {fOutDir = outdir;}
  void  setVerbose(int verbose) {fVerbose = verbose;}

protected:
  frameTree(int mode, std::string filename);
  ~frameTree(); 

  void init(int mode, std::string filename);
  void clearHitsTreeVariables();
  void fillAnotherHit(pixelHit &hit);
  void fillAnotherFrame(uint32_t frameID);
  void saveTree();

private:
  static frameTree* fInstance;
  int fVerbose, fRun, fRun0;
  std::string fFilename, fOutDir;
  TFile *fFile;

  TDirectory *fDirectory;
  TTree *fHitsTree;
  uint32_t fFrameID; // keep a record to know when to write the previous frame

  // -- hit tree variables
  int fHitsN;
  static const int NHITMAX = 10000;
  int fHitPixelID[NHITMAX];
  int fHitToT[NHITMAX];
  unsigned long fHitDebugSiData[NHITMAX];
  int fHitChipID[NHITMAX];
  int fHitCol[NHITMAX];
  int fHitRow[NHITMAX];
  int fHitTime[NHITMAX];
  int fHitTimeNs[NHITMAX];
  int fHitRawToT[NHITMAX];
  int fHitBitToT[NHITMAX];
  int fHitStatus[NHITMAX];
  int fHitStatusBits[NHITMAX];

};


#endif