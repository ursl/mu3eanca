#ifndef PIXELHISTOGRAMS_H
#define PIXELHISTOGRAMS_H

#include <string>
#include <vector>

#include <TH2D.h>
#include <TH1D.h>
#include <TFile.h>

#include "mu3e/cdb/calPixelQualityLM.hh"

#include <map>

// ----------------------------------------------------------------------
class pixelHistograms {
public:
  pixelHistograms();
  pixelHistograms(std::string filename);
  ~pixelHistograms();

  void init(int mode);
  void bookHist(std::string hname, std::string hType);
  void readHist(std::string hname, std::string hType);
  void plotAllHistograms();
  void saveHistograms(std::string filename);
  void plotHistograms(std::string hname, std::string htype);

  void  setCalPixelQualityLM(calPixelQualityLM* calPixelQualityLM) {fCalPixelQualityLM = calPixelQualityLM;}
  calPixelQualityLM* getCalPixelQualityLM() {return fCalPixelQualityLM;}

  void  setRun(int run) {fRun = run;}
  void  setOutDir(std::string outdir) {fOutDir = outdir;}
  void  setVerbose(int verbose) {fVerbose = verbose;}
  bool  goodPixel(uint32_t pixelid, uint32_t time, double ns, unsigned long debug_si_data);

  TH2D* getTH2D(std::string hname);
  TH1D* getTH1D(std::string hname);

private:
  int fVerbose, fRun;
  std::string fFilename, fOutDir;
  TFile *fFile;
  std::vector<int> fLayer1, fLayer2, fAllChips;
  calPixelQualityLM* fCalPixelQualityLM;

  std::map<std::string, TH2D*> fTH2D;
  std::map<std::string, TH1D*> fTH1D;
};


#endif