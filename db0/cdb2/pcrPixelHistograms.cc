#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <string.h>
#include <chrono>

#include "Mu3eConditions.hh"
#include "calPixelAlignment.hh"
#include "calPixelQuality.hh"
#include "calPixelQualityV.hh"
#include "calPixelQualityM.hh"
#include "util.hh"

#include "cdbJSON.hh"
#include "base64.hh"

#include "TCanvas.h"
#include "TStyle.h"
#include "TFile.h"
#include "TH2F.h"
#include "TMath.h"
#include "TKey.h"
#include "TROOT.h"

using namespace std;


// ----------------------------------------------------------------------
// pcrPixelHistograms
// ---------------
//
// DO NOT USE ANYMORE! MIGRATED TO run2025/analysis!
// ----------------------------------------------------------------------

#define JSONDIR "/Users/ursl/data/mu3e/cdb"

// ----------------------------------------------------------------------
struct pixhit {
  unsigned int ichip;
  int icol, irow;
  int tot;
  int status;
};

// ----------------------------------------------------------------------
void createPayload(string, calAbs *, map<unsigned int, vector<double> >, string);
void chipIDSpecBook(int chipid, int &station, int &layer, int &phi, int &z);

// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {

  // -- filter on US only?
  bool filterUS(true);

  // -- chipIDs that are in fact turned on
  vector<pair<int, int> > vLayLdrTurnedOn = {
    {1, 2}, {1, 7}, {2, 2}, {2, 3}, {2, 8}
  };
  // -- from https://docs.google.com/document/d/13trDuxYuw8mPrYTQEUO2uBgPBGwcF2SZZyf_sdEJekg/edit?tab=t.0
  // Layer 1 Ladder 2 (top side)
  // Layer 1 Ladder 7 (bottom side)
  // Layer 2 Ladder 2 (top side)
  // Layer 2 Ladder 3 (top side)
  // Layer 2 Ladder 8 (bottom side)



  // -- command line arguments
  int verbose(0), mode(1);
  // note: mode = 1 PixelQuality, 2 PixelQualityV, 3 PixelQualityM
  string jsondir(JSONDIR), filename("nada.root");
  string gt("mcidealv6.1");
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-f"))      {filename = argv[++i];}
    if (!strcmp(argv[i], "-g"))      {gt = argv[++i];}
    if (!strcmp(argv[i], "-j"))      {jsondir = argv[++i];}
    if (!strcmp(argv[i], "-m"))      {mode = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-v"))      {verbose = atoi(argv[++i]);}
  }
  
  cdbAbs *pDB = new cdbJSON(jsondir, verbose);
  Mu3eConditions* pDC = Mu3eConditions::instance(gt, pDB);
  pDC->setRunNumber(1);
  if(!pDC->getDB()) {
      std::cout << "CDB database not found" << std::endl;
  }

  vector<uint32_t> allSensorIDs;
  allSensorIDs.reserve(3000);
  calAbs* cal = pDC->getCalibration("pixelalignment_");
  calPixelAlignment* cpa = dynamic_cast<calPixelAlignment*>(cal);
  uint32_t i = 0;
  cpa->resetIterator();
  while(cpa->getNextID(i)) {
      allSensorIDs.push_back(cpa->id(i));
  }
  
  cout << "fill mChipIDOK from allSensorIDs.size() = " << allSensorIDs.size() << endl;
  map <unsigned int, tuple<int, int, int> > mChipIDOK;
  for (auto chipID : allSensorIDs) {
    bool found(false);
    int station(0), layer(0), phi(0), z(0);
    for (auto itL : vLayLdrTurnedOn) {
      chipIDSpecBook(chipID, station, layer, phi, z);
      // cout << "chipID = " << chipID << " in layer/ladder = " << layer << "/" << phi << endl;
      // -- check if chipID is in the list of turned on chips
      if (itL.first == layer && itL.second == phi && (filterUS ? (z < 4) : true)) {
        // cout << " FOUND chipID = " << chipID << " in layer/ladder = " << layer << "/" << phi << endl;
        found = true;
        break;
      }
    }
    if (found) {
      mChipIDOK.insert({chipID, {layer, phi, z}});
    }
  }

  int run(-1);
  string sbla(filename);
  if (string::npos != filename.find_last_of("/")) {
    string dir = filename.substr(0, filename.find_last_of("/")+1);
    cout << "dir ->" << dir << "<-" << endl;
    replaceAll(sbla, dir, "");
  }
  // -- extract the runnumber. It directly precedes .root
  replaceAll(sbla, ".root", "");
  size_t rIdx = sbla.rfind("_")+1;
  sbla = sbla.substr(rIdx);
  cout << "sbla ->" << sbla << "<-" << endl;
  run = ::stoi(sbla);
  cout << "run = " << run << endl;
  

  string hash = string("tag_pixelquality_") + gt + string("_iov_") + to_string(run);
  
  TFile *f = TFile::Open(filename.c_str());
  
  // -- read in all chipids in VTX
  vector<unsigned int> vchipid;
  vector<string> vStations = {"station_0"};
  map<string, int> vLayers  = {{"layer_1", 8}, {"layer_2", 10}};
  map<int, TH2*> mHitmaps;

  for (auto itS : vStations) {
    for (auto itL : vLayers) {
      for (int iLdr = 1; iLdr <= itL.second; ++iLdr) {
        stringstream ss;
        ss << "pixel/hitmaps/" << itS << "/" << itL.first
           << Form("/ladder_%02d", iLdr);
        string s = ss.str();
        cout << "TDirectory " << s << "<-" << endl;
        f->cd(s.c_str());
        TIter next(gDirectory->GetListOfKeys());
        TKey *key(0);
        while ((key = (TKey *)next())) {
          if (key) {
            cout << "found key " << s << endl;
            if (gROOT->GetClass(key->GetClassName())->InheritsFrom("TDirectory")) continue;
            if (!gROOT->GetClass(key->GetClassName())->InheritsFrom("TH1")) continue;       
            string name = key->GetName();
            if (string::npos != name.find("hitmap_perChip")) {
              unsigned int chipID;
              replaceAll(name, "hitmap_perChip_", "");
              chipID = ::stoi(name); 
              cout << "   .. chipID = " << chipID << " from name = " << name << endl;
              vchipid.push_back(chipID);
              TH2F *h = (TH2F*)key->ReadObj();
              mHitmaps.insert(make_pair(chipID, h));
            }
          }
        }
      }
    }
  }

  TCanvas *c0 = new TCanvas("c0", "c0", 800, 600);
  c0->Divide(3,5);
  int ipad(1);
  gStyle->SetOptStat(0);
  for (auto it: mChipIDOK){
    c0->cd(ipad++);
    shrinkPad(0.1, 0.17);
    mHitmaps[it.first]->Draw("colz");
    mHitmaps[it.first]->SetTitle(Form("Run %d, LAY/LDR/CHP %d/%d/%d", run, get<0>(it.second), get<1>(it.second), get<2>(it.second)));
    mHitmaps[it.first]->GetXaxis()->SetTitle("col");
    mHitmaps[it.first]->GetYaxis()->SetTitle("row");
  }
  c0->SaveAs(Form("run%d_hitmaps.pdf", run));


  for (auto it: mHitmaps) {
    // TODO: check that non-zero entries and that it.first is not in mChipIDOK
  }

  return 0;
}

// ----------------------------------------------------------------------
void createPayload(string hash, calAbs *a, map<unsigned int, vector<double> > mdet, string jsondir) {

  string sblob = a->makeBLOB(mdet);
  
  payload pl;
  pl.fHash = hash;
  pl.fComment = "testing";
  pl.fBLOB = sblob;
  //  cout << "######################################################################" << endl;
  //  cout << "### createPayload" << endl;
  //  a->printBLOB(sblob);
  
  a->writePayloadToFile(hash, jsondir, pl);
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
