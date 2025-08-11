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

#include "pixelHistograms.hh"

using namespace std;
// ----------------------------------------------------------------------
int main(int argc, char *argv[]) {

  string filename;
  // -- command line arguments
  for (int i = 0; i < argc; i++){                             //
    if (!strcmp(argv[i], "-f"))  {filename = argv[++i];}         //
  }

  // -- start time
  TTimeStamp ts0;
  cout << "start time: " << ts0.AsString("lc") << endl;

  pixelHistograms ph(filename);
  ph.plotHistograms("hitmap", "chipmap");
  ph.plotHistograms("hittot", "chipToT");

  TTimeStamp ts1;
  cout << "end time:   " << ts1.AsString("lc") << ", this is the end, my friend." << endl;

  return 0;
}
