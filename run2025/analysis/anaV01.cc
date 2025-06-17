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
#include "TTree.h"
#include "TH1.h"

using namespace std;


// ----------------------------------------------------------------------
// anaV01
// ---------------
//
// Examples:
// bin/anaV01 /data/experiment/mu3e/data/2025/trirec/250613/run03*.root
// ----------------------------------------------------------------------


void chipIDSpecBook(int chipid, int &station, int &layer, int &phi, int &z);

void plotBasics(string filename, TFile *results, map<string, TH1*> &mHistos);

// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {

  // -- command line arguments
  int verbose(0), mode(1);

  // -- read in all files
  vector<string> vFiles;
  for (int i = 1; i < argc; i++) {
    vFiles.push_back(argv[i]);
  }

  map<string, TH1*> mHistos;

  // -- open results file
  TFile *fResults = TFile::Open("out/results.root", "RECREATE");

  // -- create histograms
  mHistos["hRuns"] = new TH1D("hRuns", "Runs", 1000, 3000, 4000);
  mHistos["hFrames"] = new TH2D("hFrames", "Frames", 1000, 3000, 4000, 100, 0, 1.e11);

  // -- process files sequentially
  for (const auto& sFile : vFiles) {
    cout << "Processing file " << sFile << endl;
    // -- Process the file directly
    plotBasics(sFile, fResults, mHistos);
  }

  // -- save results
  fResults->cd();
  for (auto m : mHistos) {
    m.second->Write();
  }
  fResults->Close();
}


// ----------------------------------------------------------------------
void plotBasics(string filename, TFile *results, map<string, TH1*> &mHistos) {
  cout << "plotBasics " << filename << endl;

  // -- create histograms
  string sRun = filename.substr(filename.rfind("/run") + 1);
  sRun = sRun.substr(0, sRun.rfind("-trirec"));
  replaceAll(sRun, "run", "");
  int run = atoi(sRun.c_str());
  sRun = "run" + sRun;
  cout << "sRun " << sRun << " filename " << filename << " run " << run << endl;


  mHistos[sRun + "_frameId"] = new TH1D(string(sRun + "_frameId").c_str(), string(sRun + "_frameId").c_str(), 1000, 0, 1.e11);
  mHistos[sRun + "_runId"] = new TH1D(string(sRun + "_runId").c_str(), string(sRun + "_runId").c_str(), 1000, 0, 1.e11);

  // -- get the trirec tree
  TFile *f = TFile::Open(filename.c_str());
  TTree *tSegs = (TTree*)f->Get("segs");
  if (!tSegs) {
    cout << "ERROR: Could not find 'segs' tree in file" << endl;
    f->Close();
    return;
  }

  // -- get the frames tree
  TTree *tFrames = (TTree*)f->Get("frames");
  if (!tFrames) {
    cout << "ERROR: Could not find 'frames' tree in file" << endl;
    f->Close();
    return;
  }
 
  // -- set branch addresses for segs tree
  unsigned long frameId;
  unsigned int runId;
  tSegs->SetBranchAddress("frameId", &frameId); 
  tSegs->SetBranchAddress("runId", &runId); 

  // -- loop over all segments
  Long64_t nEntries = tSegs->GetEntries();
  cout << "Processing " << nEntries << " segments for run " << run << endl;
  
  for (Long64_t i = 0; i < nEntries; i++) {
    if (i % 10000 == 0) {
      cout << "run " << run << " segment " << i << " of " << nEntries << endl;
    }
    tSegs->GetEntry(i);
    cout << "frameId " << frameId << " runId " << runId << endl;
    mHistos[sRun + "_frameId"]->Fill(frameId);
    mHistos[sRun + "_runId"]->Fill(runId);

    mHistos["hRuns"]->Fill(runId);
    mHistos["hFrames"]->Fill(runId, frameId);
  }

  cout << "Finished processing run " << run << " with " << nEntries << " segments" << endl;

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



