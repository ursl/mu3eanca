#include "anaEnsemble.hh"

#include <dirent.h>  /// for directory reading
#include <algorithm> /// for sorting

#include <TROOT.h>
#include <TBranch.h>
#include <TVector3.h>
#include <TChain.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TTimeStamp.h>

#include "util/util.hh"


using namespace std;

// ----------------------------------------------------------------------
anaEnsemble::anaEnsemble(string dirname, string pdf): fDirectory(dirname), fPDFPrefix(pdf) {
  cout << "anaEnsemble::anaEnsemble ctor, fDirectory = "
       << fDirectory
       << endl;
 
  // -- use sequencer_variables*.json
  vector<string> vfiles;
  DIR *folder;
  struct dirent *entry;
  
  folder = opendir(dirname.c_str());
  if (folder == NULL) {
    puts("Unable to read directory");
    return;
  }
  
  while ((entry=readdir(folder))) {
    if (8 == entry->d_type) {
      string sdirentry = entry->d_name;
      if (string::npos == sdirentry.find("sequencer_variables")) continue;
      vfiles.push_back(entry->d_name);
    }
  }
  closedir(folder);
  
  sort(vfiles.begin(), vfiles.end());

  fEnsemble.clear();
  for (auto it: vfiles) {
    if (string::npos != it.find("_US.json")) continue;
    string lname = it;
    replaceAll(lname, string("sequencer_variables_"), string(""));
    replaceAll(lname, string("_DS.json"), string(""));
    fEnsemble.push_back({lname, new anaLadder(fDirectory, lname)});
    ++fnLadders;
  }

};


// ----------------------------------------------------------------------
anaEnsemble::anaEnsemble(string dirname, vector<string>& ladderlist, string pdf): 
  fDirectory(dirname), fPDFPrefix(pdf) {
  cout << "anaEnsemble::anaEnsemble ctor, fDirectory = "
       << fDirectory
       << " reading ladders: "
       << endl;
  for (auto it: ladderlist) {
    cout << it << endl;
  }
 
  fEnsemble.clear();
  for (auto it: ladderlist) {
    if (string::npos != it.find("_US.json")) continue;
    string lname = it;
    replaceAll(lname, string("sequencer_variables_"), string(""));
    replaceAll(lname, string("_DS.json"), string(""));
    fEnsemble.push_back({lname, new anaLadder(fDirectory, lname)});
    ++fnLadders;
  }

}


// ----------------------------------------------------------------------
anaEnsemble::~anaEnsemble() {
  // for (auto it: fEnsemble) {
  //   it.second->printAll();
  // }

  TFile *f = TFile::Open(Form("%s/anaEnsemble.root", fPDFDir.c_str()), "RECREATE");
  for (auto it: fHists) {
    it.second->SetDirectory(f);
    it.second->Write();
  }
  f->Close();
}

  
// ----------------------------------------------------------------------
void anaEnsemble::analysis(int mode) {

  legg = 0;
  c0 = c1 = c2 = c3 = c4 = c5 =0;
  tl = new TLatex();
  box = new TBox();
  pa = new TArrow();
  pl = new TLine();
  legge = 0;

  c0 = (TCanvas*)gROOT->FindObject("c0");
  if (!c0) c0 = new TCanvas("c0","--c0--",0,0,1200,700);

  plotLVCurrents(mode);
  plotLinkQuality(mode);
  plotNoisyPixels(1);
}


// ----------------------------------------------------------------------
void anaEnsemble::plotNoisyPixels(int thr) {
  TH2D *h;
  if (fHists.find(Form("noisypixels%d", thr)) == fHists.end()) {
    h = new TH2D(Form("noisypixels%d", thr), Form("threshold for N(noisy pixels) < %d", thr), 6, 0., 6., 
                 fnLadders, 0., fnLadders);
    labelAxes(h);

    fHists.insert({Form("noisypixels%d", thr), h});
  }

  cout << "== anaEnsemble::plotNoisyPixels" << endl;
  for (auto iy: fEnsemble) {
    cout << "## anaEnsemble::plotNoisyPixels: " << iy.first << endl;
    for (auto ic: iy.second->fAnaNoisyPixels) {
      vector<pair<int, int>> v = ic.second;
      for (unsigned ithr = 0; ithr < v.size(); ++ithr) {
        if (v[ithr].first == thr) {
          int bx = getXbin(ic.first, h);
          int by = getYbin(iy.first, h);
          cout << "   ## anaEnsemble::plotNoisyPixels: n(np) = " << v[ithr].second
               << " thr =  " << v[ithr].first
               << " bins: " << bx << "/" << by
               << endl;
          h->SetBinContent(bx, by, v[ithr].second); 
        }  
      }
    }
  }

  gStyle->SetOptStat(0);
  h->Draw("colztext");
  c0->SaveAs(Form("%s/%sThrForNoisyPixels%d.pdf", fPDFDir.c_str(), fPDFPrefix.c_str(), thr));
}


