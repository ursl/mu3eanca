#ifndef PLOTRESULTS_h
#define PLOTRESULTS_h

#include "util/plotClass.hh"

#include <fstream>
#include <iostream>

// ----------------------------------------------------------------------
class plotResults: public plotClass {

public :
  plotResults(std::string dir = "results",
              std::string files = "plotResults.files",
              std::string cuts = "baseCuts.cuts",
              std::string setup = "");

  virtual ~plotResults();

  // -- code for loops
  void   loopFunction1();
  void   loopFunction2();

  void   loopOverTree(TTree *t, int ifunc, int nevts = -1, int nstart = 0);

  // -- main analysis methods, local and overriding
  void   makeAll(std::string what = "all");
  void   loadFiles(std::string afiles);
  void   resetHistograms(bool deleteThem = false);


  void   scanAnalyses();
  double totalMuons(TH1 *h);
};

#endif
