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

class pixelHit {
public:
  int fPixelID; // input
  int fHitToT;  // input
  unsigned long fDebugSiData; // input
  int fChipID, fCol, fRow, fTime, fNs, fRawToT, fBitToT; // calculated
  int fStatus; // 0 = good, 1 = edge, 2 = low, 3 = rj, 4 = ok
};



// ----------------------------------------------------------------------
class pixelHistograms {
public:
  static pixelHistograms* instance(std::string filename = "unset");

  void readHist(std::string hname, std::string hType);
  void plotAllHistograms();
  void saveHistograms(std::string filename);
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
    TH1D *fphitToT;
    pixelHit fPixelHit;

protected:
  pixelHistograms(std::string filename);
  ~pixelHistograms();

  void init(int mode);
  void bookHist(std::string hname, std::string hType);


private:
  static pixelHistograms* fInstance;
  int fVerbose, fRun;
  uint32_t fFrameID; // keep a record to know when to write the previous frame
  std::string fFilename, fOutDir;
  TFile *fFile;
  std::vector<int> fLayer1, fLayer2, fAllChips;
  calPixelQualityLM* fCalPixelQualityLM;

  std::map<std::string, TH2D*> fTH2D;
  std::map<std::string, TH1D*> fTH1D;
  std::map<std::string, TProfile2D*> fTProfile2D;


  TTree *fHitsTree;
  int fFrameID;
  std::vector<pixelHit> fHits;
};


#endif