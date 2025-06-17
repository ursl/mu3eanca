#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <string.h>
#include <chrono>

#include "util.hh"

#include "TCanvas.h"
#include "TStyle.h"
#include "TFile.h"
#include "TH2F.h"
#include "TMath.h"
#include "TKey.h"
#include "TROOT.h"

using namespace std;


// ----------------------------------------------------------------------
// anaV01
// ---------------
//
// Examples:
// bin/anaV01 /data/experiment/mu3e/data/2025/trirec/250613/run03*.root
// ----------------------------------------------------------------------


void chipIDSpecBook(int chipid, int &station, int &layer, int &phi, int &z);


// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {

  // -- command line arguments
  int verbose(0), mode(1);

  // -- read in all files
  vector<string> vFiles;
  for (int i = 1; i < argc; i++) {
    vFiles.push_back(argv[i]);
  }

  // -- loop over all files
  for (auto sFile : vFiles) {
    cout << "file " << sFile << endl;
  }

}



// ----------------------------------------------------------------------
void chipIDSpecBook(int chipid, int &station, int &layer, int &phi, int &z) {
  station = chipid/(0x1<<12);
  layer   = chipid/(0x1<<10) % 4 + 1;
  phi     = chipid/(0x1<<5) % (0x1<<5) + 1;
 
  int zp  = chipid % (0x1<<5);
 
  if (layer == 3) {
    z = zp - 7;
  } else if (layer == 4) {
    z = zp - 6;
  } else {
    z = zp;
  }

}



