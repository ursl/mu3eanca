#ifndef PIXELHISTOGRAMS_H
#define PIXELHISTOGRAMS_H

#include <string>
#include <vector>

#include <TH2D.h>
#include <TH1D.h>
#include <TFile.h>
#include <TProfile2D.h>
#include <TFile.h>


#include "mu3e/cdb/calPixelQualityLM.hh"

#include <map>

#include "pixelHit.hh"
  

// ----------------------------------------------------------------------
// The primary motivation for this class is to HISTOGRAM pixel hits
// used in trirec, depending on their category and possibly conditions
// ----------------------------------------------------------------------
class pixelHistograms {
public:
  pixelHistograms(TFile *file);
  pixelHistograms(std::string filename, std::string outdir = "output");
  ~pixelHistograms(); 

  void readHist(std::string hname, std::string hType);
  void plotAllHistograms();
  void saveHistograms();
  void plotHistograms(std::string hname, std::string htype);

  void  setCalPixelQualityLM(calPixelQualityLM* calPixelQualityLM) {fCalPixelQualityLM = calPixelQualityLM;}
  calPixelQualityLM* getCalPixelQualityLM() {return fCalPixelQualityLM;}

  // -- return 0 if the pixel is good, 1 if it is rejected, 2 if it is a special pixel
  int goodPixel(pixelHit &hitIn);
  int pixelQuality(pixelHit &hitIn);

  TH2D* getTH2D(std::string hname);
  TH1D* getTH1D(std::string hname);

  // -- allow booking from outside this class
  // -- Possible types: chipmap, chipToT, chipprof2d
  void bookHist(std::string hname, std::string hType);

  // -- fill from outside of this class
  void fillPixelHist(std::string name, int chipid, int col, int row, double val);

protected:
  void init(TFile *file);
  void init(std::string filename);

private:
  TFile *fFile;
  TDirectory *fDirectory;
  std::string fOutDir;

  std::vector<int> fLayer1, fLayer2, fAllChips;
  calPixelQualityLM* fCalPixelQualityLM;

  std::map<std::string, TH2D*> fTH2D;
  std::map<std::string, TH1D*> fTH1D;
  std::map<std::string, TProfile2D*> fTProfile2D;

  TH1D* fHistLayer1, *fHistLayer2;
  
  int fRun;

  // -- hit tree variables
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