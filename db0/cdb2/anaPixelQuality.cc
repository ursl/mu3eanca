#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>
#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TH1.h>
#include <string.h>

#include "cdbRest.hh"
#include "cdbJSON.hh"

#include "Mu3eConditions.hh"

#include "calPixelQualityLM.hh"

using namespace std;

// ----------------------------------------------------------------------
// anaPixelQuality [-v] [-b] [-f FILENAME]
// ----------------
//
// Analyze pixel quality data
//
// -v          verbose output
// -o FILENAME output filename
// -f first    runnumber
// -l last     runnumber
//
// ----------------------------------------------------------------------

// -- main function
int main(int argc, char *argv[]) {

  // -- command line arguments
  string filename("");
  string gt("datav6.2=2025Beam");
  string db("/Users/ursl/data/mu3e/cdb");
  int first(0), last(0);
  int verbose(0);
  
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-o"))     {filename = argv[++i];}
    if (!strcmp(argv[i], "-db"))    {db = argv[++i];}
    if (!strcmp(argv[i], "-f"))     {first = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-g"))     {gt = argv[++i];}
    if (!strcmp(argv[i], "-l"))     {last = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-v"))     {verbose = 1;}
  }
  

  cdbAbs *pDB(0);
  if (string::npos != db.find("rest") || string::npos != db.find("http://")) {
    string ms("http://pc11740.psi.ch/cdb");
    pDB = new cdbRest(gt, ms, verbose);
  } else {
    // -- hope for the best that this is a JSON directory
    pDB = new cdbJSON(gt, db, verbose);
  }

  // -- initialize the conditions
  Mu3eConditions *pDC = Mu3eConditions::instance(gt, pDB);
  pDC->setVerbosity(verbose);
  vector<string> vTags = pDC->getTags();
  vector<int> vIoV;
  bool foundPQ(false);
  for (auto it: vTags) {
    if (string::npos != it.find("pixelqualitylm_")) {
      foundPQ = true;
      vIoV = pDC->getIOVs(it);
    }
  }
  if (!foundPQ) {
    cout << "ERROR: no pixel quality data found" << endl;
    return 0;
  }

  for (auto it: vIoV) {
    cout << it << endl;
  }



  return 0;
}