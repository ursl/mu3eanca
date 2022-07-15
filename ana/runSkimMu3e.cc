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

#include "util/util.hh"

#include "skimMu3e.hh"

using namespace std;


// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// %% Usage examples:
// %% bin/runSkimMu3e -f ~/data/mu3e/run2022/root_output_files/mu3eTree01066.root -p noise
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

int main(int argc, char *argv[]) {

  int processID = gSystem->GetPid();
  cout << "Running under process ID  " << processID << endl;

  string fileName;
  int file(0);
  int dirspec(0);
  int nevents(-1), start(-1);
  int verbose(-99);
  bool readMaskFiles(true);
  
  // Change the MaxTreeSize to 100 GB (default since root v5.26)
  TTree::SetMaxTreeSize(100000000000ll); // 100 GB

  // -- Some defaults
  string dirBase(".");                      // this could point to "/home/ursl/data/root/."
  string dirName("results"); dirspec = 1;   // and this to, e.g. "bmm", "bee", "bem", ...
  string oDirName(dirName);
  string cutFile("tree.default.cuts");

  string treeName("mu3e");
  string proc("noise");
  string mode("1/1.5"); //noiseMode/noiseLevel
  string histfile("");

  // -- command line arguments
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i],"-h")) {
      cout << "List of arguments:" << endl;
      cout << "-c filename    chain definition file" << endl;
      cout << "-C filename    file with cuts" << endl;
      cout << "-D path        where to put the output" << endl;
      cout << "-f filename          single file instead of chain" << endl;
      cout << "-m {noise|...}       mode details for proc" << endl;
      cout << "-n integer           number of events to run on" << endl;
      cout << "-p {noise|pixel|...} proc" << endl;
      cout << "-s number            seed for random number generator" << endl;
      cout << "-S start             starting event number" << endl;
      cout << "-o filename          set output file" << endl;
      cout << "-v level             set verbosity level" << endl;
      cout << "-h                   prints this message and exits" << endl;
      return 0;
    }
    if (!strcmp(argv[i],"-c"))  {fileName   = string(argv[++i]); file = 0; }     // file with chain definition
    if (!strcmp(argv[i],"-C"))  {cutFile    = string(argv[++i]);           }     // file with cuts
    if (!strcmp(argv[i],"-D"))  {dirName    = string(argv[++i]);  dirspec = 1; } // where to put the output
    if (!strcmp(argv[i],"-f"))  {fileName   = string(argv[++i]); file = 1; }     // single file instead of chain
    if (!strcmp(argv[i],"-m"))  {mode       = string(argv[++i]); }               // mode
    if (!strcmp(argv[i],"-n"))  {nevents    = atoi(argv[++i]); }                 // number of events to run
    if (!strcmp(argv[i],"-p"))  {proc       = string(argv[++i]); }               // proc
    if (!strcmp(argv[i],"-o"))  {histfile   = string(argv[++i]); }               // set output file
    if (!strcmp(argv[i],"-S"))  {start = atoi(argv[++i]); }                      // set start event number
    if (!strcmp(argv[i],"-v"))  {verbose    = atoi(argv[++i]); }                 // set verbosity level
  }


  // -- Prepare histfilename variation with (part of) cut file name
  TString fn(cutFile);
  fn.ReplaceAll("cuts/", "");
  fn.ReplaceAll(".cuts", "");
  fn.ReplaceAll("tree", "");

  // -- Determine filename for output histograms and 'final' small/reduced tree
  TString meta = fileName.c_str();
  if(histfile == "") {
    TString  barefile(fileName.c_str()), chainFile, meta;
    if (file == 0) {
      // -- input from chain
      if (barefile.Contains("chains/")) {
        barefile.ReplaceAll("chains/", "");
        histfile = barefile + "." + fn + ".root";
        if (dirspec) {
          if (dirName[0] == '/') {
            histfile = dirName + "/" + histfile;
          } else {
            histfile = dirBase + "/" + dirName + "/" + histfile;
          }
        }
      } else {
        histfile =  barefile + "." + fn + ".root";
        if (dirspec) {
          if (dirName[0] == '/') {
            histfile = dirName + "/" + histfile;
            oDirName = dirName; 
          } else {
            histfile = dirBase + "/" + dirName + "/" + histfile;
            oDirName = dirBase + "/" + dirName; 
          }
        }
      }
      // -- The following lines strip everything from the string up to and including the last '/'
      int fl = barefile.Last('/');
      TString bla(barefile);
      bla.Replace(0, fl+1, ' '); bla.Strip(TString::kLeading, ' ');  bla.Remove(0,1);
      histfile =  bla + "." + fn + ".root";
      if (dirspec) {
        histfile = dirBase + "/" + dirName + "/" + histfile;
        oDirName = dirBase + "/" + dirName;
      }
    }  else if (file == 1) {
      // -- single file input
      // -- The following lines strip everything from the string up to and including the last '/'
      int fl = barefile.Last('/');
      TString bla(barefile);
      bla.Replace(0, fl+1, ' '); bla.Strip(TString::kLeading, ' ');  bla.Remove(0,1);
      histfile =  bla;
      replaceAll(histfile, ".root", "");
      histfile +=  "." + fn + ".root";
      if (dirspec) {
        if (dirName[0] == '/') {
          histfile = dirName + "/" + histfile;
          oDirName = dirBase;
        } else {
          histfile = dirBase + "/" + dirName + "/" + histfile;
          oDirName = dirBase + "/" + dirName;
        }
      }
    }
  }
  string shistfile = histfile;
  replaceAll(shistfile, "..", ".");
  histfile = shistfile.c_str();
  cout << "Opening " << histfile << " for output histograms" << endl;
  cout << "Opening " << fileName.c_str() << " for input" << endl;


  // -- Set up chain
  TChain *chain = new TChain(TString(treeName));
  cout << "Chaining ->" << treeName << "<-" << endl;
  char pName[2000];
  int nentries;
  if (file == 0) {
    // -- non-trivial chain input
    ifstream is(meta);
    while(meta.ReadLine(is) && (!meta.IsNull())){
      nentries = -1;
      if (meta.Data()[0] == '#') continue;
      sscanf(meta.Data(), "%s %d", pName, &nentries);
      if (nentries > -1) {
        cout << pName << " -> " << nentries << " entries" << endl;
        chain->Add(pName, nentries);
      } else {
        cout << meta << endl;
        chain->Add(meta);
      }
    }
    is.close();
  } else if (file == 1) {
    // -- single file input
    cout << fileName << endl;
    chain->Add(TString(fileName));
  }

  // -- Now instantiate the tree-analysis class object, initialize, and run it ...
  skimMu3e *a = NULL;
  if (string::npos != proc.find("noise")) {
    cout << "instantiating skimMu3e " << endl;
    a = new skimMu3e(chain, treeName);
    readMaskFiles = false; 
  }
  
  if (a) {
    a->setVerbosity(verbose);
    a->setOutputDirectoryName(oDirName);
    a->openHistFile(histfile);
    //    a->readCuts(cutFile.c_str(), 1);
    a->bookHist(0);

    a->startAnalysis();
    a->loop(nevents, start, readMaskFiles);
    a->endAnalysis();
    a->closeHistFile();
  }

  delete a; // so we can dump some information in the destructor

  return 0;
}
