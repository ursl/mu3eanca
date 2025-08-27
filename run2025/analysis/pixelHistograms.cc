#include "pixelHistograms.hh"

#include <TProfile2D.h>
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
pixelHistograms::pixelHistograms(TFile *file) {
  cout << "pixelHistograms::pixelHistograms() ctor" << endl;
  init(file); 
}


// ----------------------------------------------------------------------
pixelHistograms::pixelHistograms(std::string filename, std::string outdir) : fOutDir(outdir) {
  cout << "pixelHistograms::pixelHistograms() ctor" << endl;

  fOutDir = outdir;

  fFile = TFile::Open(filename.c_str());
  if (!fFile) {
    cout << "pixelHistograms::pixelHistograms() ctor: failed to open file " << filename << endl;
    return;
  }

  // -- Parse run number from filename - look for digits before ".root"
  size_t dotPos = filename.find(".root");
  if (dotPos != string::npos) {
    // Find the last sequence of digits before ".root"
    size_t startPos = dotPos;
    while (startPos > 0 && isdigit(filename[startPos - 1])) {
      startPos--;
    }
    if (startPos < dotPos) {
      fRun = stoi(filename.substr(startPos, dotPos - startPos));
      cout << "pixelHistograms::init() fRun = " << fRun << endl;
    }
  }
  readHist("all_hitmap", "chipmap");
  readHist("all_hittot", "chipToT");
  readHist("all_hittot", "chipprof2d");
  // -- distributions for rejected pixel hits
  readHist("rj_hitmap", "chipmap");
  readHist("rj_hittot", "chipToT");


  plotAllHistograms();

}

// ----------------------------------------------------------------------
void pixelHistograms::init(TFile *file) {
  fFile = file;
  cout << "pixelHistograms::init() file = " << file << endl;
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

  fHistLayer1 = new TH1D("layer1", "layer1", fLayer1.size(), 0, fLayer1.size());
  fHistLayer2 = new TH1D("layer2", "layer2", fLayer2.size(), 0, fLayer2.size());

  int cnt(1);
  for (auto iChip: fLayer1) {
    fHistLayer1->SetBinContent(cnt, iChip);
    ++cnt;
  }
  cnt = 1;
  for (auto iChip: fLayer2) {
    fHistLayer2->SetBinContent(cnt, iChip);
    ++cnt;
  }

  fAllChips = fLayer1;
  fAllChips.insert(fAllChips.end(), fLayer2.begin(), fLayer2.end());
  cout << "pixelHistograms::init() fAllChips.size() = " << fAllChips.size() << endl;

  TDirectory *dir = gDirectory;
  fFile->cd();
  fFile->mkdir("pixelHistograms");
  gDirectory->ls();
  fFile->cd("pixelHistograms");
  fDirectory = gDirectory;
  fDirectory->ls();

  // -- all pixel hits
  bookHist("all_hitmap", "chipmap");
  bookHist("all_hittot", "chipToT");
  bookHist("all_hittot", "chipprof2d");

  // -- distributions for various special selections
  bookHist("edge_hitmap", "chipmap");
  bookHist("edge_hittot", "chipToT");
  bookHist("low_hitmap",  "chipmap");

  // -- good pixel hits
  bookHist("ok_hitmap", "chipmap");
  bookHist("ok_hittot", "chipToT");
  bookHist("ok_hittot", "chipprof2d");

  // -- rejected pixel hits
  bookHist("rj_hitmap", "chipmap");
  bookHist("rj_hittot", "chipToT");

  fTH1D["badChipIDs"] = new TH1D("badChipIDs", "badChipIDs", 200, 0, 1000000);

  gDirectory = dir;
  gDirectory->ls();

}


// ----------------------------------------------------------------------
pixelHistograms::~pixelHistograms() {
  cout << "pixelHistograms::~pixelHistograms() dtor" << endl;
}

