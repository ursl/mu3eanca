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
#include "cdbUtil.hh"
#include "base64.hh"

#include "TCanvas.h"
#include "TStyle.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TMath.h"
#include "TKey.h"
#include "TROOT.h"
#include "TSystem.h"

#include <map>
#include <cstdint>

using namespace std;

// ----------------------------------------------------------------------
// anaSortFileCombine combined.root run*.root
// ------------------
// 
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
int main(int argc, char** argv) {

  if (argc < 2) {
    cout << "Usage: " << argv[0] << " combined.root run*.root" << endl;
    return 1;
  }

  TFile* out = new TFile(argv[1], "RECREATE");
  if (!out || out->IsZombie()) {
    cout << "Error: could not open " << argv[1] << endl;
    return 1;
  }

  for (int i = 2; i < argc; ++i) {

    out->cd();
    string runName = argv[i];
    cout << "Processing " << runName << endl;
    size_t slash = runName.rfind('/');
    if (slash != string::npos) runName = runName.substr(slash + 1);
    replaceAll(runName, ".root", "");
    TDirectory* outRun = out->mkdir(runName.c_str());
    outRun->cd();

    TFile* in = new TFile(argv[i], "READ");
    if (!in || in->IsZombie()) {
      cout << "Error: could not open " << argv[i] << endl;
      return 1;
    }


    TIter next(in->GetListOfKeys());
    TKey* key = 0;
    bool any = false;
    while ((key = (TKey*)next())) {
      any = true;
      TObject* obj = key->ReadObj();
      if (!obj) {
        cout << "Error: could not read object " << key->GetName() << " from " << argv[i] << endl;
        return 1;
      }
      TH1* h = dynamic_cast<TH1*>(obj);
      if (h) h->SetDirectory(outRun);
      else obj->Write();
    }
    if (!any) {
      cout << "Error: could not get key from " << argv[i] << endl;
      return 1;
    }
    outRun->Write();
    in->Close();

  }


  out->Write();
  out->Close();
  return 0;
}
