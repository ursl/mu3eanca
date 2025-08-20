#include "anaFrameTree.hh"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TFile.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

using namespace std;


// ---------------------------------------------------------------------- 
anaFrameTree::anaFrameTree(TChain *chain) : fpChain(0), fNentries(0) {
  fpChain = chain;
  fNentries = fpChain->GetEntries();
  fChainName = fpChain->GetName();
  Init();
}
  
// ---------------------------------------------------------------------- 
anaFrameTree::~anaFrameTree() {
   if (!fpChain) return;
   delete fpChain->GetCurrentFile();
}

// ---------------------------------------------------------------------- 
void anaFrameTree::startAnalysis() {
  if (fVerbose > 0) cout << "==> anaFrameTree: Starting analysis" << endl;
}

// ---------------------------------------------------------------------- 
void anaFrameTree::openHistFile(std::string histfile) {
  fHistFileName = histfile;
  if (fVerbose > 0) cout << "==> anaFrameTree: Opening histograms file " << fHistFileName << endl;
  fpHistFile = new TFile(fHistFileName.c_str(), "RECREATE");
  fpHistFile->cd();
}


// ---------------------------------------------------------------------- 
void anaFrameTree::closeHistFile() {
  if (fVerbose > 0) cout << "==> anaFrameTree: Closing histograms file" << endl;
  fpHistFile->Close();
  fpHistFile = 0;
}

// ---------------------------------------------------------------------- 
void anaFrameTree::bookHistograms() {
  if (fVerbose > 0) cout << "==> anaFrameTree: Booking histograms" << endl;
}



// ---------------------------------------------------------------------- 
void anaFrameTree::endAnalysis() {
  if (fVerbose > 0) cout << "==> anaFrameTree: Ending analysis" << endl;
}
// ---------------------------------------------------------------------- 
void anaFrameTree::loop(int nevents, int start) {
  int maxEvents(0);
 
  cout << "==> anaFrameTree: Chain " << fpChain->GetName()
       << " has a total of " << fpChain->GetEntriesFast() << " events" << endl;
  
  if (nevents < 0) {
    maxEvents = fpChain->GetEntriesFast();
  } else {
    cout << "==> hitDataBase: Running over " << nevents << " events" << endl;
    maxEvents = nevents;
  }
  if (start < 0) {
    start = 0;
  } else {
    cout << "==> hitDataBase: Starting at event " << start << endl;
    if (maxEvents >  fpChain->GetEntriesFast()) {
      cout << "==> hitDataBase: Requested to run until event " << maxEvents << ", but will run only to end of chain at ";
      maxEvents = fpChain->GetEntriesFast();
      cout << maxEvents << endl;
    } else {
      cout << "==> hitDataBase: Requested to run until event " << maxEvents << endl;
    }
  }
 
   if (fpChain == 0) return;

   Long64_t nentries = fpChain->GetEntriesFast();
   if (nevents > 0) nentries = nevents;

  // -- The main loop
  int step(50000);
  if (maxEvents < 1000000) step = 10000;
  if (maxEvents < 100000)  step = 1000;
  if (maxEvents < 10000)   step = 500;
  if (maxEvents < 1000)    step = 100;

   Long64_t nbytes = 0, nb = 0;
   for (Long64_t jentry=start; jentry < nentries ; ++jentry) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fpChain->GetEntry(jentry);   nbytes += nb;
      if (jentry % step == 0) cout << "==> anaFrameTree: Processing event " << setw(8) << setfill(' ') << jentry 
                                   << " of " << nentries << " (" << jentry*100./nentries << "%) " 
                                   << endl;

      // if (Cut(ientry) < 0) continue;
   }
}
  
// ---------------------------------------------------------------------- 
Int_t anaFrameTree::GetEntry(Long64_t entry) {
  // Read contents of entry.
  if (!fpChain) return 0;
  return fpChain->GetEntry(entry);
  }
  Long64_t anaFrameTree::LoadTree(Long64_t entry) {
  // Set the environment to read one entry
     if (!fpChain) return -5;
     Long64_t centry = fpChain->LoadTree(entry);
     if (centry < 0) return centry;
     if (fpChain->GetTreeNumber() != fCurrent) {
        fCurrent = fpChain->GetTreeNumber();
        Notify();
     }
     return centry;
  }
  
// ---------------------------------------------------------------------- 
void anaFrameTree::Init() {
    // The Init() function is called when the selector needs to initialize
    // a new tree or chain. Typically here the branch addresses and branch
    // pointers of the tree will be set.
    // It is normally not necessary to make changes to the generated
    // code, but the routine can be extended by the user if needed.
    // Init() will be called many times when running on PROOF
    // (once per file to be processed).

    // Set branch addresses and branch pointers
    if (!fpChain) return;

    fCurrent = -1;

    fpChain->SetBranchAddress("run", &run, &b_run);
    fpChain->SetBranchAddress("frameID", &frameID, &b_frameID);
    fpChain->SetBranchAddress("hitN", &hitN, &b_hitN);
    fpChain->SetBranchAddress("hitPixelID", hitPixelID, &b_hitPixelID);
    fpChain->SetBranchAddress("hitToT", hitToT, &b_hitToT);
    fpChain->SetBranchAddress("hitDebugSiData", hitDebugSiData, &b_hitDebugSiData);
    fpChain->SetBranchAddress("hitChipID", hitChipID, &b_hitChipID);
    fpChain->SetBranchAddress("hitCol", hitCol, &b_hitCol);
    fpChain->SetBranchAddress("hitRow", hitRow, &b_hitRow);
    fpChain->SetBranchAddress("hitTime", hitTime, &b_hitTime);
    fpChain->SetBranchAddress("hitTimeNs", hitTimeNs, &b_hitTimeNs);
    fpChain->SetBranchAddress("hitRawToT", hitRawToT, &b_hitRawToT);
    fpChain->SetBranchAddress("hitBitToT", hitBitToT, &b_hitBitToT);
    fpChain->SetBranchAddress("hitStatus", hitStatus, &b_hitStatus);
    fpChain->SetBranchAddress("hitStatusBits", hitStatusBits, &b_hitStatusBits);
    Notify();
}
  
// ---------------------------------------------------------------------- 
bool anaFrameTree::Notify() {
  // The Notify() function is called when a new file is opened. This
  // can be either for a new TTree in a TChain or when when a new TTree
  // is started when using PROOF. It is normally not necessary to make changes
  // to the generated code, but the routine can be extended by the
  // user if needed. The return value is currently not used.

  return true;
}
  