// ----------------------------------------------------------------------
void anaEnsemble::plotLinkQuality(int mode) {
  TH2D *h;
  if (fHists.find("goodLinks") == fHists.end()) {
    h = new TH2D("goodLinks", "Link quality", 6*3, 0., 6., fnLadders, 0., fnLadders);
    labelAxes(h);
    // -- override x-axis labeling
    TAxis *ha = h->GetXaxis();
    for (int i = 1; i <= 6; ++i) {
      ha->SetBinLabel(3*(i-1) + 1, Form("C%d/0", i));
      ha->SetBinLabel(3*(i-1) + 2, Form("C%d/1", i));
      ha->SetBinLabel(3*(i-1) + 3, Form("C%d/2", i));
    }

    fHists.insert({"goodLinks", h});
  }

  cout << "== anaEnsemble::plotLinkQuality" << endl;
  for (auto iy: fEnsemble) {
    cout << "## anaEnsemble::plotLVCurrents: " << iy.first << endl;
    for (auto ix: iy.second->fAnaErrorRate) {
      int bx = 0;
      sscanf(ix.first.c_str(), "C%d", &bx);
      if (bx < 1 || bx > 6) {
        cout << "XXXXXXXX Chip number invalid: " << ix.first << endl;
        continue;
      }
      int by = getYbin(iy.first, h);
      cout << "   ## anaEnsemble::plotLinkQuality: " << ix.first 
           << " -> " << ix.second.linkErrors[0]
           << " bins: " << bx << "/" << by
           << endl;
      h->SetBinContent((bx-1)*3+1, by, ix.second.linkErrors[0]); 
      h->SetBinContent((bx-1)*3+2, by, ix.second.linkErrors[1]); 
      h->SetBinContent((bx-1)*3+3, by, ix.second.linkErrors[2]); 
    }  
  }
  
  gStyle->SetOptStat(0);
  h->Draw("colztext");
  c0->SaveAs(Form("%s/%sLinkQuality.pdf", fPDFDir.c_str(), fPDFPrefix.c_str()));
}


// ----------------------------------------------------------------------
void anaEnsemble::plotLVCurrents(int mode) {
  TH2D *h;
  if (fHists.find("LVCurrents") == fHists.end()) {
    h = new TH2D("LVCurrents", "LV currents", 6, 0., 6., fnLadders, 0., fnLadders);
    labelAxes(h);
    fHists.insert({"LVCurrents", h});
  }

  cout << "== anaEnsemble::plotLVCurrents" << endl;
  for (auto iy: fEnsemble) {
    cout << "## anaEnsemble::plotLVCurrents: " << iy.first << endl;
    for (auto ix: iy.second->fAnaLVCurrents) {
      int bx = getXbin(ix.first, h);
      int by = getYbin(iy.first, h);
      cout << "   ## anaEnsemble::plotLVCurrents: " << ix.first 
           << " -> " << ix.second
           << " bins: " << bx << "/" << by
           << endl;
      h->SetBinContent(bx, by, ix.second); 
    }  
  }
  
  gStyle->SetOptStat(0);
  h->Draw("colztext");
  c0->SaveAs(Form("%s/%slvcurrents.pdf", fPDFDir.c_str(), fPDFPrefix.c_str()));
}


// ----------------------------------------------------------------------
void anaEnsemble::labelAxes(TH2D *h) {
  TAxis *ha = h->GetXaxis();
  for (int i = 1; i <= h->GetNbinsX(); ++i) {
    ha->SetBinLabel(i, Form("C%d", i));
  }

  ha = h->GetYaxis();
  for (int i = 0; i < fnLadders; ++i) {
    ha->SetBinLabel(i+1, fEnsemble[i].first.c_str());
  }
}


// ----------------------------------------------------------------------
int anaEnsemble::getXbin(string label, TH2D *h) {
  TAxis *ha = h->GetXaxis();
  int idx(-1);
  for (int i = 1; i <= h->GetNbinsX(); ++i) {
    TString la = ha->GetBinLabel(i);
    if (string(la.Data()) == label) {
      idx = i;
      break;
    }
  }
  return idx; 
}


// ----------------------------------------------------------------------
int anaEnsemble::getYbin(string label, TH2D *h) {
  TAxis *ha = h->GetYaxis();
  int idx(-1);
  for (int i = 1; i <= h->GetNbinsY(); ++i) {
    TString la = ha->GetBinLabel(i);
    if (string(la.Data()) == label) {
      idx = i;
      break;
    }
  }
  return idx; 
}


// ----------------------------------------------------------------------
void anaEnsemble::makeCanvas(int i) {
  if (i & 16) {
    c5 = new TCanvas("c5", "c5", 210,   0, 800, 900);
    c5->ToggleEventStatus();
  }
  if (i & 8) {
    c4 = new TCanvas("c4", "c4", 210,   0, 800, 600);
    c4->ToggleEventStatus();
  }
  if (i & 4) {
    c3 = new TCanvas("c3", "c3", 200,  20, 800, 800);
    c3->ToggleEventStatus();
  }
  if (i & 1) {
    //    c1 = new TCanvas("c1", "c1", 20,  60, 1200, 400);
    c1 = new TCanvas("c1", "c1", 20,  60, 1000, 400);
    c1->ToggleEventStatus();
  }
  if (i & 2) {
    c2 = new TCanvas("c2", "c2", 300, 200, 400, 800);
    c2->ToggleEventStatus();
  }
}
