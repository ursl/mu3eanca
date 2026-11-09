#include <TDirectory.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <string.h>
#include <chrono>
#include <algorithm> // for std::lower_bound
#include <dirent.h>
#include <sys/stat.h>

#include <fstream>

#include "Mu3eConditions.hh"
#include "calTileAlignment.hh"
#include "calTileQuality.hh"

#include <nlohmann/json.hpp>
using json = nlohmann::ordered_json;

#include "cdbJSON.hh"
#include "cdbUtil.hh"
#include "base64.hh"

#include "TCanvas.h"
#include "TStyle.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TMath.h"
#include "TKey.h"
#include "TROOT.h"
#include "TSystem.h"

#include <map>
#include <cstdint>

using namespace std;

// ----------------------------------------------------------------------
// lookAtFrameRatesVsRuns -f combined.root [-n hL1_lad2]
// ------------------
// Writes hL1_lad2.gif (or the histogram named with -n), one frame per run.
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
static int runNumber(const string& name) {
  int n = 0;
  bool any = false;
  for (char c : name) {
    if (c >= '0' && c <= '9') {
      any = true;
      n = n * 10 + (c - '0');
    } else if (any) {
      break;
    }
  }
  return any ? n : 0;
}


// ----------------------------------------------------------------------
int main(int argc, char** argv) {

  if (argc < 2) {
    cout << "Usage: " << argv[0] << " combined.root run*.root" << endl;
    return 1;
  }

  string rootFilename("nada.root");
  string hname("hL1_lad2");
  bool help(false);
  // -- command line parsing
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-f"))      {rootFilename = argv[++i];}
    if (!strcmp(argv[i], "-n"))      {hname = argv[++i];}
    if (!strcmp(argv[i], "-h"))      {help = true;}
  }

  // -- provide glimpse into what is around
  if (help) {
    TFile *f = new TFile(rootFilename.c_str(), "READ");
    if (!f || f->IsZombie()) {
      cout << "Error: could not open " << rootFilename << endl;
      return 1;
    }
    TIter next(f->GetListOfKeys());
    TKey* key = 0;
    while ((key = (TKey*)next())) {
      TObject* obj = key->ReadObj();
      TDirectoryFile* d = dynamic_cast<TDirectoryFile*>(obj);
      cout << obj->GetName() << " d: " << d << endl;
      if (d) {
        d->cd();
        TIter hNext(d->GetListOfKeys());
        TKey *hKey(0); 
        while ((hKey = (TKey*)hNext())) {
          TObject* hobj = hKey->ReadObj();
          cout << hobj->GetName() << endl;
          TH1* h = dynamic_cast<TH1*>(hobj);
          if (!h) continue;
        }
        return 0; 
      }
    }
    return 0;
  }

  // -- make an animated gif for "hL1_lad2" (or whatever)
  TFile *f = new TFile(rootFilename.c_str(), "READ");
  if (!f || f->IsZombie()) {
    cout << "Error: could not open " << rootFilename << endl;
    return 1;
  }
  vector<string> runs;
  TIter next(f->GetListOfKeys());
  TKey* key = 0;
  while ((key = (TKey*)next())) {
    if (strcmp(key->GetClassName(), "TDirectoryFile") != 0) continue;
    runs.push_back(key->GetName());
  }
  sort(runs.begin(), runs.end(), [](const string& a, const string& b) {
    int na = runNumber(a);
    int nb = runNumber(b);
    if (na != nb) return na < nb;
    return a < b;
  });

  gROOT->SetBatch(true);
  gStyle->SetOptStat(0);
  TCanvas* c = new TCanvas("cGif", hname.c_str(), 800, 600);
  c->SetLogy(1);
  string gif = hname + ".gif";
  int nFrame = 0;
  for (const string& run : runs) {
    TH1* h = dynamic_cast<TH1*>(f->Get(Form("%s/%s", run.c_str(), hname.c_str())));
    if (!h) {
      cout << "skip " << run << ", no " << hname << endl;
      continue;
    }
    if (h->GetXaxis()->GetXmax() <= 1.e6) {
      cout << "skip " << run << ", xmax " << h->GetXaxis()->GetXmax() << endl;
      continue;
    }
    h->SetTitle(run.c_str());
    c->SetBottomMargin(0.16);
    h->Draw("HIST");
    c->Modified();
    c->Update();
    c->Print((gif + "+").c_str());
    cout << run << endl;
    ++nFrame;
  }
  if (nFrame == 0) {
    cout << "Error: no frames for " << hname << endl;
    return 1;
  }
  c->Print((gif + "++").c_str());
  cout << "wrote " << gif << "  frames " << nFrame << endl;



  return 0;
}
