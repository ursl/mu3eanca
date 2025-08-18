#ifndef PIXELHISTOGRAMS_H
#define PIXELHISTOGRAMS_H

#include <string>
#include <vector>

#include <TH2D.h>
#include <TH1D.h>
#include <TFile.h>
#include <TProfile2D.h>
#include <TTree.h>

#include "mu3e/cdb/calPixelQualityLM.hh"

#include <map>

struct pixelHit {
  // -- input
  int fPixelID; 
  int fHitToT;  
  unsigned long fDebugSiData;
  int fChipID, fCol, fRow, fTime, fTimeNs;

  // -- calculated locally
  int fRawToT, fBitToT; 
  int fStatus; // 0 = good, 1 = rj, 2 = invalid
  int fStatusBits; // 0 = edge, 1 = low
 
};
  

// ----------------------------------------------------------------------
class pixelHistograms {
public:
  static pixelHistograms* instance(int mode = -1, std::string filename = "pixelHistograms");

  void readHist(std::string hname, std::string hType);
  void plotAllHistograms();
  void saveHistograms();
  void plotHistograms(std::string hname, std::string htype);

  void  setCalPixelQualityLM(calPixelQualityLM* calPixelQualityLM) {fCalPixelQualityLM = calPixelQualityLM;}
  calPixelQualityLM* getCalPixelQualityLM() {return fCalPixelQualityLM;}

  void  setRun(int run) {fRun = run;}
  void  setOutDir(std::string outdir) {fOutDir = outdir;}
  void  setVerbose(int verbose) {fVerbose = verbose;}
//  bool  goodPixel(uint32_t pixelid, uint32_t time, double ns, unsigned long debug_si_data);
  bool  goodPixel(uint32_t frameID, pixelHit &hit);

  TH2D* getTH2D(std::string hname);
  TH1D* getTH1D(std::string hname);

  // -- special histograms for mu3eTrirec
  TH1D *fphitToT, *fpSiHitsSize, *fpMppcHitsSize, *fpTlHitsSize;

protected:
  pixelHistograms(int mode, std::string filename);
  ~pixelHistograms(); 

  void init(int mode, std::string filename);
  void bookHist(std::string hname, std::string hType);
  void clearHitsTreeVariables();
  void fillAnotherHit(pixelHit &hit);
  void fillAnotherFrame(uint32_t frameID);

private:
  static pixelHistograms* fInstance;
  int fVerbose, fRun, fRun0;
  std::string fFilename, fOutDir;
  TFile *fFile;
  std::vector<int> fLayer1, fLayer2, fAllChips;
  calPixelQualityLM* fCalPixelQualityLM;

  std::map<std::string, TH2D*> fTH2D;
  std::map<std::string, TH1D*> fTH1D;
  std::map<std::string, TProfile2D*> fTProfile2D;

  TDirectory *fDirectory;
  TTree *fHitsTree;
  uint32_t fFrameID; // keep a record to know when to write the previous frame

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