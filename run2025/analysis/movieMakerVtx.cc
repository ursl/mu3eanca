#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <math.h>
#include <string>
#include <TH2.h>
#include "TROOT.h"
#include "TRint.h"
#include "TChain.h"
#include "TFile.h"
#include "TDirectory.h"
#include "TKey.h"
#include "TPaveText.h"
#include "TString.h"
#include "TRandom.h"
#include "TUnixSystem.h"
#include "TStyle.h"


#include "util.hh"

using namespace std;

// ----------------------------------------------------------------------
int main(int argc, char *argv[]) {

  string dir("/Users/ursl/data/mu3e/run2025/mlzr"), sRuns("nada");
  string mode("tot");
  // -- command line arguments
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i], "-d"))  {dir    = string(argv[++i]);}
    if (!strcmp(argv[i], "-m"))  {mode   = string(argv[++i]);}
    if (!strcmp(argv[i], "-r"))  {sRuns = string(argv[++i]);}
  }

  vector<int> Layer1 = {1,2,3,4,5,6,
    33, 34, 35, 36, 37, 38,
    65, 66, 67, 68, 69, 70,
    97, 98, 99, 100, 101, 102,
    129, 130, 131, 132, 133, 134,
    161, 162, 163, 164, 165, 166,
    193, 194, 195, 196, 197, 198,
    225, 226, 227, 228, 229, 230};
  vector<int> Layer2 = {1025, 1026, 1027, 1028, 1029, 1030,
    1057, 1058, 1059, 1060, 1061, 1062,
    1089, 1090, 1091, 1092, 1093, 1094,
    1121, 1122, 1123, 1124, 1125, 1126,
    1153, 1154, 1155, 1156, 1157, 1158,
    1185, 1186, 1187, 1188, 1189, 1190,
    1217, 1218, 1219, 1220, 1221, 1222,
    1249, 1250, 1251, 1252, 1253, 1254,
    1281, 1282, 1283, 1284, 1285, 1286,
    1313, 1314, 1315, 1316, 1317, 1318};

  for (auto iChip: Layer1) {
    system(Form("rm -f hitmap%d.gif", iChip));
  }
  for (auto iChip: Layer2) {
    system(Form("rm -f hitToT%d.gif", iChip));
  }

  vector<int> allChips;
  vector<string> allLadders;
  for (int iLadder = 1; iLadder <= 8; ++iLadder) {
      allLadders.push_back(string("pixel/hitmaps/station_0/layer_1/ladder_") + (iLadder < 10 ? "0" : "") + to_string(iLadder));
  }
  for (int iLadder = 1; iLadder <= 10; ++iLadder) {
      allLadders.push_back(string("pixel/hitmaps/station_0/layer_2/ladder_") + (iLadder < 10 ? "0" : "") + to_string(iLadder));
  }
    

  vector<string> allLaddersTiming = allLadders;
  for (auto& ladder : allLaddersTiming) {
    replaceAll(ladder, "hitmaps", "timing");
  }

  gROOT->Clear();  gROOT->DeleteAll();

  if (sRuns == "nada") {
    cout << "movieMakerVtx::main() sRuns = " << sRuns << " not supported" << endl;
    return 1;
  }
  vector<string> vRuns;
  vRuns = split(sRuns, ',');


  TCanvas *c1 = new TCanvas("c1", "c1", 1000, 1000);
  // -- loop over all files
  for (auto run: vRuns) {
    string filename = dir + Form("/merged-dqm_histos_0%s.root", run.c_str());
    cout << "movieMakerVtx::main() filename = " << filename << endl;
    TFile *f = TFile::Open(filename.c_str());
    if (!f) {
      cout << "movieMakerVtx::main() failed to open file " << filename << endl;
      continue;
    }

    if (mode == "all" || mode == "hitmap") {
      gStyle->SetOptStat(0); 
      string name1("hitmap_perChip_");
      // -- read in all hitmaps
      for (auto sLadder : allLadders) {
        gFile->cd(sLadder.c_str());
        TIter next(gDirectory->GetListOfKeys());
        TKey *key(0);
        while (key = (TKey*)next()) {
          string kname = key->GetName();
          if (kname.find(name1) != string::npos) {
            shrinkPad(0.1, 0.17, 0.17);
            TH2 *h = (TH2*)key->ReadObj();
            string hname(h->GetName());
            replaceAll(hname, "hitmap_perChip_", "");
            int ichip = ::stoi(hname);  
            cout << "hitmap chip " << ichip << " " << hname << " run " << run << endl;
            h->SetTitle(Form("Chip %d (0x%x) run %s", ichip, ichip, run.c_str()));
            gPad->SetLogy(0);
            h->Draw("colz");
            TString s = Form("pdf/hitmap%d.gif+50", ichip);
            c1->Print(s);  
            c1->SaveAs(Form("pdf/hitmap%d-%s.pdf", ichip, run.c_str()));
          }
        }
      }
    }

    if (mode == "all" || mode == "hitToT") {
      gStyle->SetOptStat(11111);
      // -- read in all hitmaps
      string name2("hitToT_perChip_");
      for (auto sLadder : allLaddersTiming) {
        gFile->cd(sLadder.c_str());
        TIter next(gDirectory->GetListOfKeys());
        TKey *key(0);
        while (key = (TKey*)next()) {
          string kname = key->GetName();
          if (kname.find(name2) != string::npos) {
            TH2 *h = (TH2*)key->ReadObj();
            string hname(h->GetName());
            replaceAll(hname, "hitToT_perChip_", "");
            int ichip = ::stoi(hname);  
            cout << "hitToT chip " << ichip << " " << hname << " run " << run << endl;
            h->SetTitle(Form("Chip %d (0x%x) run %s", ichip, ichip, run.c_str()));
            h->SetMinimum(0.5);
            gPad->SetLogy(1);
            h->Draw("colz");
            TString s = Form("pdf/hitToT%d.gif+50", ichip);
            c1->Print(s);  
            c1->SaveAs(Form("pdf/hitToT%d-%s.pdf", ichip, run.c_str()));
          }
        }
      }
    }
    f->Close();
  }





}
