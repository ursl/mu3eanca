#include <Rtypes.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string>
#include <vector>
#include <string.h>
#include <chrono>

#include "util.hh"

#include "TCanvas.h"
#include "TStyle.h"
#include "TFile.h"
#include "TH2F.h"
#include "TMath.h"
#include "TKey.h"
#include "TROOT.h"
#include "TLatex.h"
#include "TArrow.h"

using namespace std;


// ----------------------------------------------------------------------
// allHistograms
// ---------------
//
// Examples:
// bin/allHistograms -f /Users/ursl/mu3e/software/250429/minalyzer/root_output_files/dqm_histos_00553.root
// ----------------------------------------------------------------------


void chipIDSpecBook(int chipid, int &station, int &layer, int &phi, int &z);
void mkCombinedPDF(int run, string rof);
void mkVtxPlots(int run, string barefilename, bool noRebin);
void mkTilePlots(int run, string barefilename);
void mkFiberPlots(int run, string barefilename);
void mkDAQPlots(int run, string barefilename);
string getCurrentDateTime();





// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {

  // -- command line arguments
  int verbose(0), mode(1);
  // note: mode = 1 PixelQuality, 2 PixelQualityV, 3 PixelQualityM
  string jsondir(""), filename("nada.root");
  string gt("intrun");
  bool noRebin(false);
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-f"))      {filename = argv[++i];}
    if (!strcmp(argv[i], "-g"))      {gt = argv[++i];}
    if (!strcmp(argv[i], "-j"))      {jsondir = argv[++i];}
    if (!strcmp(argv[i], "-m"))      {mode = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-n"))      {noRebin = true;}
    if (!strcmp(argv[i], "-v"))      {verbose = atoi(argv[++i]);}
  }
  
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
  if (f && f->IsOpen()) {
  } else {
    cout << "allHistograms::main() failed to open file " << filename << endl;
    return 1;
  }
  mkVtxPlots(run, barefilename, noRebin);
  mkTilePlots(run, barefilename);
  mkFiberPlots(run, barefilename);
  mkDAQPlots(run, barefilename);

  mkCombinedPDF(run, filename);
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


// ----------------------------------------------------------------------
void mkCombinedPDF(int run, string rof) {
  ifstream ifs("template.tex");
  string line;
  vector<string> vLines;
  string date = getCurrentDateTime();

  while (getline(ifs, line)) {
    replaceAll(line, "RUNNUMBER", to_string(run));
    replaceAll(line, "ROF", rof);
    replaceAll(line, "DATE", date);
    replaceAll(line, "_", "\\_");
    vLines.push_back(line);
  }
  ifs.close();

  ofstream ofs("summary-" + to_string(run) + ".tex");
  for (auto sLine : vLines) {
    ofs << sLine << endl;
  }
  ofs.close();

  system(("pdflatex summary-" + to_string(run) + ".tex").c_str());
  system(("mv summary-" + to_string(run) + ".pdf tex/").c_str());
  system(("mv summary-" + to_string(run) + ".tex tex/").c_str());
  system(("rm summary-" + to_string(run) + ".aux").c_str());
  system(("rm summary-" + to_string(run) + ".log").c_str());
}



// ----------------------------------------------------------------------
string getCurrentDateTime() {
  auto now = chrono::system_clock::now();
  auto time_t = chrono::system_clock::to_time_t(now);
  auto tm = *localtime(&time_t);
  
  ostringstream oss;
  oss << put_time(&tm, "%Y-%m-%d %H:%M:%S");
  return oss.str();
}


