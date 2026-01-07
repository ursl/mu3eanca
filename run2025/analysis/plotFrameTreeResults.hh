#ifndef PLOTFRAMETREERESULTS_h
#define PLOTFRAMETREERESULTS_h

#include "../../util/plotClass.hh"
#include "../../util/mu3ePlotUtils.hh"

#include <TTree.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TProfile2D.h>
#include <TCanvas.h>
#include <TPad.h>
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

  // -- main analysis methods, local and overriding
  void   makeAll(std::string what = "all");

  // -- Plotting functions
  void   plotPixelHistograms(std::string histname, std::string htype);
  void   plotAllPixelHistograms();  
  void   readHist(std::string dir, std::string hname, std::string hType);
  void   plotTrkGraphs(int run = -1);
  void   plotTrkGraphsWithTitleFilter(int run = -1, const std::string& titleFilter = "", int filterType = 0);
  void   plotTrkHitmaps(int run = -1);
  void   plotAllOnOnePage(std::string hname, std::string opt = "colz");

  // -- code for loops
  void   loopFunction1();

  void   loopOverTree(TTree *t, int ifunc, int nevts = -1, int nstart = 0);

 private:
  std::map<std::string, TH1D*> fTH1D;
  std::map<std::string, TH2D*> fTH2D;
  std::map<std::string, TProfile2D*> fTProfile2D;

  // -- canvas and pad for plotting
  mu3ePlotUtils fPlotUtils;
  TCanvas *fCanvas;
  TPad *fPad;

  std::vector<int> fLayer1, fLayer2, fAllChips;
  int fRun;
};

#endif
