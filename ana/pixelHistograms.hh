#ifndef PIXELHISTOGRAMS_H
#define PIXELHISTOGRAMS_H

#include <string>
#include <vector>

#include <TH2D.h>
#include <TH1D.h>

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
  void saveHistograms();
  void plotHistograms();

  void  setCalPixelQualityLM(calPixelQualityLM* calPixelQualityLM) {fCalPixelQualityLM = calPixelQualityLM;}
  void  setRun(int run) {fRun = run;}
  
  bool  goodPixel(uint32_t pixelid, uint32_t time, double ns, unsigned long debug_si_data);

  TH2D* getTH2D(std::string hname);
  TH1D* getTH1D(std::string hname);

private:
  int fVerbose, fRun;
  std::string fFilename;
  std::vector<int> fLayer1, fLayer2, fAllChips;
  calPixelQualityLM* fCalPixelQualityLM;

  std::map<std::string, TH2D*> fTH2D;
  std::map<std::string, TH1D*> fTH1D;
};


#endif