#ifndef PLOTFRAMETREERESULTS_h
#define PLOTFRAMETREERESULTS_h

#include "../../util/plotClass.hh"
#include <TTree.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TProfile2D.h>

#include <fstream>
#include <iostream>

// ----------------------------------------------------------------------
class plotFrameTreeResults: public plotClass {

public :
  plotFrameTreeResults(std::string dir = "results",
              std::string files = "nada",
              std::string cuts = "nada",
              std::string setup = "");

  virtual ~plotFrameTreeResults();

  // -- Plot
  void   plotPixelHistograms(std::string histname, std::string htype);
  void   plotAllPixelHistograms();  
  void   readHist(std::string hname, std::string hType);


  // -- code for loops
  void   loopFunction1();

  void   loopOverTree(TTree *t, int ifunc, int nevts = -1, int nstart = 0);

  // -- main analysis methods, local and overriding
  void   makeAll(std::string what = "all");


 private:
  std::map<std::string, TH1D*> fTH1D;
  std::map<std::string, TH2D*> fTH2D;
  std::map<std::string, TProfile2D*> fTProfile2D;

  std::vector<int> fLayer1, fLayer2, fAllChips;
  int fRun;
};

#endif
