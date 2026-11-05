#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <filesystem>

#include <cstdint>
#include <vector>
#include <string.h>
#include <chrono>
#include <algorithm> // for std::lower_bound
#include <dirent.h>

#include <fstream>

//#include "cdbUtil.hh" 
#include "Mu3eConditions.hh"
#include "calPixelAlignment.hh"
#include "calPixelQualityLM.hh"

#include "cdbJSON.hh"
#include "base64.hh"

#include <nlohmann/json.hpp>
using json = nlohmann::ordered_json;


#include "asicInfo.hh"
#include "util.hh"

#include "TCanvas.h"
#include "TStyle.h"
#include "TFile.h"
#include "TTree.h"
#include "TH2F.h"
#include "TMath.h"
#include "TKey.h"
#include "TROOT.h"
#include "TSystem.h"

using namespace std;

// ----------------------------------------------------------------------
// fillDataInformation
// -------------------
// 
//  The goal of this simple program is to determine the total hit count/rate for each subdector.
// 
// cd mu3eanca/run2026
//    ./bin/fillQualityPixels -g datav7.1=2026V2
// ----------------------------------------------------------------------



// ----------------------------------------------------------------------
void parseMidasMetaData(string filename);

string MLZRDIR("/Users/ursl/data/mu3e/run2026/mlzr");
string DATADIR("/Users/ursl/data/mu3e/run2026/raw");

//string DATADIR("/data/experiment/mu3e/data/2026/mlzr");


// ----------------------------------------------------------------------
static bool fileExists(const string& filename) {
  ifstream in(filename.c_str());
  return in.good();
}

// ----------------------------------------------------------------------
string getDataSubdir(int runnumber) {
  int block = runnumber / 1000;
  string blockdir = Form("%03d", block);
  return Form("%s", blockdir.c_str());
}

// ----------------------------------------------------------------------
string findFile(string dir, string filename, int run) {
  vector<string> vdirs;  
  for (filesystem::directory_iterator it(dir), end; it != end; ++it) {
    cout << "it->path() = " << it->path() << endl;
    if (is_directory(it->path())) {
        vdirs.push_back(it->path().string());
    }
  }
  sort(vdirs.begin(), vdirs.end(), [](const string& a, const string& b) {
    return filesystem::last_write_time(a) > filesystem::last_write_time(b);
  });

  for (const auto &d : vdirs) {
    string completeName = d + "/" + Form("%s/", getDataSubdir(run).c_str()) + filename;
    cout << "checking file " << completeName << endl;
    if (fileExists(completeName)) {
      cout << "found file " << completeName << endl;
      return completeName;
    }
  }
  return  "NOT FOUND";
}


// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {

  // -- command line arguments
  int verbose(0), run(0);
  string fstub("dqm_histos"), mstub("run"), filename("");
  string gt("datav7.1=2026V2");

  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-g"))      {gt = argv[++i];}
    if (!strcmp(argv[i], "-r"))      {run = atoi(argv[++i]);}
  }
   
  // -- open midas meta data file
  filename = DATADIR + "/" + getDataSubdir(run) + Form("/%s%d.mid.lz4.json", mstub.c_str(), run);
  if (!fileExists(filename)) {
    cout << "XXXXXXXXX fillDataInformation failed to open file " << filename << endl;
    return 0;
  }
  json j; 
  ifstream mf(filename); 
  mf >> j; 
  uint64_t tsLast = j["stat"].value("last_frame_of_the_run", uint64_t{0});
  cout << "tsLast = " << tsLast << " (" << Form("%llu", tsLast) << ")" << endl;

  uint64_t pixelHits = j["stat"].value("n_phit", uint64_t{0});
  uint64_t tileHits = j["stat"].value("n_thit", uint64_t{0});
  uint64_t fibreHits = j["stat"].value("n_fhit", uint64_t{0});
  int      midasEvts = j["stat"].value("n_midasEvent", int{0});

  cout << "pixelHits = " << pixelHits << " (" << Form("%llu", pixelHits) << ")" << endl;
  cout << "tileHits = " << tileHits << " (" << Form("%llu", tileHits) << ")" << endl;
  cout << "fibreHits = " << fibreHits << " (" << Form("%llu", fibreHits) << ")" << endl;
  cout << "midasEvts = " << midasEvts << " (" << Form("%d", midasEvts) << ")" << endl;

  // -- now upload to RDB: any subsystem hit count != is updating the subsystem quality to not-bad

  return 0; 

  // -- below a version accessing (also) the mlzr file for better granularity
  filename = findFile(MLZRDIR, Form("%s_%d.root",fstub.c_str(), run), run);
  if (filename == "NOT FOUND") {
    cout << "XXXXXXXXX fillDataInformation failed to open file " << filename << endl;
    return 0;
  }
  TFile *f = TFile::Open(filename.c_str());
  if (!f) {
    cout << "XXXXXXXXX fillDataInformation failed to open file " << filename << endl;
    return 0;
  }

  // -- read in ALL chipids in VTX
  vector<unsigned int> vchipid;
  vector<string> vStations = {"station_0"};
  map<string, int> vLayers  = {{"layer_1", 8}, {"layer_2", 10}, {"layer_3", 24}};
  map<int, TH2*> pixelHitmaps;

  // -- read in all hitmaps
  for (auto itS : vStations) {
    for (auto itL : vLayers) {
      for (int iLdr = 1; iLdr <= itL.second; ++iLdr) {
        stringstream ss;
        ss << "pixel/hitmaps/" << itS << "/" << itL.first
           << Form("/ladder_%02d", iLdr);
        string s = ss.str();
        if (0) cout << "TDirectory " << s << "<-" << endl;
        f->cd(s.c_str());
        TIter next(gDirectory->GetListOfKeys());
        TKey *key(0);
        while ((key = (TKey *)next())) {
          if (key) {
            if (0) cout << "found key " << s << endl;
            if (gROOT->GetClass(key->GetClassName())->InheritsFrom("TDirectory")) continue;
            if (!gROOT->GetClass(key->GetClassName())->InheritsFrom("TH1")) continue;       
            string name = key->GetName();
            string oname = s + "/" + name;
            if (string::npos != name.find("hitmap_perChip_withToTCut_")) continue;
            if (string::npos != name.find("hitmap_perChip")) {
              unsigned int chipID;
              replaceAll(name, "hitmap_perChip_", "");
              if (verbose > 9) cout << "   .. chipID = ";
              chipID = ::stoi(name); 
              if (verbose > 9) cout << chipID << " from name = " << oname << endl;
              if (0 == chipID) {
                cout << "  XXXXXXXXX  skipping illegal chipID = " << chipID << " from name = " << name << " is 0, skipping" << endl;
                continue;
              }
              vchipid.push_back(chipID);
              TH2F *h = (TH2F*)key->ReadObj();
              pixelHitmaps.insert(make_pair(chipID, h));
            }
          }
        }
      }
    }
  }
  cout << "pixelHitmaps.size() = " << pixelHitmaps.size() << endl;

  // -- Tile are so cool
  int nTileHitsTotal = static_cast<int>(((TH2*)(f->Get("tile/Zphi_TileHitmap_Global")))->GetSumOfWeights());

  // -- Fibres nearly so as well
  int nFibreHitsTotal = static_cast<int>(((TH2*)(f->Get("fibre/channelID_all")))->GetSumOfWeights());

  // -- do analysis
  int nHitsPixelTotal(0);
  for (auto it: pixelHitmaps){
    // -- debug with first 12 only FIXME
    int nhits = pixelHitmaps[it.first]->GetSumOfWeights();
    if (verbose > 5) cout << "chipID = " << it.first << " nhits = " << nhits << endl;
    nHitsPixelTotal += nhits;
  }
  cout << "nHitsPixelTotal = " << nHitsPixelTotal << endl;
  cout << "nTileHitsTotal = " << nTileHitsTotal << endl;
  cout << "nFibreHitsTotal = " << nFibreHitsTotal << endl;

  return 0;
}

// ----------------------------------------------------------------------
void parseMidasMetaData(string filename) {
  if (!fileExists(filename)) {
    cout << "parseMidasMetaData> ERROR: file " << filename << " does not exist" << endl;
    return;
  }
  json j;
  ifstream f(filename);
  f >> j;
} 