// ---------------------------------------------------------------------- 
void pixelHistograms::bookHist(string hname, string hType) {

  for (auto iChip: fAllChips) {
    string name = Form("C%d_%s_%s", iChip, hname.c_str(), hType.c_str());
    if (hType == "chipmap") {
      fTH2D[name] = new TH2D(name.c_str(), name.c_str(), 256, 0, 256, 250, 0, 250);
    } else if (hType == "chipToT") {
      fTH1D[name] = new TH1D(name.c_str(), name.c_str(), 32, 0, 32);
    } else if (hType == "chipprof2d") {
      fTProfile2D[name] = new TProfile2D(name.c_str(), name.c_str(), 256, 0, 256, 250, 0, 250);
    }
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
int pixelHistograms::pixelQuality(pixelHit &hitIn) {
  int result(0);
  // -- rawtot should simply be between 0 .. 31. You need ckdivend and ckdivend2 to get something meaningful
  uint32_t chipid = ((hitIn.fPixelID >> 16) & 0xFFFF);
  uint32_t col = int((hitIn.fPixelID >> 8) & 0xFF);
  uint32_t row = int((hitIn.fPixelID >> 0) & 0xFF);
  result = fCalPixelQualityLM->getStatus(chipid, col, row);

  return result;
}

// ---------------------------------------------------------------------- 
int pixelHistograms::goodPixel(pixelHit &hitIn) {
  int result(0);
  // -- rawtot should simply be between 0 .. 31. You need ckdivend and ckdivend2 to get something meaningful
  uint32_t chipid = ((hitIn.fPixelID >> 16) & 0xFFFF);
  uint32_t col    = int((hitIn.fPixelID >> 8) & 0xFF);
  uint32_t row    = int((hitIn.fPixelID >> 0) & 0xFF);

  pixelHit hit = hitIn;
  // -- verify input data
  if (hit.fChipID != chipid) {
    cout << "pixelHistograms::goodPixel() chipid mismatch: " << hit.fChipID << " != " << chipid << endl;
  }
  if (hit.fCol != col) {
    cout << "pixelHistograms::goodPixel() col mismatch: " << hit.fCol << " != " << col << endl;
  }
  if (hit.fRow != row) {
    cout << "pixelHistograms::goodPixel() row mismatch: " << hit.fRow << " != " << row << endl;
  }

  // -- FIXME!!!!
  int run = 6000;
  int ckdivend(0), ckdivend2(0);
  if (run < 3098) {
    ckdivend2 = 15;
  } else {
    ckdivend2 = 31;
  }

  uint32_t rawtot = (hit.fDebugSiData >> 27) & 0x1F;
  uint32_t localTime = hit.fTimeInt % (1 << 11);  // local pixel time is first 11 bits of the global time
  uint32_t hitToA=localTime * 8/*ns*/ * (ckdivend + 1);
  uint32_t hitToT = ( ( (0x1F+1) + rawtot -  ( (localTime * (ckdivend+1) / (ckdivend2+1) ) & 0x1F) ) & 0x1F);//  * 8 * (ckdivend2+1) ;
  hit.fBitToT = hitToT;
  hit.fRawToT = rawtot;

  //cout << "pixelHistograms::goodPixel() hitTot = " << hitToT << " hitTot() = " << hit.hitToT() << endl;

  hit.fStatus = 0;
  hit.fStatusBits = 0;

  bool isEdgePixel = false;
  if (col <= 11 || col >= 245 || row <= 11 || row >= 239) {
    isEdgePixel = true;
    hit.fStatusBits = 1;
  }

  bool isLowToT = false;
  if (hitToT < 3) {
    isLowToT = true;
    hit.fStatusBits |= 2;
  }

    if (chipid > 12000) {
    fTH1D["badChipIDs"]->Fill(chipid);
    result |= (1 << 16);
  }


  string hname("");
  if (result < 99 && fCalPixelQualityLM) {
    int status = fCalPixelQualityLM->getStatus(chipid, col, row);

    if (status > 0 && status < 9) {
      result |= (0x1 << status);
    }

    hname = Form("C%d_all_hitmap_chipmap", chipid);
    fTH2D[hname]->Fill(col, row);
    hname = Form("C%d_all_hittot_chipToT", chipid);
    fTH1D[hname]->Fill(hitToT);
    hname = Form("C%d_all_hittot_chipprof2d", chipid);
    fTProfile2D[hname]->Fill(col, row, hitToT);

    if (isEdgePixel) {
      hname = Form("C%d_edge_hitmap_chipmap", chipid);
      fTH2D[hname]->Fill(col, row);
      hname = Form("C%d_edge_hittot_chipToT", chipid);
      fTH1D[hname]->Fill(hitToT);
    }

    if (isLowToT) {
      hname = Form("C%d_low_hitmap_chipmap", chipid);
      fTH2D[hname]->Fill(col, row);
    } 

    if (isEdgePixel || isLowToT) {
      if (0) cout << "pixelHistograms::goodPixel() chipid = " << chipid << " col = " << col << " row = " << row << " has status " << status 
                  << "hitTot = " << hitToT 
                  << endl;

      hname = Form("C%d_rj_hitmap_chipmap", chipid);
      fTH2D[hname]->Fill(col, row);
      hname = Form("C%d_rj_hittot_chipToT", chipid);
      fTH1D[hname]->Fill(hitToT);
 
      result |= (0x1 << 10); 
    }
  }

  if (0) cout << "pixelHistograms::pixel() id = " << chipid << " col = " << col << " row = " << row 
              << " time = " << hit.fTime << " ns = " <<  hit.fTimeNs << " rawtot = " << hit.fRawToT 
              << " hitToA = " << hitToA << " hitToT = " << hitToT 
              << endl;

  if (0 == result) {
    hname = Form("C%d_ok_hitmap_chipmap", chipid);
    fTH2D[hname]->Fill(col, row);
    hname = Form("C%d_ok_hittot_chipToT", chipid);
    fTH1D[hname]->Fill(hitToT);
    hname = Form("C%d_ok_hittot_chipprof2d", chipid);
    fTProfile2D[hname]->Fill(col, row, hitToT);
  }
  return result;
}

// ---------------------------------------------------------------------- 
void pixelHistograms::saveHistograms() {
  fDirectory->cd();
  cout << "pixelHistograms::saveHistograms() fDirectory->ls(): " << endl;
  //fDirectory->ls();
  for (auto ih: fTH2D) {
    ih.second->SetDirectory(fDirectory);
    ih.second->Write();
  }
  for (auto ih: fTH1D) {
    ih.second->SetDirectory(fDirectory);
    ih.second->Write();
  }
  for (auto ih: fTProfile2D) {
    ih.second->SetDirectory(fDirectory);
    ih.second->Write();
  } 

  fHistLayer1->SetDirectory(fDirectory);
  fHistLayer1->Write();
  fHistLayer2->SetDirectory(fDirectory);
  fHistLayer2->Write();
}


// ---------------------------------------------------------------------- 
void pixelHistograms::plotHistograms(string histname, string htype) {  
  cout << "pixelHistograms::plotHistograms() histname = " << histname << " htype = " << htype << endl;
  string opt = "col";
  if (htype == "chipmap") {
    opt = "col";
  } else if (htype == "chipToT") {
    opt = "hist";
  }
  if (htype == "chipprof2d") {
    opt = "colz";
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

      string hname = Form("C%d_%s_%s", fLayer1[i], histname.c_str(), htype.c_str());
      TH1 *h;
      if (htype == "chipmap") {  
        gPad->SetLogz(1);
        h = fTH2D[hname];
        //cout << "pixelHistograms::plotHistograms() " << hname << " = " << h << endl;
        h->Draw(opt.c_str());
        h->SetTitleSize(0.3);
      } else if (htype == "chipToT") {
        gPad->SetLogz(1);
        h = fTH1D[hname];
        h->Draw(opt.c_str());
        h->SetTitleSize(0.3);
      } else if (htype == "chipprof2d") {
        gPad->SetLogz(0);
        TProfile2D *hp = fTProfile2D[hname];
        //cout << "pixelHistograms::plotHistograms() " << hname << " = " << hp << endl;
        hp->Draw(opt.c_str());
      }
      //cout << "pixelHistograms::plotHistograms() " << hname << " = " << h << endl;
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
      gPad->SetBottomMargin(0.0);
      gPad->SetLeftMargin(0.0);
      gPad->SetRightMargin(0.0);
      gPad->SetTopMargin(0.0);
      // cout << "fLayer2[i] = " << fLayer2[i] << " " << fTH2D[fLayer2[i]] << endl;
      string hname = Form("C%d_%s_%s", fLayer2[i], histname.c_str(), htype.c_str());
      TH1 *h;
      if (htype == "chipmap") {  
        gPad->SetLogz(1);
        h = fTH2D[hname];
        h->Draw(opt.c_str());
        h->SetTitleSize(0.3);
      } else if (htype == "chipToT") {
        gPad->SetLogz(1);
        h = fTH1D[hname];
        h->Draw(opt.c_str());
        h->SetTitleSize(0.3);
      } else if (htype == "chipprof2d") {
        gPad->SetLogz(0);
        TProfile2D *hp = fTProfile2D[hname];
        hp->Draw(opt.c_str());
      }
    }
    
    // -- Check if fOutDir directory exists, create if it doesn't
    struct stat st = {0};
    if (stat(fOutDir.c_str(), &st) == -1) {
        gSystem->mkdir(fOutDir.c_str(), true);
    }
    
    c->SaveAs((fOutDir + "/pixelHistograms-" + histname + "-" + htype + "-" + to_string(fRun) + ".pdf").c_str());
    c->SaveAs((fOutDir + "/pixelHistograms-" + histname + "-" + htype + "-" + to_string(fRun) + ".png").c_str());
    delete c;
}

// ----------------------------------------------------------------------
void pixelHistograms::plotAllHistograms() {
  plotHistograms("all_hitmap", "chipmap");
  plotHistograms("all_hittot", "chipToT");

  plotHistograms("all_hittot", "chipprof2d");
  if (fTH1D["badChipIDs"]) {
    cout << "Total bad chipIDs = " << fTH1D["badChipIDs"]->GetEntries() << endl;
  }
}


// ---------------------------------------------------------------------- 
void pixelHistograms::readHist(string hname, string hType) {
  string dir = "pixelHistograms";
  int cnt = 0;
  cout << "pixelHistograms::readHist() hname = " << hname << " hType = " << hType; 
  for (auto iChip: fAllChips) {
    string name = Form("C%d_%s_%s", iChip, hname.c_str(), hType.c_str());
    cnt++;
    if (hType == "chipmap") {  
      fTH2D[name] = (TH2D*)fFile->Get(Form("%s/%s", dir.c_str(), name.c_str()));
      //cout << "pixelHistograms::readHist() " << name << " = " << fTH2D[name] << endl;
    } else if (hType == "chipToT") {
      fTH1D[name] = (TH1D*)fFile->Get(Form("%s/%s", dir.c_str(), name.c_str()));
      //cout << "pixelHistograms::readHist() " << name << " = " << fTH1D[name] << endl;
    } else if (hType == "chipprof2d") {
      fTProfile2D[name] = (TProfile2D*)fFile->Get(Form("%s/%s", dir.c_str(), name.c_str()));
      //cout << "pixelHistograms::readHist() " << name << " = " << fTProfile2D[name] << endl;
    }
  }

  cout << ", read " << cnt << " histograms" << endl; 
  fTH1D["badChipIDs"] = (TH1D*)fFile->Get(Form("%s/badChipIDs", dir.c_str()));
  //cout << "pixelHistograms::readHist() badChipIDs = " << fTH1D["badChipIDs"] << endl;
}
