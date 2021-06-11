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
#include "TTimeStamp.h"

#include "plotResults.hh"

using namespace std;
// ----------------------------------------------------------------------
int main(int argc, char *argv[]) {

  TTimeStamp ts0;
  cout << "start time: " << ts0.AsString("lc") << endl;

  string progName  = argv[0];

  string dir("results"), cuts("nada"), files("nada"), plot("nada"), mode("nada"), setup("nada"), rootfilename("nada"), syear("0");
  int year(0);

  // -- command line arguments
  for (int i = 0; i < argc; i++){                             //
    if (!strcmp(argv[i], "-m"))  {mode  = argv[++i];}         //
    if (!strcmp(argv[i], "-p"))  {plot  = argv[++i];}         //
    if (!strcmp(argv[i], "-r"))  {rootfilename  = argv[++i];} // output
    if (!strcmp(argv[i], "-s"))  {setup = argv[++i];}         //
  }

  // -- results
  if (string::npos != plot.find("nada")) {
    gROOT->Clear();  gROOT->DeleteAll();
    plotResults a(dir, files, cuts, setup);
    if (rootfilename != "nada") a.changeSetup(dir, rootfilename, setup);
    if (mode != "nada") {
      a.makeAll(mode);
    } else {
      a.makeAll();
    }
  }

  TTimeStamp ts1;
  cout << "end time: " << ts1.AsString("lc") << ", this is the end, my friend." << endl;


  return 0;
}
