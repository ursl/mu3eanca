#ifndef FRAME_TREE_H
#define FRAME_TREE_H

#include <string>
#include <vector>

#include <TH1D.h>
#include <TFile.h>
#include <TTree.h>

#include <map>

#include "pixelHit.hh"
#include "track.hh"


//class Hit;

// ----------------------------------------------------------------------
class frameTree {
public:
  static frameTree* instance(std::string filename = "frameTree");

  void  setRun(int run) {fRun = run;}
  void  setFrameID(unsigned long frameID) {fFrameID = frameID;}
  void  setRunAndFrameID(int run, unsigned long frameID) {fRun = run; fFrameID = frameID;} 
  void  setOutDir(std::string outdir) {fOutDir = outdir;}
  void  setVerbose(int verbose) {fVerbose = verbose;}

  TDirectory* getDirectory() {return fDirectory;}
  TFile* getFile() {return fFile;}

  void clearHitsTreeVariables();
  void clearTrackTreeVariables();
  void fillPixelHit(pixelHit &hit);
  void fillTrack(track &trk);
//  void fillSiHit(Hit *);
  void fillFrame();
  void saveTree();
  void closeFile();

  int findHitIndex(uint32_t hitID);
  unsigned long getFrameID() {return fFrameID;}
  int getRun() {return fRun;}

  protected:
  frameTree(std::string filename);
  ~frameTree(); 

  void init(std::string filename);

private:
  static frameTree* fInstance;

  int fVerbose, fRun, fRun0;
  std::string fFilename, fOutDir;
  TFile *fFile;

  TDirectory *fDirectory;
  TTree *fHitsTree;
  unsigned long fFrameID; // keep a record to know when to write the previous frame

  // -- hit tree variables
  int fHitsN;
  static const int NHITMAX = 10000, NTRKMAX = 1000, NTRKHITMAX = 20;
  uint32_t fHitID[NHITMAX];
  uint32_t fHitTS[NHITMAX];
  uint32_t fHitRawToT[NHITMAX];
  uint32_t fHitStatus[NHITMAX];
  uint32_t fHitFrameID[NHITMAX];
  double fHitX[NHITMAX];
  double fHitY[NHITMAX];
  double fHitZ[NHITMAX];
  double fHitTime[NHITMAX];

  // -- track tree variables
  int fTrkN;
  double fTrkMomentum[NTRKMAX];
  double fTrkChi2[NTRKMAX];
  int   fTrkType[NTRKMAX];
  double fTrkPhi[NTRKMAX];
  double fTrkLambda[NTRKMAX];  
  double fTrkK[NTRKMAX];
  double fTrkKerr2[NTRKMAX];
  double fTrkT0[NTRKMAX];
  double fTrkT0Err[NTRKMAX];
  double fTrkT0RMS[NTRKMAX];
  double fTrkT0Si[NTRKMAX];
  double fTrkT0SiErr[NTRKMAX];
  double fTrkT0SiRMS[NTRKMAX];
  double fTrkDoca[NTRKMAX];
  int   fTrkSegmentN[NTRKMAX];
  int   fTrkHitOverflow[NTRKMAX];
  int   fTrkNhits[NTRKMAX];
  int   fTrkHitIndices[NTRKMAX][NTRKHITMAX];
};


#endif