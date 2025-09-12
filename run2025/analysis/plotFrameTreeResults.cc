#include "plotFrameTreeResults.hh"

#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>


#include "TROOT.h"
#include "TStyle.h"
#include "TKey.h"
#include "TMath.h"
#include "TMarker.h"
#include "TPad.h"
#include "TRandom3.h"
#include "TString.h"
#include "TCanvas.h"
#include "TLorentzVector.h"
#include "TPad.h"
#include "TF1.h"
#include "THStack.h"
#include "TFitResult.h"
#include "TTimeStamp.h"

#include "../../util/dataset.hh"
#include "../../util/util.hh"

ClassImp(plotFrameTreeResults)

using namespace std;

// ----------------------------------------------------------------------
plotFrameTreeResults::plotFrameTreeResults(string dir, string files, string cuts, string setup):
  plotClass(dir, files, cuts, setup) {
  if (string::npos != files.rfind(".files")) {
    plotClass::loadFiles(files);
  } else if (string::npos != files.rfind(".root")) {
    fHistFile = plotClass::loadFile(files);
    // -- Parse run number from filename - look for digits before ".root"
    fRun = -1;
    size_t dotPos = files.find(".root");
    if (dotPos != string::npos) {
      // Find the last sequence of digits before ".root"
      size_t startPos = dotPos;
      while (startPos > 0 && isdigit(files[startPos - 1])) {
        startPos--;
      }
      if (startPos < dotPos) {
        fRun = stoi(files.substr(startPos, dotPos - startPos));
        cout << "pixelHistograms::init() fRun = " << fRun << endl;
      }
    }
  }

  changeSetup(dir, "plotFrameTreeResults", setup);

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
  cout << "==> plotFrameTreeResults: fAllChips.size() = " << fAllChips.size() << endl;

  gDirectory->ls();
  readHist("all_hitmap", "chipmap");
  readHist("all_hittot", "chipToT");
  readHist("all_hittot", "chipprof2d");
  readHist("trk_hitmap", "chipmap");
}

// ----------------------------------------------------------------------
plotFrameTreeResults::~plotFrameTreeResults() {
  cout << "plotFrameTreeResults destructor" << endl;
  cout << "done with histogram deletion" << endl;
}

// ----------------------------------------------------------------------
void plotFrameTreeResults::makeAll(string what) {
  if (what == "trk") {
    plotTrkGraphs(-1);
    plotTrkHitmaps();
    return;
  } else if (what == "pixel") {
    plotAllPixelHistograms();
    return;
  } 
  // -- plot all
  plotTrkGraphs(-1);
  plotTrkHitmaps();
  plotAllPixelHistograms();
  
}

// ----------------------------------------------------------------------
void   plotFrameTreeResults::plotTrkHitmaps(int run) {
  cout << "plotFrameTreeResults::plotTrkGraphs() run = " << run << endl;

  TCanvas *c = new TCanvas("c", "c", 400, 1000);
  c->Divide(1, 4);

  TH2D *h2d; 
  vector<string> sloop = {"l2top", "l1top", "l1bot", "l2bot"};
  for (int i = 0; i < 4; ++i) {
    c->cd(i+1);
    shrinkPad(0.1,0.1,0.15,0.1);
    h2d = (TH2D*)fHistFile->Get(Form("%s", sloop[i].c_str()));
    h2d->Draw("colz");
  }
  c->SaveAs((fDirectory + "/trkHitmap0.pdf").c_str());
  delete c;

  // -- plot all hits in hitmaps sliced according to chip position
  c = new TCanvas("c", "c", 1400, 1000);
  c->Divide(6, 4);
  for (int il = 0; il < 4; ++il) {
    for (int ic = 0; ic < 6; ++ic) {
      c->cd(il*6 + ic + 1);
      shrinkPad(0.1,0.1,0.15,0.1);
      h2d = (TH2D*)fHistFile->Get(Form("%s_C%d", sloop[il].c_str(), ic+1));
      h2d->Draw("colz");
    }
  }
  c->SaveAs((fDirectory + "/trkHitmap1.pdf").c_str());
  delete c;


  // -- plot all hits in hitmaps sliced according to chip position
  c = new TCanvas("c", "c", 2100, 300);
  c->Divide(6, 1);
  for (int ic = 1; ic <= 6; ++ic) {
    c->cd(ic);
    shrinkPad(0.1,0.1,0.15,0.1);
    h2d = (TH2D*)fHistFile->Get(Form("allHitsXY_C%d", ic));
    cout << "plotting: " << Form("allHitsXY_C%d", ic) << endl;
    h2d->Draw("colz");
  }
  c->SaveAs((fDirectory + "/trkHitmapXY.pdf").c_str());
  delete c;

}