// ----------------------------------------------------------------------
void mkVtxPlots(int run, string barefilename, bool noRebin) {
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
      // cout << "layer 1 chip " << ichip << " station " << station << " layer " << layer << " phi " << phi << " z " << z << endl;
    }
    for (auto ichip: vLayer2) {
      chipIDSpecBook(ichip, station, layer, phi, z);
      // cout << "layer 2 chip " << ichip << " station " << station << " layer " << layer << " phi " << phi << " z " << z << endl;
    }

    map<unsigned int, vector<double> > mdet{};
    
    vector<string> allLadders;
    for (int iLadder = 1; iLadder <= 8; ++iLadder) {
      allLadders.push_back(string("pixel/hitmaps/station_0/layer_1/ladder_") + (iLadder < 10 ? "0" : "") + to_string(iLadder));
    }
    for (int iLadder = 1; iLadder <= 10; ++iLadder) {
      allLadders.push_back(string("pixel/hitmaps/station_0/layer_2/ladder_") + (iLadder < 10 ? "0" : "") + to_string(iLadder));
    }
    
    map<int, TH2*> mHitmaps;
    string name1("hitmap_perChip_");
    // -- read in all hitmaps
    for (auto sLadder : allLadders) {
      gFile->cd(sLadder.c_str());
      TIter next(gDirectory->GetListOfKeys());
      TKey *key(0);
      while (key = (TKey*)next()) {
        string kname = key->GetName();
        if (kname.find(name1) != string::npos) {
          TH2 *h = (TH2*)key->ReadObj();
          string hname(h->GetName());
          cout << "hname " << hname << endl;
          replaceAll(hname, "hitmap_perChip_", "");
          int ichip = ::stoi(hname);  
          cout << "hitmap chip " << ichip << " " << hname << endl;
          if (!noRebin) h->Rebin2D(4,10);
          mHitmaps[ichip] = h;
          mHitmaps[ichip]->SetTitle(Form("Chip %d (0x%x)", ichip, ichip));
          mHitmaps[ichip]->SetTitleSize(0.2);
        }
      }
    }

    // -- read in all tots
    map<int, TH1*> mToTs;
    string name2("hitToT_perChip_");
    // Replace "hitmap" with "hitToA" in all ladder paths
    for (auto& ladder : allLadders) {
      replaceAll(ladder, "hitmaps", "timing");
    }

    for (auto sLadder : allLadders) {
      gFile->cd(sLadder.c_str());
      TIter next(gDirectory->GetListOfKeys());
      TKey *key(0);
      while (key = (TKey*)next()) {
        string kname = key->GetName();
        if (kname.find(name2) != string::npos) {
          TH1 *h = (TH1*)key->ReadObj();
          string hname(h->GetName());
          replaceAll(hname, "hitToT_perChip_", "");
          int ichip = ::stoi(hname);  
          // cout << "toa chip " << ichip << " " << hname << endl;
          mToTs[ichip] = h;
          mToTs[ichip]->SetTitle(Form("Chip %d (0x%x)", ichip, ichip));
          mToTs[ichip]->SetTitleSize(0.2);
        }
      }
    }

    // -- read in all relevant time correlations
    map<string, TH2*> mTimeCorrelations;
    vector<string> allTimeCorrelations;
    allTimeCorrelations = {
      "localTimeL1TopVsL1Bot",
      "localTimeL1TopVsL2Bot",
      "localTimeL1TopVsL2Top",
      "localTimeL2TopVsL2Bot",
      "localTimeL1BotVsL2Bot",
      "localTimeL1BotVsL2Top",
      "localTimeL1Lad1VsL2Lad1",
      "localTimeL1Lad2VsL2Lad23",
      "localTimeL1Lad3VsL2Lad34",
      "localTimeL1Lad4VsL2Lad5",
      "localTimeL1Lad5VsL2Lad6",
      "localTimeL1Lad6VsL2Lad78",
      "localTimeL1Lad7VsL2Lad89",
      "localTimeL1Lad8VsL2Lad10"
    };
    TDirectory *dir = gFile->GetDirectory("timecorrelations");
    cout << "GetDirectory: " << dir << endl;
    if (nullptr != dir) {
      cout << "reading time correlations from timecorrelations/pixel" << endl;
      dir->cd("pixel");
      TIter next(gDirectory->GetListOfKeys());
      TKey *key(0);
      while (key = (TKey*)next()) {
        string kname = key->GetName();
        if (find(allTimeCorrelations.begin(), allTimeCorrelations.end(), kname) != allTimeCorrelations.end()) {
          TH2 *h = (TH2*)key->ReadObj();
          // cout << "time correlation " << kname << endl;
          h->Rebin2D(64,64);
          mTimeCorrelations[kname] = h;
        }
      }
    } else{
      cout << "timecorrelations/pixel not found" << endl;
    }


    // -- read in the LVDS links -- this is generated with lvdsVsGlobalChipID!
    map<int, vector<string>> mLinksChipID = {
      {1, {"lvds/FEB_00/Link_00", "lvds/FEB_00/Link_01", "lvds/FEB_00/Link_02"}},
      {2, {"lvds/FEB_00/Link_03", "lvds/FEB_00/Link_04", "lvds/FEB_00/Link_05"}},
      {3, {"lvds/FEB_00/Link_06", "lvds/FEB_00/Link_07", "lvds/FEB_00/Link_08"}},
      {4, {"lvds/FEB_05/Link_06", "lvds/FEB_05/Link_07", "lvds/FEB_05/Link_08"}},
      {5, {"lvds/FEB_05/Link_03", "lvds/FEB_05/Link_04", "lvds/FEB_05/Link_05"}},
      {6, {"lvds/FEB_05/Link_00", "lvds/FEB_05/Link_01", "lvds/FEB_05/Link_02"}},
      {33, {"lvds/FEB_00/Link_09", "lvds/FEB_00/Link_10", "lvds/FEB_00/Link_11"}},
      {34, {"lvds/FEB_00/Link_12", "lvds/FEB_00/Link_13", "lvds/FEB_00/Link_14"}},
      {35, {"lvds/FEB_00/Link_15", "lvds/FEB_00/Link_16", "lvds/FEB_00/Link_17"}},
      {36, {"lvds/FEB_05/Link_15", "lvds/FEB_05/Link_16", "lvds/FEB_05/Link_17"}},
      {37, {"lvds/FEB_05/Link_12", "lvds/FEB_05/Link_13", "lvds/FEB_05/Link_14"}},
      {38, {"lvds/FEB_05/Link_09", "lvds/FEB_05/Link_10", "lvds/FEB_05/Link_11"}},
      {65, {"lvds/FEB_00/Link_18", "lvds/FEB_00/Link_19", "lvds/FEB_00/Link_20"}},
      {66, {"lvds/FEB_00/Link_21", "lvds/FEB_00/Link_22", "lvds/FEB_00/Link_23"}},
      {67, {"lvds/FEB_00/Link_24", "lvds/FEB_00/Link_25", "lvds/FEB_00/Link_26"}},
      {68, {"lvds/FEB_05/Link_24", "lvds/FEB_05/Link_25", "lvds/FEB_05/Link_26"}},
      {69, {"lvds/FEB_05/Link_21", "lvds/FEB_05/Link_22", "lvds/FEB_05/Link_23"}},
      {70, {"lvds/FEB_05/Link_18", "lvds/FEB_05/Link_19", "lvds/FEB_05/Link_20"}},
      {97, {"lvds/FEB_00/Link_27", "lvds/FEB_00/Link_28", "lvds/FEB_00/Link_29"}},
      {98, {"lvds/FEB_00/Link_30", "lvds/FEB_00/Link_31", "lvds/FEB_00/Link_32"}},
      {99, {"lvds/FEB_00/Link_33", "lvds/FEB_00/Link_34", "lvds/FEB_00/Link_35"}},
      {100, {"lvds/FEB_05/Link_33", "lvds/FEB_05/Link_34", "lvds/FEB_05/Link_35"}},
      {101, {"lvds/FEB_05/Link_30", "lvds/FEB_05/Link_31", "lvds/FEB_05/Link_32"}},
      {102, {"lvds/FEB_05/Link_27", "lvds/FEB_05/Link_28", "lvds/FEB_05/Link_29"}},
      {129, {"lvds/FEB_01/Link_00", "lvds/FEB_01/Link_01", "lvds/FEB_01/Link_02"}},
      {130, {"lvds/FEB_01/Link_03", "lvds/FEB_01/Link_04", "lvds/FEB_01/Link_05"}},
      {131, {"lvds/FEB_01/Link_06", "lvds/FEB_01/Link_07", "lvds/FEB_01/Link_08"}},
      {132, {"lvds/FEB_06/Link_06", "lvds/FEB_06/Link_07", "lvds/FEB_06/Link_08"}},
      {133, {"lvds/FEB_06/Link_03", "lvds/FEB_06/Link_04", "lvds/FEB_06/Link_05"}},
      {134, {"lvds/FEB_06/Link_00", "lvds/FEB_06/Link_01", "lvds/FEB_06/Link_02"}},
      {161, {"lvds/FEB_01/Link_09", "lvds/FEB_01/Link_10", "lvds/FEB_01/Link_11"}},
      {162, {"lvds/FEB_01/Link_12", "lvds/FEB_01/Link_13", "lvds/FEB_01/Link_14"}},
      {163, {"lvds/FEB_01/Link_15", "lvds/FEB_01/Link_16", "lvds/FEB_01/Link_17"}},
      {164, {"lvds/FEB_06/Link_15", "lvds/FEB_06/Link_16", "lvds/FEB_06/Link_17"}},
      {165, {"lvds/FEB_06/Link_12", "lvds/FEB_06/Link_13", "lvds/FEB_06/Link_14"}},
      {166, {"lvds/FEB_06/Link_09", "lvds/FEB_06/Link_10", "lvds/FEB_06/Link_11"}},
      {193, {"lvds/FEB_01/Link_18", "lvds/FEB_01/Link_19", "lvds/FEB_01/Link_20"}},
      {194, {"lvds/FEB_01/Link_21", "lvds/FEB_01/Link_22", "lvds/FEB_01/Link_23"}},
      {195, {"lvds/FEB_01/Link_24", "lvds/FEB_01/Link_25", "lvds/FEB_01/Link_26"}},
      {196, {"lvds/FEB_06/Link_24", "lvds/FEB_06/Link_25", "lvds/FEB_06/Link_26"}},
      {197, {"lvds/FEB_06/Link_21", "lvds/FEB_06/Link_22", "lvds/FEB_06/Link_23"}},
      {198, {"lvds/FEB_06/Link_18", "lvds/FEB_06/Link_19", "lvds/FEB_06/Link_20"}},
      {225, {"lvds/FEB_01/Link_27", "lvds/FEB_01/Link_28", "lvds/FEB_01/Link_29"}},
      {226, {"lvds/FEB_01/Link_30", "lvds/FEB_01/Link_31", "lvds/FEB_01/Link_32"}},
      {227, {"lvds/FEB_01/Link_33", "lvds/FEB_01/Link_34", "lvds/FEB_01/Link_35"}},
      {228, {"lvds/FEB_06/Link_33", "lvds/FEB_06/Link_34", "lvds/FEB_06/Link_35"}},
      {229, {"lvds/FEB_06/Link_30", "lvds/FEB_06/Link_31", "lvds/FEB_06/Link_32"}},
      {230, {"lvds/FEB_06/Link_27", "lvds/FEB_06/Link_28", "lvds/FEB_06/Link_29"}},
      {1025, {"lvds/FEB_02/Link_00", "lvds/FEB_02/Link_01", "lvds/FEB_02/Link_02"}},
      {1026, {"lvds/FEB_02/Link_03", "lvds/FEB_02/Link_04", "lvds/FEB_02/Link_05"}},
      {1027, {"lvds/FEB_02/Link_06", "lvds/FEB_02/Link_07", "lvds/FEB_02/Link_08"}},
      {1028, {"lvds/FEB_07/Link_06", "lvds/FEB_07/Link_07", "lvds/FEB_07/Link_08"}},
      {1029, {"lvds/FEB_07/Link_03", "lvds/FEB_07/Link_04", "lvds/FEB_07/Link_05"}},
      {1030, {"lvds/FEB_07/Link_00", "lvds/FEB_07/Link_01", "lvds/FEB_07/Link_02"}},
      {1057, {"lvds/FEB_02/Link_09", "lvds/FEB_02/Link_10", "lvds/FEB_02/Link_11"}},
      {1058, {"lvds/FEB_02/Link_12", "lvds/FEB_02/Link_13", "lvds/FEB_02/Link_14"}},
      {1059, {"lvds/FEB_02/Link_15", "lvds/FEB_02/Link_16", "lvds/FEB_02/Link_17"}},
      {1060, {"lvds/FEB_07/Link_15", "lvds/FEB_07/Link_16", "lvds/FEB_07/Link_17"}},
      {1061, {"lvds/FEB_07/Link_12", "lvds/FEB_07/Link_13", "lvds/FEB_07/Link_14"}},
      {1062, {"lvds/FEB_07/Link_09", "lvds/FEB_07/Link_10", "lvds/FEB_07/Link_11"}},
      {1089, {"lvds/FEB_02/Link_18", "lvds/FEB_02/Link_19", "lvds/FEB_02/Link_20"}},
      {1090, {"lvds/FEB_02/Link_21", "lvds/FEB_02/Link_22", "lvds/FEB_02/Link_23"}},
      {1091, {"lvds/FEB_02/Link_24", "lvds/FEB_02/Link_25", "lvds/FEB_02/Link_26"}},
      {1092, {"lvds/FEB_07/Link_24", "lvds/FEB_07/Link_25", "lvds/FEB_07/Link_26"}},
      {1093, {"lvds/FEB_07/Link_21", "lvds/FEB_07/Link_22", "lvds/FEB_07/Link_23"}},
      {1094, {"lvds/FEB_07/Link_18", "lvds/FEB_07/Link_19", "lvds/FEB_07/Link_20"}},
      {1121, {"lvds/FEB_03/Link_00", "lvds/FEB_03/Link_01", "lvds/FEB_03/Link_02"}},
      {1122, {"lvds/FEB_03/Link_03", "lvds/FEB_03/Link_04", "lvds/FEB_03/Link_05"}},
      {1123, {"lvds/FEB_03/Link_06", "lvds/FEB_03/Link_07", "lvds/FEB_03/Link_08"}},
      {1124, {"lvds/FEB_08/Link_06", "lvds/FEB_08/Link_07", "lvds/FEB_08/Link_08"}},
      {1125, {"lvds/FEB_08/Link_03", "lvds/FEB_08/Link_04", "lvds/FEB_08/Link_05"}},
      {1126, {"lvds/FEB_08/Link_00", "lvds/FEB_08/Link_01", "lvds/FEB_08/Link_02"}},
      {1153, {"lvds/FEB_03/Link_09", "lvds/FEB_03/Link_10", "lvds/FEB_03/Link_11"}},
      {1154, {"lvds/FEB_03/Link_12", "lvds/FEB_03/Link_13", "lvds/FEB_03/Link_14"}},
      {1155, {"lvds/FEB_03/Link_15", "lvds/FEB_03/Link_16", "lvds/FEB_03/Link_17"}},
      {1156, {"lvds/FEB_08/Link_15", "lvds/FEB_08/Link_16", "lvds/FEB_08/Link_17"}},
      {1157, {"lvds/FEB_08/Link_12", "lvds/FEB_08/Link_13", "lvds/FEB_08/Link_14"}},
      {1158, {"lvds/FEB_08/Link_09", "lvds/FEB_08/Link_10", "lvds/FEB_08/Link_11"}},
      {1185, {"lvds/FEB_03/Link_18", "lvds/FEB_03/Link_19", "lvds/FEB_03/Link_20"}},
      {1186, {"lvds/FEB_03/Link_21", "lvds/FEB_03/Link_22", "lvds/FEB_03/Link_23"}},
      {1187, {"lvds/FEB_03/Link_24", "lvds/FEB_03/Link_25", "lvds/FEB_03/Link_26"}},
      {1188, {"lvds/FEB_08/Link_24", "lvds/FEB_08/Link_25", "lvds/FEB_08/Link_26"}},
      {1189, {"lvds/FEB_08/Link_21", "lvds/FEB_08/Link_22", "lvds/FEB_08/Link_23"}},
      {1190, {"lvds/FEB_08/Link_18", "lvds/FEB_08/Link_19", "lvds/FEB_08/Link_20"}},
      {1217, {"lvds/FEB_03/Link_27", "lvds/FEB_03/Link_28", "lvds/FEB_03/Link_29"}},
      {1218, {"lvds/FEB_03/Link_30", "lvds/FEB_03/Link_31", "lvds/FEB_03/Link_32"}},
      {1219, {"lvds/FEB_03/Link_33", "lvds/FEB_03/Link_34", "lvds/FEB_03/Link_35"}},
      {1220, {"lvds/FEB_08/Link_33", "lvds/FEB_08/Link_34", "lvds/FEB_08/Link_35"}},
      {1221, {"lvds/FEB_08/Link_30", "lvds/FEB_08/Link_31", "lvds/FEB_08/Link_32"}},
      {1222, {"lvds/FEB_08/Link_27", "lvds/FEB_08/Link_28", "lvds/FEB_08/Link_29"}},
      {1249, {"lvds/FEB_04/Link_00", "lvds/FEB_04/Link_01", "lvds/FEB_04/Link_02"}},
      {1250, {"lvds/FEB_04/Link_03", "lvds/FEB_04/Link_04", "lvds/FEB_04/Link_05"}},
      {1251, {"lvds/FEB_04/Link_06", "lvds/FEB_04/Link_07", "lvds/FEB_04/Link_08"}},
      {1252, {"lvds/FEB_09/Link_06", "lvds/FEB_09/Link_07", "lvds/FEB_09/Link_08"}},
      {1253, {"lvds/FEB_09/Link_03", "lvds/FEB_09/Link_04", "lvds/FEB_09/Link_05"}},
      {1254, {"lvds/FEB_09/Link_00", "lvds/FEB_09/Link_01", "lvds/FEB_09/Link_02"}},
      {1281, {"lvds/FEB_04/Link_09", "lvds/FEB_04/Link_10", "lvds/FEB_04/Link_11"}},
      {1282, {"lvds/FEB_04/Link_12", "lvds/FEB_04/Link_13", "lvds/FEB_04/Link_14"}},
      {1283, {"lvds/FEB_04/Link_15", "lvds/FEB_04/Link_16", "lvds/FEB_04/Link_17"}},
      {1284, {"lvds/FEB_09/Link_15", "lvds/FEB_09/Link_16", "lvds/FEB_09/Link_17"}},
      {1285, {"lvds/FEB_09/Link_12", "lvds/FEB_09/Link_13", "lvds/FEB_09/Link_14"}},
      {1286, {"lvds/FEB_09/Link_09", "lvds/FEB_09/Link_10", "lvds/FEB_09/Link_11"}},
      {1313, {"lvds/FEB_04/Link_18", "lvds/FEB_04/Link_19", "lvds/FEB_04/Link_20"}},
      {1314, {"lvds/FEB_04/Link_21", "lvds/FEB_04/Link_22", "lvds/FEB_04/Link_23"}},
      {1315, {"lvds/FEB_04/Link_24", "lvds/FEB_04/Link_25", "lvds/FEB_04/Link_26"}},
      {1316, {"lvds/FEB_09/Link_24", "lvds/FEB_09/Link_25", "lvds/FEB_09/Link_26"}},
      {1317, {"lvds/FEB_09/Link_21", "lvds/FEB_09/Link_22", "lvds/FEB_09/Link_23"}},
      {1318, {"lvds/FEB_09/Link_18", "lvds/FEB_09/Link_19", "lvds/FEB_09/Link_20"}}
    };
    cout << "hallo: " << gFile << endl;
    gFile->cd();
    map<int, vector<TH1*>> mLVDSErrors;
    for (auto itChipLinks: mLinksChipID) {
      for (unsigned int iLink = 0; iLink < itChipLinks.second.size(); ++iLink) {
        string itLink = itChipLinks.second[iLink] + "/" + "8b10bErrorsVsMidasEvent";
        string sidx = to_string(itChipLinks.first);
        cout << "itLink: " << itLink << endl;
        TH1D *h = (TH1D*)gFile->Get(itLink.c_str());
        if (iLink == 0) {      
          h->SetLineColor(kBlack);
        }
        if (iLink == 1) {
          h->SetLineColor(kBlue);
        }
        if (iLink == 2) {
          h->SetLineColor(kRed);
        }
        h->SetMinimum(0.5);
        h->SetTitle(sidx.c_str());
        mLVDSErrors[itChipLinks.first].push_back(h);
      }
    }


    // -----------------------
    // -- PLOTTING starts here
    // -----------------------


    // -- hitmaps
    TCanvas *c = new TCanvas("c", "c", 800, 1000);
    gStyle->SetOptStat(0);
    gStyle->SetPadBorderMode(0);
    gStyle->SetPadBorderSize(0);
    gStyle->SetPadTopMargin(0);
    gStyle->SetPadBottomMargin(0);
    gStyle->SetPadLeftMargin(0);
    gStyle->SetPadRightMargin(0);
    gStyle->SetTitleSize(0.3);
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
      gPad->SetLogz(1);
      gPad->SetBottomMargin(0.0);
      gPad->SetLeftMargin(0.0);
      gPad->SetRightMargin(0.0);
      gPad->SetTopMargin(0.0);
      // cout << "vLayer1[i] = " << vLayer1[i] << " " << mHitmaps[vLayer1[i]] << endl;
      if (mHitmaps[vLayer1[i]]) {
        TH2F *h2 = (TH2F*)mHitmaps[vLayer1[i]]->Clone();
        //h2->Rebin2D(4,10);
        h2->Draw("col");
        mHitmaps[vLayer1[i]]->SetTitleSize(0.3);
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
      gPad->SetLogz(1);
      gPad->SetBottomMargin(0.0);
      gPad->SetLeftMargin(0.0);
      gPad->SetRightMargin(0.0);
      gPad->SetTopMargin(0.0);
      // cout << "vLayer2[i] = " << vLayer2[i] << " " << mHitmaps[vLayer2[i]] << endl;
      if (mHitmaps[vLayer2[i]]) {
        TH2F *h2 = (TH2F*)mHitmaps[vLayer2[i]]->Clone();
        //h2->Rebin2D(4,10);
        h2->Draw("col");
      }
    }
    replaceAll(barefilename, ".root", "");
    c->SaveAs(("out/vtxHitmaps-" + to_string(run) + ".pdf").c_str());
    delete c;

    // -- toTs
    c = new TCanvas("c", "c", 800, 1000);
    c->Divide(2, 1);
    c->cd(1);
    gPad->SetBottomMargin(0.0);
    gPad->SetLeftMargin(0.0);
    gPad->SetRightMargin(0.0);
    gPad->SetTopMargin(0.0);
    p = (TPad*)c->GetPad(1);
    p->Divide(6,8);
    for (int i = 0; i < vLayer1.size(); ++i) {    
      p->cd(i+1);
      gPad->SetBottomMargin(0.0);
      gPad->SetLeftMargin(0.0);
      gPad->SetRightMargin(0.0);
      gPad->SetTopMargin(0.0);
      gPad->SetLogy(1);
      // cout << "vLayer1[i] = " << vLayer1[i] << " " << mToTs[vLayer1[i]] << endl;
      if (mToTs[vLayer1[i]]) {
        mToTs[vLayer1[i]]->SetMinimum(0.5);
        mToTs[vLayer1[i]]->Draw();
      }
    }
    c->cd(2);
    p = (TPad*)c->GetPad(2);
    p->Divide(6,10);
    for (int i = 0; i < vLayer2.size(); ++i) {    
      p->cd(i+1);
      gPad->SetBottomMargin(0.0);
      gPad->SetLeftMargin(0.0);
      gPad->SetRightMargin(0.0);
      gPad->SetTopMargin(0.0);
      gPad->SetLogy(1);
      // cout << "vLayer2[i] = " << vLayer2[i] << " " << mToTs[vLayer2[i]] << endl;
      if (mToTs[vLayer2[i]]) {
        mToTs[vLayer2[i]]->SetMinimum(0.5);
        mToTs[vLayer2[i]]->Draw();
      }
    }
    replaceAll(barefilename, ".root", "");
    c->SaveAs(("out/vtxHitToTs-" + to_string(run) + ".pdf").c_str());
    delete c;

    // -- LVDS errors
    c = new TCanvas("c", "c", 800, 1000);
    c->Divide(2, 1);
    c->cd(1);
    gPad->SetBottomMargin(0.0);
    gPad->SetLeftMargin(0.0);
    gPad->SetRightMargin(0.0);
    gPad->SetTopMargin(0.0);
    p = (TPad*)c->GetPad(1);
    TLatex *tl0 = new TLatex();
    tl0->SetTextSize(0.048);
    p->Divide(6,8);
    for (int i = 0; i < vLayer1.size(); ++i) {    
      p->cd(i+1);
      gPad->SetBottomMargin(0.0);
      gPad->SetLeftMargin(0.0);
      gPad->SetRightMargin(0.0);
      gPad->SetTopMargin(0.0);
      gPad->SetLogy(1);
      mLVDSErrors[vLayer1[i]][0]->Draw("");
      mLVDSErrors[vLayer1[i]][1]->Draw("same");
      mLVDSErrors[vLayer1[i]][2]->Draw("same");
      tl0->SetTextColor(kBlack); tl0->DrawLatexNDC(0.75, 0.93, "A");
      tl0->SetTextColor(kBlue);  tl0->DrawLatexNDC(0.80, 0.93, "B");
      tl0->SetTextColor(kRed);   tl0->DrawLatexNDC(0.85, 0.93, "C");
    }
    c->cd(2);
    p = (TPad*)c->GetPad(2);
    p->Divide(6,10);
    for (int i = 0; i < vLayer2.size(); ++i) {    
      p->cd(i+1);
      gPad->SetBottomMargin(0.0);
      gPad->SetLeftMargin(0.0);
      gPad->SetRightMargin(0.0);
      gPad->SetTopMargin(0.0);
      gPad->SetLogy(1);
      mLVDSErrors[vLayer2[i]][0]->Draw("");
      mLVDSErrors[vLayer2[i]][1]->Draw("same");
      mLVDSErrors[vLayer2[i]][2]->Draw("same");
      tl0->SetTextColor(kBlack); tl0->DrawLatexNDC(0.75, 0.93, "A");
      tl0->SetTextColor(kBlue);  tl0->DrawLatexNDC(0.80, 0.93, "B");
      tl0->SetTextColor(kRed);   tl0->DrawLatexNDC(0.85, 0.93, "C");
    }
    replaceAll(barefilename, ".root", "");
    c->SaveAs(("out/vtxLVDSErrors-" + to_string(run) + ".pdf").c_str());
    delete c;

    // -- time correlations
    if (mTimeCorrelations.size() > 0) {
      c = new TCanvas("c", "c", 800, 1000);
      c->Divide(4,4);
      int i(1);
      // -- first plot for "global" correlations
      for (auto sTimeCorrelation : mTimeCorrelations) {
        if (sTimeCorrelation.first.find("Lad") != string::npos) continue;
        c->cd(i);
        gPad->SetLogz(1);
        sTimeCorrelation.second->Draw("col");
        i++;  
      }
      // -- now plot for "ladder" correlations
      i += 2;
      for (auto sTimeCorrelation : mTimeCorrelations) {
        if (sTimeCorrelation.first.find("Lad") == string::npos) continue;
        c->cd(i);
        gPad->SetLogz(1);
        sTimeCorrelation.second->Draw("col");
        i++;  
      }
      replaceAll(barefilename, ".root", "");
      c->SaveAs(("out/vtxTimeCorrelations-" + to_string(run) + ".pdf").c_str());
      delete c;
    }

  // -- now plot 1D single pixel hit rate 
  c = new TCanvas("c", "c", 800, 1000);
  gStyle->SetOptStat(0);
  gStyle->SetPadTopMargin(0);
  gStyle->SetPadBottomMargin(0);
  gStyle->SetPadLeftMargin(0);
  gStyle->SetPadRightMargin(0);
  gStyle->SetTitleSize(0.3);
  c->Divide(2, 1);
  c->cd(1);
  gPad->SetBottomMargin(0.0);
  gPad->SetLeftMargin(0.0);
  gPad->SetRightMargin(0.0);
  gPad->SetTopMargin(0.0);
  p = (TPad*)c->GetPad(1);
  p->Divide(6,8);
  TH1 *h1; 
  TLatex *tl = new TLatex();
  tl->SetTextSize(0.1);
  tl->SetTextColor(kBlack);

  TArrow *ar = new TArrow();
  ar->SetLineWidth(1);
  ar->SetAngle(50);
  ar->SetArrowSize(0.02);
  ar->SetLineColor(kRed);
  ar->SetFillStyle(0);
  int NSIGMA = 10;
  int NBINS = 200;
  double xmax = 2000;

  for (int i = 0; i < vLayer1.size(); ++i) {    
    p->cd(i+1);
    gPad->SetLogy(1);
    gPad->SetBottomMargin(0.0);
    gPad->SetLeftMargin(0.0);
    gPad->SetRightMargin(0.0);
    gPad->SetTopMargin(0.0);
    // cout << "vLayer1[i] = " << vLayer1[i] << " " << mHitmaps[vLayer1[i]] << endl;
    if (mHitmaps[vLayer1[i]]) {
      h1 = new TH1F(Form("h1_%s", mHitmaps[vLayer1[i]]->GetName()), Form("h1_%s", mHitmaps[vLayer1[i]]->GetName()), NBINS, 0, xmax);
      h1->SetTitle("");
      h1->SetMinimum(0.5);
      for (int ix = 1; ix <= mHitmaps[vLayer1[i]]->GetNbinsX(); ++ix) {
        for (int iy = 1; iy <= mHitmaps[vLayer1[i]]->GetNbinsY(); ++iy) {
          if (mHitmaps[vLayer1[i]]->GetBinContent(ix, iy) > 0)  h1->Fill(mHitmaps[vLayer1[i]]->GetBinContent(ix, iy));
        }
      }
      h1->Draw("hist");
      h1->SetTitleSize(0.3);
      tl->DrawLatexNDC(0.2, 0.9, Form("%s", mHitmaps[vLayer1[i]]->GetTitle()));
      int oflw = static_cast<int>(h1->GetBinContent(h1->GetNbinsX()+1));
      if (oflw > 0) {
        tl->SetTextColor(kRed);
      } else {
        tl->SetTextColor(kBlack);
      }
      tl->DrawLatexNDC(0.2, 0.82, Form("overflow: %d/%d", oflw, static_cast<int>(h1->GetEntries())));
      tl->SetTextColor(kBlack);
      tl->DrawLatexNDC(0.2, 0.74, Form("mean: %5.2f#pm%5.2f", h1->GetMean(), h1->GetMeanError()));
      tl->DrawLatexNDC(0.2, 0.66, Form("Thr_{N}: %5.2f", h1->GetMean()+NSIGMA * h1->GetMeanError()));
      double x = h1->GetMean() + NSIGMA * h1->GetMeanError();
      if (h1->GetEntries() > 0) ar->DrawArrow(x, h1->GetMaximum(), x, 0.6);
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
    gPad->SetLogy(1);
    gPad->SetBottomMargin(0.0);
    gPad->SetLeftMargin(0.0);
    gPad->SetRightMargin(0.0);
    gPad->SetTopMargin(0.0);
    // cout << "vLayer2[i] = " << vLayer2[i] << " " << mHitmaps[vLayer2[i]] << endl;
    if (mHitmaps[vLayer2[i]]) {
      h1 = new TH1F(Form("h1_%s", mHitmaps[vLayer2[i]]->GetName()), Form("h1_%s", mHitmaps[vLayer2[i]]->GetName()), NBINS, 0, xmax);
      h1->SetTitle("");
      h1->SetMinimum(0.5);
      for (int ix = 1; ix <= mHitmaps[vLayer2[i]]->GetNbinsX(); ++ix) {
        for (int iy = 1; iy <= mHitmaps[vLayer2[i]]->GetNbinsY(); ++iy) {
          if (mHitmaps[vLayer2[i]]->GetBinContent(ix, iy) > 0) h1->Fill(mHitmaps[vLayer2[i]]->GetBinContent(ix, iy));
        }
      }
      h1->Draw("hist");
      tl->DrawLatexNDC(0.2, 0.9, Form("%s", mHitmaps[vLayer2[i]]->GetTitle()));
      int oflw = static_cast<int>(h1->GetBinContent(h1->GetNbinsX()+1));
      if (oflw > 0) {
        tl->SetTextColor(kRed);
      } else {
        tl->SetTextColor(kBlack);
      }
      tl->DrawLatexNDC(0.2, 0.82, Form("overflow: %d/%d", oflw, static_cast<int>(h1->GetEntries())));
      tl->SetTextColor(kBlack);
      tl->DrawLatexNDC(0.2, 0.74, Form("mean: %5.2f#pm%5.2f", h1->GetMean(), h1->GetMeanError()));
      tl->DrawLatexNDC(0.2, 0.66, Form("Thr_{N}: %5.2f", h1->GetMean()+NSIGMA * h1->GetMeanError()));
      double x = h1->GetMean() + NSIGMA * h1->GetMeanError();
      if (h1->GetEntries() > 0) ar->DrawArrow(x, h1->GetMaximum(), x, 0.6);
    }
  }
  replaceAll(barefilename, ".root", "");
  c->SaveAs(("out/vtxHitCounts-" + to_string(run) + ".pdf").c_str());

}



