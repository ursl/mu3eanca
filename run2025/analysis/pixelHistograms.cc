#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <string.h>
#include <chrono>

#include "Mu3eConditions.hh"
#include "cdbUtil.hh"
#include "calPixelQualityLM.hh"

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
// pixelHistograms
// ---------------
//
// Examples:
// bin/pixelHistograms -f /Users/ursl/mu3e/software/250429/minalyzer/root_output_files/dqm_histos_00553.root
// ----------------------------------------------------------------------


void chipIDSpecBook(int chipid, int &station, int &layer, int &phi, int &z);



// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {

  // -- command line arguments
  int verbose(0), mode(1);
  // note: mode = 1 PixelQuality, 2 PixelQualityV, 3 PixelQualityM
  string jsondir(""), filename("nada.root");
  string gt("intrun");
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-f"))      {filename = argv[++i];}
    if (!strcmp(argv[i], "-g"))      {gt = argv[++i];}
    if (!strcmp(argv[i], "-j"))      {jsondir = argv[++i];}
    if (!strcmp(argv[i], "-m"))      {mode = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-v"))      {verbose = atoi(argv[++i]);}
  }
  
  vector<int> vLayer1, vLayer2;
  vLayer1 = {1,2,3,4,5,6,
             33, 34, 35, 36, 37, 38,
             65, 66, 67, 68, 69, 70,
             97, 98, 99, 100, 101, 102,
             129, 130, 131, 132, 133, 134,
             161, 162, 163, 164, 165, 166,
             193, 194, 195, 196, 197, 198,
             225, 226, 227, 228, 229, 230};
  vLayer2 = {1025, 1026, 1027, 1028, 1029, 1030,
             1057, 1058, 1059, 1060, 1061, 1062,
             1089, 1090, 1091, 1092, 1093, 1094,
             1121, 1122, 1123, 1124, 1125, 1126,
             1153, 1154, 1155, 1156, 1157, 1158,
             1185, 1186, 1187, 1188, 1189, 1190,
             1217, 1218, 1219, 1220, 1221, 1222,
             1249, 1250, 1251, 1252, 1253, 1254,
             1281, 1282, 1283, 1284, 1285, 1286,
             1313, 1314, 1315, 1316, 1317, 1318};

  int station(0), layer(0), phi(0), z(0);
  for (auto ichip: vLayer1) {
    chipIDSpecBook(ichip, station, layer, phi, z);
    cout << "layer 1 chip " << ichip << " station " << station << " layer " << layer << " phi " << phi << " z " << z << endl;
  }
  for (auto ichip: vLayer2) {
    chipIDSpecBook(ichip, station, layer, phi, z);
    cout << "layer 2 chip " << ichip << " station " << station << " layer " << layer << " phi " << phi << " z " << z << endl;
  }

  chipIDSpecBook(103, station, layer, phi, z);
  cout << "chip 103 station " << station << " layer " << layer << " phi " << phi << " z " << z << endl;
  cdbAbs *pDB = new cdbJSON(gt, jsondir, verbose);
  
  calAbs *cpq = new calPixelQualityLM();


  map<unsigned int, vector<double> > mdet{};
  
  int run(-1);
  string barefilename(filename);
  if (string::npos != filename.find_last_of("/")) {
    barefilename = filename.substr(filename.find_last_of("/")+1);
    cout << "barefilename ->" << barefilename << "<-" << endl;
    replaceAll(barefilename, "merged-dqm_histos_", "");
    replaceAll(barefilename, ".root", "");
    run = ::stoi(barefilename);
  }
  
  TFile *f = TFile::Open(filename.c_str());
  
  vector<string> allLadders;
  for (int iLadder = 1; iLadder <= 8; ++iLadder) {
    allLadders.push_back(string("pixel/hitmaps/station_0/layer_1/ladder_") + (iLadder < 10 ? "0" : "") + to_string(iLadder));
  }
  for (int iLadder = 1; iLadder <= 10; ++iLadder) {
    allLadders.push_back(string("pixel/hitmaps/station_0/layer_2/ladder_") + (iLadder < 10 ? "0" : "") + to_string(iLadder));
  }
  
  map<int, TH2*> mChips;
  // -- read in all chipids
  for (auto sLadder : allLadders) {
    f->cd(sLadder.c_str());
    TIter next(gDirectory->GetListOfKeys());
    TKey *key(0);
    while (key = (TKey*)next()) {
      if (gROOT->GetClass(key->GetClassName())->InheritsFrom("TH2")) {
        TH2 *h = (TH2*)key->ReadObj();
        string hname(h->GetName());
        replaceAll(hname, "hitmap_perChip_", "");
        int ichip = ::stoi(hname);  
        cout << "chip " << ichip << " " << hname << endl;
        mChips[ichip] = h;
      }
    }
  }
  
  for (auto ichip : mChips) {
    cout << ichip.first << " " << ichip.second << endl;
  }

  TCanvas *c = new TCanvas("c", "c", 1000, 1000);
  gStyle->SetOptStat(0);
  gStyle->SetPadBorderMode(0);
  gStyle->SetPadBorderSize(0);
  gStyle->SetPadTopMargin(0);
  gStyle->SetPadBottomMargin(0);
  gStyle->SetPadLeftMargin(0);
  gStyle->SetPadRightMargin(0);
  c->Divide(2, 1);
  c->cd(1);
  gPad->SetBottomMargin(0.0);
  gPad->SetLeftMargin(0.0);
  gPad->SetRightMargin(0.0);
  gPad->SetTopMargin(0.0);
  TPad *p = (TPad*)c->GetPad(1);
  p->Divide(6,8);
  for (int i = 0; i < vLayer1.size(); ++i) {    
    p->cd(i+1);
    gPad->SetBottomMargin(0.0);
    gPad->SetLeftMargin(0.0);
    gPad->SetRightMargin(0.0);
    gPad->SetTopMargin(0.0);
    cout << "vLayer1[i] = " << vLayer1[i] << " " << mChips[vLayer1[i]] << endl;
    if (mChips[vLayer1[i]]) {
      mChips[vLayer1[i]]->Draw("col");
    }
  }
  c->cd(2);
  gPad->SetBottomMargin(0.0);
  gPad->SetLeftMargin(0.0);
  gPad->SetRightMargin(0.0);
  gPad->SetTopMargin(0.0);
  p = (TPad*)c->GetPad(2);
  p->Divide(6,10);
  for (int i = 0; i < vLayer2.size(); ++i) {
    p->cd(i+1);
    gPad->SetBottomMargin(0.0);
    gPad->SetLeftMargin(0.0);
    gPad->SetRightMargin(0.0);
    gPad->SetTopMargin(0.0);
    cout << "vLayer2[i] = " << vLayer2[i] << " " << mChips[vLayer2[i]] << endl;
    if (mChips[vLayer2[i]]) {
      mChips[vLayer2[i]]->Draw("col");
    }
  }
  replaceAll(barefilename, ".root", "");
  c->SaveAs(("vtxHitmaps-" + to_string(run) + ".pdf").c_str());


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