// ----------------------------------------------------------------------
void   plotFrameTreeResults::plotTrkGraphs(int run) {
  cout << "plotFrameTreeResults::plotTrkGraphs() run = " << run << endl;
  fHistFile->cd("trk");
  plotTrkGraphsWithTitleFilter(run, "", 0);


  TCanvas *c = new TCanvas("c", "c", 800, 1000);
  c->Clear();
  gStyle->SetOptStat(0);
  gStyle->SetPadBorderMode(0);
  gStyle->SetPadBorderSize(0);
  gStyle->SetPadTopMargin(0);
  gStyle->SetPadBottomMargin(0);
  gStyle->SetPadLeftMargin(0);
  gStyle->SetPadRightMargin(0);
  gStyle->SetTitleSize(0.3);

  TH1D *h1 = (TH1D*)fHistFile->Get("trkLambda");
  h1->Draw("hist");
  c->SaveAs((fDirectory + "/trkLambda.pdf").c_str());

  h1 = (TH1D*)fHistFile->Get("trkPhi");
  h1->Draw("hist");
  c->SaveAs((fDirectory + "/trkPhi.pdf").c_str());

  TH2D *h2d(0);
  h2d = (TH2D*)fHistFile->Get("trkLambdaPhi");
  setTitles(h2d, "#phi [rad]", "#lambda [rad]");
  h2d->Draw("colz");
  c->SaveAs((fDirectory + "/trkLambdaPhi.pdf").c_str());

}

// ----------------------------------------------------------------------
void plotFrameTreeResults::plotTrkGraphsWithTitleFilter(int run, const string& titleFilter, int filterType) {
  cout << "plotFrameTreeResults::plotTrkGraphsWithTitleFilter() run = " << run << " titleFilter = " << titleFilter << endl;
  fHistFile->cd("trk");
  TList *list = gDirectory->GetListOfKeys();
  TCanvas *c = new TCanvas("c", "c", 1000, 1000);
  TH2D *h2d = new TH2D("h2d", "Tracks/hits in transverse plane", 100, -40, 40, 100, -40, 40);
  h2d->SetStats(0);
  h2d->Draw();
  int cnt(0);
  for (int i = 0; i < list->GetEntries(); ++i) {
    TKey *key = (TKey*)list->At(i);
    TObject *obj = key->ReadObj();
    string sname = obj->GetName();
    string srun = sname.substr(4, sname.find("_frame")-4);
    if (run > -1 && srun != to_string(run)) {
      cout << "plotFrameTreeResults::plotTrkGraphsWithTitleFilter() skipping run " << srun << " not equal to " << run << endl;
      continue;
    }
    TGraph *gr = (TGraph*)obj;
    
    // Apply title filter if specified
    string htitle = gr->GetTitle();
    if (filterType == 0) {
      // -- skip if title does not contain titleFilter
      if (!titleFilter.empty() && htitle.find(titleFilter) == string::npos) {
        cout << "plotFrameTreeResults::plotTrkGraphsWithTitleFilter() skipping graph with title '" << htitle << "' (does not contain '" << titleFilter << "')" << endl;
        continue;
      }
    } else {
      // -- skip if title contains titleFilter
      if (!titleFilter.empty() && htitle.find(titleFilter) != string::npos) {
        cout << "plotFrameTreeResults::plotTrkGraphsWithTitleFilter() skipping graph with title '" << htitle << "' (does not contain '" << titleFilter << "')" << endl;
        continue;
      }
    }    

    if (gr->GetMarkerColor() == kRed) {
      gr->SetLineStyle(kDashed);
      gr->SetMarkerStyle(2);
      gr->SetMarkerSize(2);
    } else {
      gr->SetLineStyle(kDotted);
      gr->SetMarkerStyle(24);
      gr->SetMarkerSize(1);
    }
    gr->SetLineColor(gr->GetMarkerColor());
    string hname = gr->GetName();
    cout << "plotFrameTreeResults::plotTrkGraphsWithTitleFilter() sname = " << sname << " srun = " << srun << " hname = " << hname << " htitle = " << htitle << endl;
    if (cnt == 0) {
      gr->Draw("lp");
    } else {
      gr->Draw("lp");
    }
    cnt++;
  }
  string sname = fDirectory + "/trk-" + to_string(run);
  if (run < 0) {
    sname = fDirectory + "/trk-all";
  }
  if (!titleFilter.empty()) {
    sname += "-" + titleFilter;
  }
  sname += ".pdf";
  c->SaveAs(sname.c_str());
  delete c;
}