// ----------------------------------------------------------------------
void mkTilePlots(int run, string barefilename) {
  TCanvas *c = new TCanvas("c", "c", 800, 1000);
  gStyle->SetPadBorderMode(1);
  gStyle->SetPadBorderSize(1);
  gStyle->SetPadTopMargin(0.1);
  gStyle->SetPadBottomMargin(0.1);
  gStyle->SetPadLeftMargin(0.1);
  gStyle->SetPadRightMargin(0.1);
  c->cd();
  shrinkPad(0.1,0.1,0.15,0.1);
  gPad->SetLogz(1);
  gFile->cd("tile");
  TH2 *h = (TH2*)gDirectory->Get("Zphi_TileHitmap_DS");
  h->Draw("colz");
  replaceAll(barefilename, ".root", "");
  c->SaveAs(("out/tileHitmapZphi-" + to_string(run) + ".pdf").c_str());
  delete c;

  vector<int> vASICID;
  TH1 *h1 = (TH1*)gDirectory->Get("ASICID");
  for (int ibin = 1; ibin <= h1->GetNbinsX(); ++ibin) {
    if (h1->GetBinContent(ibin) > 0) {
      //  cout << "ibin " << ibin << " " << h1->GetBinContent(ibin) << endl;
      vASICID.push_back(ibin-1);
    }
  }

  c = new TCanvas("c", "c", 800, 1000);
  c->Divide(8, 5);
  int i(1);
  for (auto kname : vASICID) {
    string hname = "Energy_ASIC_" + to_string(kname);
    // cout << "kname " << kname << " " << hname << endl;
    TH1 *h = (TH1*)gDirectory->Get(hname.c_str());
    if (h) {
      h->Rebin(16);
      c->cd(i);
      gPad->SetLogy(1);
      setFilledHist(h);
      h->Draw("hist");
      i++;
    }
  }
  replaceAll(barefilename, ".root", "");
  c->SaveAs(("out/tileASICEnergy-" + to_string(run) + ".pdf").c_str());
  delete c;

}


