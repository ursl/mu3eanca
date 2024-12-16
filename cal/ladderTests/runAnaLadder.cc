#include "anaEnsemble.hh"


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
#include "TSystem.h"
#include "TKey.h"
#include <TObjString.h>
#include <TString.h>

#include "util/util.hh"


using namespace std;


int main(int argc, char *argv[]) {

  int processID = gSystem->GetPid();
  cout << "Running under process ID  " << processID << endl;

  string fileName("nada")
    , dirName("/Users/ursl/mu3e/2024-laddertests/results-1")
    ;
  int verbose(-99);
  
  // -- command line arguments
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i],"-h")) {
      cout << "List of arguments:" << endl;
      cout << "-D dirname           set directoryname with laddertest output" << endl;
      cout << "-h                   prints this message and exits" << endl;
      cout << "-v X                 set verbosity to level X" << endl;
      return 0;
    }
    if (!strcmp(argv[i],"-v"))  {verbose  = atoi(argv[++i]); }           // set verbosity level
    if (!strcmp(argv[i],"-D"))  {dirName  = argv[++i]; }                 // directory with input files
  }

  anaEnsemble *a = new anaEnsemble(dirName);

  cout << "This is the end of the world as we know it" << endl;

  return 0;
}
