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
    , dirName("/Users/ursl/mu3e/2024-laddertests/results-2")
    , slist("nada")
    ;
  int mode(-1), verbose(-99);
  
  // -- command line arguments
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i],"-h")) {
      cout << "List of arguments:" << endl;
      cout << "-D dirname           set directoryname with laddertest output" << endl;
      cout << "-l ladder1,ladder2   use list for comma-separated list of ladders" << endl;
      cout << "-m mode              use mode for predefined set of ladders" << endl;
      cout << "-h                   prints this message and exits" << endl;
      cout << "-v X                 set verbosity to level X" << endl;
      return 0;
    }
    if (!strcmp(argv[i],"-v"))  {verbose  = atoi(argv[++i]); }           // set verbosity level
    if (!strcmp(argv[i],"-D"))  {dirName  = argv[++i]; }                 // directory with input files
    if (!strcmp(argv[i],"-l"))  {slist    = argv[++i]; }                 // list of ladders
    if (!strcmp(argv[i],"-m"))  {mode     = atoi(argv[++i]); }           // mode
  }

  vector<string> ladderList;
  if (1 == mode) {
    ladderList = {
      "408_1_9"
      , "408_1_11"
      , "408_1_12"
      , "408_1_14"
      , "408_1_21"
      , "408_2_7"
      , "408_2_9"
      , "408_2_12"
      , "408_3_3"};
  }
  if (2 == mode) {
    ladderList = {
      "551_1_1", "551_1_2", "551_1_3", "551_1_4", "551_1_5", 
      "551_2_1", "551_2_2", "551_2_3", "551_2_4", "551_2_5", 
      "551_3_1", "551_3_2", "551_3_4", "551_3_5"};
  }

  if (slist != "nada") {
    ladderList = split(slist, ',');
    mode = 0;
  }
  
  anaEnsemble *a(0);
  if (mode > -1) {
    a = new anaEnsemble(dirName, ladderList);
  } else {   
    a = new anaEnsemble(dirName);
  }

  delete a;

  cout << "This is the end of the world as we know it" << endl;

  return 0;
}
