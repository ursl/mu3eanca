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
  cout << "==> anaFrameTree: fAllChips.size() = " << fAllChips.size() << endl;
}
  
// ---------------------------------------------------------------------- 
anaFrameTree::~anaFrameTree() {
  
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
  cout << "==> anaFrameTree: Closing histograms file" << endl;
  for (auto &h: fHistograms) {
    h.second->SetDirectory(fpHistFile);
    h.second->Write();
  }
  for (auto &h: fHistogramsProfile) {
    h.second->SetDirectory(fpHistFile);
    h.second->Write();
  }
  for (auto &h: fHistograms2D) {
    h.second->SetDirectory(fpHistFile);
    h.second->Write();
  }
  fpHistFile->mkdir("vtx");
  fpHistFile->cd("vtx");
  TDirectory *vtxDir = gDirectory;
  for (auto &h: fVtx2D) {
    h.second->SetDirectory(vtxDir);
    h.second->Write();
  }
  for (auto &h: fVtx2DProfile) {
    h.second->SetDirectory(vtxDir);
    h.second->Write();
  }
  fpHistFile->cd();
  fpHistFile->Close();
  fpHistFile = 0;
}

// ---------------------------------------------------------------------- 
void anaFrameTree::bookHistograms() {
  if (fVerbose > 0) cout << "==> anaFrameTree: Booking histograms" << endl;

  double NFRAMES(2.5e7);
  fHistograms["frameCounters"] = new TH1D("frameCounters", "frameCounters", 100, 0, 100);
  fHistograms["nHitVsFrameNumber"] = new TH1D("nHitVsFrameNumber", "nHitVsFrameNumber", 1000, 0, NFRAMES);
  fHistograms["nGoodHitVsFrameNumber"] = new TH1D("nGoodHitVsFrameNumber", "nGoodHitVsFrameNumber", 1000, 0, NFRAMES);
  fHistograms["nBadHitVsFrameNumber"] = new TH1D("nBadHitVsFrameNumber", "nBadHitVsFrameNumber", 1000, 0, NFRAMES);
  fHistograms["nInvalidHitVsFrameNumber"] = new TH1D("nInvalidHitVsFrameNumber", "nInvalidHitVsFrameNumber", 1000, 0, NFRAMES);

  fHistogramsProfile["nHitVsFrameNumber"] = new TProfile("nHitVsFrameNumberProfile", "nHitVsFrameNumberProfile", 1000, 0, NFRAMES);
  fHistogramsProfile["nGoodHitVsFrameNumber"] = new TProfile("nGoodHitVsFrameNumberProfile", "nGoodHitVsFrameNumberProfile", 1000, 0, NFRAMES);
  fHistogramsProfile["nBadHitVsFrameNumber"] = new TProfile("nBadHitVsFrameNumberProfile", "nBadHitVsFrameNumberProfile", 1000, 0, NFRAMES);
  fHistogramsProfile["nInvalidHitVsFrameNumber"] = new TProfile("nInvalidHitVsFrameNumberProfile", "nInvalidHitVsFrameNumberProfile", 1000, 0, NFRAMES);

  fHistograms["burstToT"] = new TH1D("burstToT", "burstToT", 32, 0, 32);
  fHistograms["goodToT"] = new TH1D("goodToT", "goodToT", 32, 0, 32);
  fHistograms["badToT"] = new TH1D("badToT", "badToT", 32, 0, 32);

  bookVtx2D("nonburstGood");
  bookVtx2D("burstGood");
  bookVtx2D("burstBad");
  bookVtx2DProfile("burstGoodToT");
  bookVtx2DProfile("burstBadToT");
  bookVtx2DProfile("nonburstGoodToT");
}

// ---------------------------------------------------------------------- 
void anaFrameTree::bookVtx2D(string batch) {
  cout << "==> anaFrameTree: Booking vtx histograms for batch -> " << batch << "<-" << endl;

  for (auto &chip: fAllChips) {
    string name = Form("vtx2D_%s_%d", batch.c_str(), chip);
    fVtx2D[name] = new TH2D(name.c_str(), name.c_str(), 256, 0, 256, 250, 0, 250);
  }
}

// ---------------------------------------------------------------------- 
void anaFrameTree::bookVtx2DProfile(string batch) {
  cout << "==> anaFrameTree: Booking vtx2DProfile histograms for batch -> " << batch << "<-" << endl;

  for (auto &chip: fAllChips) {
    string name = Form("vtx2DProfile_%s_%d", batch.c_str(), chip);
    fVtx2DProfile[name] = new TProfile2D(name.c_str(), name.c_str(), 256, 0, 256, 250, 0, 250);
  }
}

// ---------------------------------------------------------------------- 
void anaFrameTree::endAnalysis() {
  if (fVerbose > 0) cout << "==> anaFrameTree: Ending analysis" << endl;
}


