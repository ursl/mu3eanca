#include "pixelHistograms.hh"

#include <_types/_uint32_t.h>
#include <iostream>
#include <sys/stat.h>

#include <TDirectory.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TPad.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TStyle.h>
#include <TFile.h>

using namespace std;

// ----------------------------------------------------------------------
pixelHistograms::pixelHistograms() {
  fVerbose = 0;
  fRun = 0;
  cout << "pixelHistograms::pixelHistograms() ctor" << endl;

  init(0); 

}

// ----------------------------------------------------------------------
pixelHistograms::pixelHistograms(std::string filename) {
  fFilename = filename;
  init(1);
}


// ----------------------------------------------------------------------
void pixelHistograms::init(int mode) {
  fLayer1 = {1,2,3,4,5,6,
            33, 34, 35, 36, 37, 38,
            65, 66, 67, 68, 69, 70,
            97, 98, 99, 100, 101, 102,
            129, 130, 131, 132, 133, 134,
            161, 162, 163, 164, 165, 166,
            193, 194, 195, 196, 197, 198,
            225, 226, 227, 228, 229, 230};
  fLayer2 = {1025, 1026, 1027, 1028, 1029, 1030,
            1057, 1058, 1059, 1060, 1061, 1062,
            1089, 1090, 1091, 1092, 1093, 1094,
            1121, 1122, 1123, 1124, 1125, 1126,
            1153, 1154, 1155, 1156, 1157, 1158,
            1185, 1186, 1187, 1188, 1189, 1190,
            1217, 1218, 1219, 1220, 1221, 1222,
            1249, 1250, 1251, 1252, 1253, 1254,
            1281, 1282, 1283, 1284, 1285, 1286,
            1313, 1314, 1315, 1316, 1317, 1318};

  fAllChips = fLayer1;
  fAllChips.insert(fAllChips.end(), fLayer2.begin(), fLayer2.end());

  if (0 == mode) {
    bookHist("hitmap", "chipmap");
    bookHist("rawtot", "chipToT");
    bookHist("hittot", "chipToT");

    // -- distributions for rejected pixel hits
    bookHist("rj_hm", "chipmap");
    bookHist("rj_ht", "chipToT");


    fTH1D["badChipIDs"] = new TH1D("badChipIDs", "badChipIDs", 200, 0, 1000000);
  } else if (1 == mode) {
    fFile = TFile::Open(fFilename.c_str());
    readHist("hitmap", "chipmap");
    readHist("rawtot", "chipToT");
    readHist("hittot", "chipToT");

    // -- distributions for rejected pixel hits
    readHist("rj_hm", "chipmap");
    readHist("rj_ht", "chipToT");

  } else {
    cout << "pixelHistograms::init() mode = " << mode << " not implemented" << endl;
  }
}


// ----------------------------------------------------------------------
pixelHistograms::~pixelHistograms() {
  cout << "pixelHistograms::~pixelHistograms() dtor" << endl;

  if (fFile) {
    fFile->Close();
    delete fFile;
  } else {
    plotHistograms("hitmap", "chipmap");
    plotHistograms("hittot", "chipToT");
  
    if (fTH1D["badChipIDs"]) {
      cout << "Total bad chipIDs = " << fTH1D["badChipIDs"]->GetEntries() << endl;
    }
    for (auto it = fTH2D.begin(); it != fTH2D.end(); ++it) {
      delete it->second;
    }
    for (auto it = fTH1D.begin(); it != fTH1D.end(); ++it) {
      delete it->second;
    }
  }
}


// ---------------------------------------------------------------------- 
void pixelHistograms::bookHist(string hname, string hType) {

  for (auto iChip: fAllChips) {
    string name = Form("C%d_%s", iChip, hname.c_str());
    if (string::npos != hType.find("chipmap")) {
      fTH2D[name] = new TH2D(name.c_str(), name.c_str(), 256, 0, 256, 250, 0, 250);
    } else if (string::npos != hType.find("chipToT")) {
      fTH1D[name] = new TH1D(name.c_str(), name.c_str(), 32, 0, 32);
    } 
  }
}


// ---------------------------------------------------------------------- 
void pixelHistograms::readHist(string hname, string hType) {
  string dir = "pixelHistograms";
  for (auto iChip: fAllChips) {
    string name = Form("C%d_%s", iChip, hname.c_str());
    if (string::npos != hType.find("chipmap")) {  
      fTH2D[name] = (TH2D*)fFile->Get(Form("%s/%s", dir.c_str(), name.c_str()));
      cout << "pixelHistograms::readHist() " << name << " = " << fTH2D[name] << endl;
    } else if (string::npos != hType.find("chipToT")) {
      fTH1D[name] = (TH1D*)fFile->Get(Form("%s/%s", dir.c_str(), name.c_str()));
      cout << "pixelHistograms::readHist() " << name << " = " << fTH1D[name] << endl;
    }
  }
  fTH1D["badChipIDs"] = (TH1D*)fFile->Get(Form("%s/badChipIDs", dir.c_str()));
  cout << "pixelHistograms::readHist() badChipIDs = " << fTH1D["badChipIDs"] << endl;
}


// ---------------------------------------------------------------------- 
void pixelHistograms::saveHistograms() {
  gFile->cd();
  gFile->mkdir("pixelHistograms");
  gFile->cd("pixelHistograms");
  for (auto ih: fTH2D) {
    ih.second->SetDirectory(gDirectory);
    ih.second->Write();
  }
  for (auto ih: fTH1D) {
    ih.second->SetDirectory(gDirectory);
    ih.second->Write();
  }
}

// ---------------------------------------------------------------------- 
TH2D* pixelHistograms::getTH2D(std::string hname) {
  return fTH2D[hname];
}

