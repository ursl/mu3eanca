#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <math.h>

#include "TROOT.h"
#include "TRint.h"
#include "TChain.h"
#include "TFile.h"
#include "TDirectory.h"
#include "TString.h"
#include "TRandom.h"
#include "TUnixSystem.h"

#include "plotFrameTreeResults.hh"

using namespace std;

// ----------------------------------------------------------------------
int main(int argc, char *argv[]) {

  string progName  = argv[0]; 
  string dir("results"), 
    ifiles("plotFrameTreeResults.files"), 
    ana("plotFrameTreeResults"),
    setup("nada");
  
  string mode("all");

  // -- command line arguments
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i], "-a"))  {ana    = string(argv[++i]);}
    if (!strcmp(argv[i], "-d"))  {dir    = string(argv[++i]);}
    if (!strcmp(argv[i], "-f"))  {ifiles = string(argv[++i]);}
    if (!strcmp(argv[i], "-m"))  {mode   = string(argv[++i]);}
    if (!strcmp(argv[i], "-s"))  {setup  = string(argv[++i]);}
  }

  gROOT->Clear();  gROOT->DeleteAll();

  if (ana == "plotFrameTreeResults") {
    plotFrameTreeResults a(dir, ifiles, setup);
    if (mode == "all") {
      a.makeAll(mode);  
    } else if (string::npos != mode.find("trkgraphs:")) {
      string patt = mode.substr(mode.find(":") + 1);
      string ptype = mode.substr(mode.rfind(":") + 1);
      int ptypeint = stoi(ptype);
      a.plotTrkGraphsWithTitleFilter(-1, patt, ptypeint);
    } else if (string::npos != mode.find("trkhitmaps")) {
      a.plotTrkHitmaps();
    } else if (mode == "pixelhistograms") {
      a.plotAllPixelHistograms();
    } else if (mode == "p1") {
      a.makeAll("p1");
    } else {
      cout << "plotFrameTreeResults::main() mode = " << mode << " not supported" << endl;
      return 1;
    }
  } 
}
