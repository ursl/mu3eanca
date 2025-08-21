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

// ----------------------------------------------------------------------
class frameTree {
public:
  static frameTree* instance(std::string filename = "frameTree");

  void  setRun(int run) {fRun = run;}
  void  setFrameID(uint32_t frameID) {fFrameID = frameID;}
  void  setRunAndFrameID(int run, uint32_t frameID) {fRun = run; fFrameID = frameID;} 
  void  setOutDir(std::string outdir) {fOutDir = outdir;}
  void  setVerbose(int verbose) {fVerbose = verbose;}

  TDirectory* getDirectory() {return fDirectory;}
  TFile* getFile() {return fFile;}

  void clearHitsTreeVariables();
  void clearTrackTreeVariables();
  void fillPixelHit(pixelHit &hit);
  void fillTrack(track &trk);
  void fillFrame();
  void saveTree();
  void closeFile();

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
  uint32_t fFrameID; // keep a record to know when to write the previous frame

  // -- hit tree variables
  int fHitsN;
  static const int NHITMAX = 10000, NTRKMAX = 1000;
  int fHitPixelID[NHITMAX];
  int fHitToT[NHITMAX];
  unsigned long fHitDebugSiData[NHITMAX];
  int fHitChipID[NHITMAX];
  int fHitCol[NHITMAX];
  int fHitRow[NHITMAX];
  int fHitTime[NHITMAX];
  int fHitTimeNs[NHITMAX];
  Float_t fHitX[NHITMAX];
  Float_t fHitY[NHITMAX];
  Float_t fHitZ[NHITMAX];
  int fHitRawToT[NHITMAX];
  int fHitBitToT[NHITMAX];
  int fHitStatus[NHITMAX];
  int fHitStatusBits[NHITMAX];
  Bool_t fHitValidHit[NHITMAX];

  // -- track tree variables
  int fTrkN;
  float fTrkMomentum[NTRKMAX];
  float fTrkChi2[NTRKMAX];
  int   fTrkType[NTRKMAX];
  float fTrkPhi[NTRKMAX];
  float fTrkLambda[NTRKMAX];  
  int fTrkNhits[NTRKMAX];
  //  int fHitIndices[NTRKMAX][TRKNHITMAX];
  // FIXME still need array PER TRK (i.e. 2D array) for hit pointing
  };


#endif