// ---------------------------------------------------------------------- 
TH1D* pixelHistograms::getTH1D(std::string hname) {
  return fTH1D[hname];
}

// ---------------------------------------------------------------------- 
//this is not implemented yet, but for now you can find raw 5-bit tot in bits 31-27 of debug_si_data
bool pixelHistograms::goodPixel(uint32_t pixelid, uint32_t time, double ns, int tot) {
  uint32_t rawtot = tot; //(tot >> 27) & 0x1F;
  uint32_t chipid = ((pixelid >> 16) & 0xFFFF);
  uint32_t col = int((pixelid >> 8) & 0xFF);
  uint32_t row = int((pixelid >> 0) & 0xFF);

  int run = 6000;
  int ckdivend(0), ckdivend2(0);
  if (run < 3098) {
    ckdivend2 = 15;
  } else {
    ckdivend2 = 31;
  }

  uint32_t localTime = time % (1 << 11);  // local pixel time is first 11 bits of the global time
  uint32_t hitToA=localTime * 8/*ns*/ * (ckdivend + 1);
  uint32_t hitToT = tot; //( ( (0x1F+1) + rawtot -  ( (localTime * (ckdivend+1) / (ckdivend2+1) ) & 0x1F) ) & 0x1F);//  * 8 * (ckdivend2+1) ;

  bool isEdgePixel = false;
  if (col <= 11 || col >= 245 || row <= 11 || row >= 239) {
    isEdgePixel = true;
  }

  bool isLowToT = false;
  if (hitToT < 3) {
    isLowToT = true;
  }

  if (chipid > 12000) {
    fTH1D["badChipIDs"]->Fill(chipid);
    return false;
  }

  if (fCalPixelQualityLM) {
    int status = fCalPixelQualityLM->getStatus(chipid, col, row);
    if (status > 0 || isEdgePixel || isLowToT) {
      if (0) cout << "pixelHistograms::goodPixel() chipid = " << chipid << " col = " << col << " row = " << row << " has status " << status 
                  << "hitTot = " << hitToT 
                  << endl;

      string hname = Form("C%d_rj_hm", chipid);
      fTH2D[hname]->Fill(col, row);
      hname = Form("C%d_rj_ht", chipid);
      fTH1D[hname]->Fill(hitToT);

      return false;
    }
  }

  if (0) cout << "pixelHistograms::pixel() id = " << chipid << " col = " << col << " row = " << row 
              << " time = " << time << " ns = " << ns << " rawtot = " << rawtot 
              << " hitToA = " << hitToA << " hitToT = " << hitToT 
              << endl;

  vector<string> histnames = {"hitmap", "hittot", "rawtot"};
  for (auto hist: histnames) {
    string hname = Form("C%d_%s", chipid, hist.c_str());
    // cout << "pixelHistograms::pixel() hname = " << hname << " hist = " << hist << endl;
    if (string::npos != hist.find("hitmap")) {
      fTH2D[hname]->Fill(col, row);
    } else if (string::npos != hist.find("hittot")) {
      fTH1D[hname]->Fill(hitToT);
    } else if (string::npos != hist.find("rawtot")) {
      fTH1D[hname]->Fill(rawtot);
    }
  }

  return true;
}


// ---------------------------------------------------------------------- 
void pixelHistograms::plotHistograms(string hname, string htype) {  
  string opt = "col";
  if (string::npos != htype.find("chipmap")) {
    opt = "col";
  } else if (string::npos != htype.find("chipToT")) {
    opt = "hist";
  }

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

    for (int i = 0; i < fLayer1.size(); ++i) {    
      p->cd(i+1);
      gPad->SetLogz(1);
      gPad->SetBottomMargin(0.0);
      gPad->SetLeftMargin(0.0);
      gPad->SetRightMargin(0.0);
      gPad->SetTopMargin(0.0);
      // cout << "fLayer1[i] = " << fLayer1[i] << " " << fTH2D[fLayer1[i]] << endl;

      string hname = Form("C%d_hitmap", fLayer1[i]);
      TH1 *h;
      if (string::npos != htype.find("chipmap")) {  
        h = fTH2D[hname];
      } else if (string::npos != htype.find("chipToT")) {
        h = fTH1D[hname];
      }
      cout << "pixelHistograms::plotHistograms() " << hname << " = " << h << endl;
      h->Draw(opt.c_str());
      h->SetTitleSize(0.3);
    }

    c->cd(2);
    gPad->SetBottomMargin(0.0);
    gPad->SetLeftMargin(0.0);
    gPad->SetRightMargin(0.0);
    gPad->SetTopMargin(0.0);
    p = (TPad*)c->GetPad(2);
    p->Divide(6,10);
    for (int i = 0; i < fLayer2.size(); ++i) {
      p->cd(i+1);
      gPad->SetLogz(1);
      gPad->SetBottomMargin(0.0);
      gPad->SetLeftMargin(0.0);
      gPad->SetRightMargin(0.0);
      gPad->SetTopMargin(0.0);
      // cout << "fLayer2[i] = " << fLayer2[i] << " " << fTH2D[fLayer2[i]] << endl;
      string hname = Form("C%d_hitmap", fLayer2[i]);
      TH1 *h;
      if (string::npos != htype.find("chipmap")) {  
        h = fTH2D[hname];
      } else if (string::npos != htype.find("chipToT")) {
        h = fTH1D[hname];
      }
      h->Draw(opt.c_str());
    }
    
    // Check if "out" directory exists, create if it doesn't
    struct stat st = {0};
    if (stat("out", &st) == -1) {
        gSystem->mkdir("out", true);
    }
    
    c->SaveAs(("out/pixelHitmaps-" + to_string(fRun) + ".pdf").c_str());
    delete c;
}