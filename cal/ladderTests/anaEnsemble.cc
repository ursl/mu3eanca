#include "anaEnsemble.hh"

#include <dirent.h>  /// for directory reading
#include <algorithm> /// for sorting

#include <TROOT.h>
#include <TBranch.h>
#include <TVector3.h>
#include <TChain.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TTimeStamp.h>

#include "util/util.hh"


using namespace std;

anaEnsemble::anaEnsemble(string dirname): fDirectory(dirname) {
  cout << "anaEnsemble::anaEnsemble ctor, fDirectory = "
       << fDirectory
       << endl;
 
  // -- use sequencer_variables*.json
  vector<string> vfiles;
  DIR *folder;
  struct dirent *entry;
  
  folder = opendir(dirname.c_str());
  if (folder == NULL) {
    puts("Unable to read directory");
    return;
  }
  
  while ((entry=readdir(folder))) {
    if (8 == entry->d_type) {
      string sdirentry = entry->d_name;
      if (string::npos == sdirentry.find("sequencer_variables")) continue;
      vfiles.push_back(entry->d_name);
    }
  }
  closedir(folder);
  
  sort(vfiles.begin(), vfiles.end());

  fEnsemble.clear();
  for (auto it: vfiles) {
    if (string::npos != it.find("_US.json")) continue;
    string lname = it;
    replaceAll(lname, string("sequencer_variables_"), string(""));
    replaceAll(lname, string("_DS.json"), string(""));
    fEnsemble.insert({lname, new anaLadder(dirname, lname)});
  }

  
  for (auto it: fEnsemble) {
    cout << it.first << endl;
    
  }
};

anaEnsemble::~anaEnsemble() {

}

  