// ---------------------------------------------------------------------- 
void anaFrameTree::loop(int nevents, int start) {
  int maxEvents(0);
 
  cout << "==> anaFrameTree: Chain " << fpChain->GetName()
       << " has a total of " << fNentries << " events" << endl;
  
  if (nevents < 0) {
    maxEvents = fNentries;
  } else {
    maxEvents = nevents;
  }
  cout << "==> anaFrameTree: Running over " << nevents << " events" << endl;

  if (start < 0) {
    start = 0;  
  } else {
    if (maxEvents >  fNentries) {
      cout << "==> anaFrameTree: Requested to run until event " << maxEvents << ", but will run only to end of chain at ";
      maxEvents = fNentries;
      cout << maxEvents << endl;
    } else {
      cout << "==> anaFrameTree: Requested to run until event " << maxEvents << endl;
    }
  }
  cout << "==> anaFrameTree: Starting at event " << start << endl;
 
  if (fpChain == 0) return;

  if (maxEvents > fNentries) {
     cout << "==> anaFrameTree: Requested to run until event " << nevents << ", but will run only to end of chain at ";
     maxEvents = fNentries;
     cout << maxEvents << endl;
  }

  cout << "==> anaFrameTree: maxEvents = " << maxEvents << endl;

   // -- The main loop
  int step(100000);
  if (maxEvents < 1000000) step = 10000;
  if (maxEvents < 100000)  step = 1000;
  if (maxEvents < 10000)   step = 500;
  if (maxEvents < 1000)    step = 100;

   Long64_t nbytes = 0, nb = 0;
   for (Long64_t jentry = start; jentry < maxEvents ; ++jentry) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fpChain->GetEntry(jentry);   
      nbytes += nb;
      if (jentry % step == 0) cout << "==> anaFrameTree: Processing event " << setw(8) << setfill(' ') << jentry 
                                   << " of " << maxEvents << " (" << Form("%4.1f",  jentry*100./maxEvents) << "%) " 
                                   << endl;

      // -- is the hit multiplicity constant?
      fHistograms["nHitVsFrameNumber"]->Fill(jentry, hitN);
      fHistogramsProfile["nHitVsFrameNumber"]->Fill(jentry, hitN);

      // -- frame counters
      TH1 *hc = fHistograms["frameCounters"];
      hc->Fill(1);
      hc->Fill(10, hitN);
      if (0 == hitN) hc->Fill(9);

      int badHitN(0), goodHitN(0), invalidHitN(0);
      for (int ihit = 0; ihit < hitN; ++ihit) {
        if (hitValidHit[ihit]) {
          hc->Fill(11);
          if (hitStatus[ihit] == 0) {
            fHistograms["nGoodHitVsFrameNumber"]->Fill(jentry);
            goodHitN++;
          } else {
            fHistograms["nBadHitVsFrameNumber"]->Fill(jentry);
            badHitN++;
          }
        } else {
          hc->Fill(12);
          fHistograms["nInvalidHitVsFrameNumber"]->Fill(jentry);
          invalidHitN++;
        }
      }
      fHistogramsProfile["nGoodHitVsFrameNumber"]->Fill(jentry, goodHitN);
      fHistogramsProfile["nBadHitVsFrameNumber"]->Fill(jentry, badHitN);
      fHistogramsProfile["nInvalidHitVsFrameNumber"]->Fill(jentry, invalidHitN);
      if (badHitN > 8) {
        for (int ihit = 0; ihit < hitN; ++ihit) {
          if (hitValidHit[ihit]) {
            if (hitStatus[ihit] == 0) {
              fVtx2D[Form("vtx2D_burstGood_%d", hitChipID[ihit])]->Fill(hitCol[ihit], hitRow[ihit]);
              fVtx2DProfile[Form("vtx2DProfile_burstGoodToT_%d", hitChipID[ihit])]->Fill(hitCol[ihit], hitRow[ihit], hitToT[ihit]);
            } else {
              fVtx2D[Form("vtx2D_burstBad_%d", hitChipID[ihit])]->Fill(hitCol[ihit], hitRow[ihit]);
              fVtx2DProfile[Form("vtx2DProfile_burstBadToT_%d", hitChipID[ihit])]->Fill(hitCol[ihit], hitRow[ihit], hitToT[ihit]);
            }
          }
        }
      } else {
        for (int ihit = 0; ihit < hitN; ++ihit) {
          if (hitValidHit[ihit] && hitStatus[ihit] == 0) {
            fVtx2D[Form("vtx2D_nonburstGood_%d", hitChipID[ihit])]->Fill(hitCol[ihit], hitRow[ihit]);
            fVtx2DProfile[Form("vtx2DProfile_nonburstGoodToT_%d", hitChipID[ihit])]->Fill(hitCol[ihit], hitRow[ihit], hitToT[ihit]);
          }
        }
      }
    } 
   
}
  
// ---------------------------------------------------------------------- 
Int_t anaFrameTree::GetEntry(Long64_t entry) {
  // Read contents of entry.
  if (!fpChain) return 0;
  return fpChain->GetEntry(entry);
}

// ---------------------------------------------------------------------- 
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
    fpChain->SetBranchAddress("hitX", hitX, &b_hitX);
    fpChain->SetBranchAddress("hitY", hitY, &b_hitY);
    fpChain->SetBranchAddress("hitZ", hitZ, &b_hitZ);
    fpChain->SetBranchAddress("hitRawToT", hitRawToT, &b_hitRawToT);
    fpChain->SetBranchAddress("hitBitToT", hitBitToT, &b_hitBitToT);
    fpChain->SetBranchAddress("hitStatus", hitStatus, &b_hitStatus);
    fpChain->SetBranchAddress("hitStatusBits", hitStatusBits, &b_hitStatusBits);
    fpChain->SetBranchAddress("hitValidHit", hitValidHit, &b_hitValidHit);
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
  
