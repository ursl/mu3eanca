#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <string.h>
#include <chrono>
#include <algorithm> // for std::lower_bound
#include <dirent.h>
#include <sys/stat.h>

#include <fstream>

#include "Mu3eConditions.hh"
#include "calTileAlignment.hh"
#include "calTileQuality.hh"

#include <nlohmann/json.hpp>
using json = nlohmann::ordered_json;

#include "cdbJSON.hh"
#include "base64.hh"

#include "TCanvas.h"
#include "TStyle.h"
#include "TFile.h"
#include "TTree.h"
#include "TH2F.h"
#include "TMath.h"
#include "TKey.h"
#include "TROOT.h"

using namespace std;

// ----------------------------------------------------------------------
// lookAtSortFile
// --------------
// Look at a sort file and print the contents
//
// Examples:
//          ./bin/lookAtSortFile ~/data/mu3e/run2026/sort/v04/012/run12272-sorted.root
// ----------------------------------------------------------------------



int main(int argc, char** argv) {
  if (argc != 2) {
    cout << "Usage: " << argv[0] << " <sortfile>" << endl;
    return 1;
  }
  string sortfile = argv[1];
  TFile* file = TFile::Open(sortfile.c_str());
  if (!file) {
    cout << "Error: could not open file " << sortfile << endl;
    return 1;
  }
  TTree* tree = (TTree*) file->Get("mu3e");
  if (!tree) {  
    cout << "Error: could not get tree from file " << sortfile << endl;
    return 1;
  }
  tree->Print();
  return 0;
}