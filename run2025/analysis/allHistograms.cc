#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
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
    gFile->cd("timecorrelations/pixel");
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
        h2->Rebin2D(4,10);
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
        h2->Rebin2D(4,10);
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

    // -- time correlations
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