// ----------------------------------------------------------------------
void mkFiberPlots(int run, string barefilename) {
  gFile->cd("fibre");
  gStyle->SetPadBorderMode(1);
  gStyle->SetPadBorderSize(1);
  gStyle->SetPadTopMargin(0.1);
  gStyle->SetPadBottomMargin(0.1);
  gStyle->SetPadLeftMargin(0.1);
  gStyle->SetPadRightMargin(0.1);

  TCanvas *c = new TCanvas("c", "c", 800, 400);
  c->Divide(4,2);
  TH1 *h1; 
  TH2 *h2;
  c->cd(1);
  h1 = (TH1*)gDirectory->Get("totalTS_all_50ps");
  h1->Rebin(5);
  h1->Draw("hist");
  c->cd(2);
  h1 = (TH1*)gDirectory->Get("ASICID_all");
  h1->Rebin(5);
  h1->Draw("hist");
  c->cd(3);
  h1 = (TH1*)gDirectory->Get("FEBID_all");
  h1->Rebin(5);
  h1->Draw("hist");
  c->cd(4);
  h2 = (TH2*)gDirectory->Get("channelID_TimeStampDeltaSameChannelPerChannel");
  h2->Rebin2D(5,5);
  h2->Draw("box");
  c->cd(5);
  h1->Rebin(5);
  h1 = (TH1*)gDirectory->Get("channelID_all");
  h1->Rebin(5);
  h1->Draw("hist");
  c->cd(6);
  h1 = (TH1*)gDirectory->Get("timeStampDeltaSameChannel_all");
  h1->Rebin(5);
  h1->Draw("hist");
  c->cd(7);
  h1->Rebin(5);
  h1 = (TH1*)gDirectory->Get("totalTS_all_50ps");
  h1->Draw("hist");

  c->SaveAs(("out/fibers-" + to_string(run) + ".pdf").c_str());
  delete c;


} 