// ---------------------------------------------------------------------- 
void plotFrameTreeResults::plotPixelHistograms(string histname, string htype) {  
  cout << "plotFrameTreeResults::plotPixelHistograms() histname = " << histname << " htype = " << htype << endl;
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
        cout << "pixelHistograms::plotHistograms() " << hname << " = " << h << endl;
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
    string outname = fDirectory + "/pixelHistograms-" + histname + "-" + htype;
    if (fRun > 0) {
      outname = fDirectory + "/pixelHistograms-" + histname + "-" + htype + "-" + to_string(fRun);
    }
    c->SaveAs((outname + ".pdf").c_str());
    c->SaveAs((outname + ".png").c_str());
    
    delete c;
}

// ----------------------------------------------------------------------
void plotFrameTreeResults::plotAllPixelHistograms() {
  plotPixelHistograms("trk_hitmap", "chipmap");

  plotPixelHistograms("all_hitmap", "chipmap");
  plotPixelHistograms("all_hittot", "chipToT");

  plotPixelHistograms("all_hittot", "chipprof2d");
  if (fTH1D["badChipIDs"]) {
    cout << "Total bad chipIDs = " << fTH1D["badChipIDs"]->GetEntries() << endl;
  }

}



// ---------------------------------------------------------------------- 
void plotFrameTreeResults::readHist(string hname, string hType) {
  string dir = "pixelHistograms";
  int cnt = 0;
  cout << "plotFrameTreeResults::readHist() hname = " << hname << " hType = " << hType; 
  for (auto iChip: fAllChips) {
    string name = Form("C%d_%s_%s", iChip, hname.c_str(), hType.c_str());
    cnt++;
    if (hType == "chipmap") {  
      fTH2D[name] = (TH2D*)fHistFile->Get(Form("%s/%s", dir.c_str(), name.c_str()));
      //cout << "plotFrameTreeResults::readHist() " << name << " = " << fTH2D[name] << endl;
    } else if (hType == "chipToT") {
      fTH1D[name] = (TH1D*)fHistFile->Get(Form("%s/%s", dir.c_str(), name.c_str()));
      //cout << "plotFrameTreeResults::readHist() " << name << " = " << fTH1D[name] << endl;
    } else if (hType == "chipprof2d") {
      fTProfile2D[name] = (TProfile2D*)fHistFile->Get(Form("%s/%s", dir.c_str(), name.c_str()));
      //cout << "plotFrameTreeResults::readHist() " << name << " = " << fTProfile2D[name] << endl;
    }
  }

  cout << ", read " << cnt << " histograms" << endl; 
  fTH1D["badChipIDs"] = (TH1D*)fHistFile->Get(Form("%s/badChipIDs", dir.c_str()));
  //cout << "plotFrameTreeResults::readHist() badChipIDs = " << fTH1D["badChipIDs"] << endl;
}





// ----------------------------------------------------------------------
void plotFrameTreeResults::loopOverTree(TTree *t, int ifunc, int nevts, int nstart) {
  int nentries = Int_t(t->GetEntries());
  int nbegin(0), nend(nentries);
  if (nevts > 0 && nentries > nevts) {
    nentries = nevts;
    nbegin = 0;
    nend = nevts;
  }
  if (nevts > 0 && nstart > 0) {
    nentries = nstart + nevts;
    nbegin = nstart;
    if (nstart + nevts < t->GetEntries()) {
      nend = nstart + nevts;
    } else {
      nend = t->GetEntries();
    }
  }

  nentries = nend - nstart;

  int step(1000000);
  if (nentries < 5000000)  step = 500000;
  if (nentries < 1000000)  step = 100000;
  if (nentries < 100000)   step = 10000;
  if (nentries < 10000)    step = 1000;
  if (nentries < 1000)     step = 100;
  cout << "==> plotFrameTreeResults::loopOverTree> loop over dataset " << fCds->fName << " in file "
       << t->GetDirectory()->GetName()
       << " with " << nentries << " entries"
       << " nbegin = " << nbegin << " nend = " << nend
       << endl;

  // -- setup loopfunction through pointer to member functions
  //    (this is the reason why this function is NOT in plotClass!)
  void (plotFrameTreeResults::*pF)(void);
  if (ifunc == 1) pF = &plotFrameTreeResults::loopFunction1;

  // ----------------------------
  // -- the real loop starts here
  // ----------------------------
  for (int jentry = nbegin; jentry < nend; jentry++) {
    t->GetEntry(jentry);
    if (jentry%step == 0) {
      TTimeStamp ts;
      cout << Form(" .. evt = %d", jentry)
           << ", time now: " << ts.AsString("lc")
           << endl;
    }

    candAnalysis();
    (this->*pF)();
  }
}

// ----------------------------------------------------------------------
void plotFrameTreeResults::loopFunction1() {

}