// ----------------------------------------------------------------------
void mkDAQPlots(int run, string barefilename) {
  gFile->cd("/DAQfills/overflow");
  gStyle->SetPadBorderMode(1);
  gStyle->SetPadBorderSize(1);
  gStyle->SetPadTopMargin(0.1);
  gStyle->SetPadBottomMargin(0.1);
  gStyle->SetPadLeftMargin(0.1);
  gStyle->SetPadRightMargin(0.1);

  TCanvas *c = new TCanvas("c", "c", 800, 800);
  c->Divide(2,2);
  TH1 *h1; 
  TH2 *h2;
  c->cd(1);
  gPad->SetLogz(1);
  shrinkPad(0.1,0.1,0.15,0.1);
  h2 = (TH2*)gDirectory->Get("timestampWhenSorterOverflowed");
  h2->Draw("colz");

  c->cd(2);
  gPad->SetLogz(1);
  shrinkPad(0.1,0.1,0.15,0.1);
  h2 = (TH2*)gDirectory->Get("timestampWhenAllHitsDiscarded");
  h2->Draw("colz");

  c->cd(3);
  gPad->SetLogz(1);
  shrinkPad(0.1,0.1,0.15,0.1);
  h2 = (TH2*)gDirectory->Get("timestampWhenBothOverflowBitsHigh");
  h2->Draw("colz");

  c->SaveAs(("out/daqOverflowPlots-" + to_string(run) + ".pdf").c_str());

  delete c;



}
