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
#include <iomanip>
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

  cout << "==> anaFrameTree: instatiate pixelHistograms, " << "  fHistFileName = " << fHistFileName << endl;
}
  
// ---------------------------------------------------------------------- 
anaFrameTree::~anaFrameTree() {
  delete fpPixelHistograms;
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

  fpPixelHistograms = new pixelHistograms(fpHistFile);
}

// ---------------------------------------------------------------------- 
void anaFrameTree::closeHistFile() {
  cout << "==> anaFrameTree: Closing histograms file " << fHistFileName << endl;
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
  for (auto &h: fVtx1D) {
    h.second->SetDirectory(vtxDir);
    h.second->Write();
  }
  fpHistFile->mkdir("trk");
  fpHistFile->cd("trk");
  for (auto &g: fTrkGraph) {
    g->Write();
  }
  fpHistFile->cd();
  fpPixelHistograms->saveHistograms();
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

  fHistograms["trkPhi"] = new TH1D("trkPhi", "trkPhi", 60, -3.14, 3.14);
  fHistograms["trkLambda"] = new TH1D("trkLambda", "trkLambda", 60, -3.14, 3.14);
  fHistograms["trkChi2"] = new TH1D("trkChi2", "trkChi2", 100, 0, 100);
  fHistograms["trkToT"] = new TH1D("trkToT", "trkToT", 32, 0, 32);
  fHistograms["trkT0SiRMS"] = new TH1D("trkT0SiRMS", "trkT0SiRMS", 100, 0, 100);

  fHistograms2D["trkLambdaPhi"] = new TH2D("trkLambdaPhi", "trk Lambda vs Phi", 60, -3.14, 3.14, 60, -3.14, 3.14);

  fHistograms2D["l1top"] = new TH2D("l1top", "l1top", 256, 0, 256, 250, 0, 250);
  fHistograms2D["l1bot"] = new TH2D("l1bot", "l1bot", 256, 0, 256, 250, 0, 250);
  fHistograms2D["l2top"] = new TH2D("l2top", "l2top", 256, 0, 256, 250, 0, 250);
  fHistograms2D["l2bot"] = new TH2D("l2bot", "l2bot", 256, 0, 256, 250, 0, 250);
  for (int i = 1; i <= 6; ++i) {
    fHistograms2D[Form("l1top_C%d", i)] = new TH2D(Form("l1top_C%d", i), Form("l1top_C%d", i), 256, 0, 256, 250, 0, 250);
    fHistograms2D[Form("l1bot_C%d", i)] = new TH2D(Form("l1bot_C%d", i), Form("l1bot_C%d", i), 256, 0, 256, 250, 0, 250);
    fHistograms2D[Form("l2top_C%d", i)] = new TH2D(Form("l2top_C%d", i), Form("l2top_C%d", i), 256, 0, 256, 250, 0, 250);
    fHistograms2D[Form("l2bot_C%d", i)] = new TH2D(Form("l2bot_C%d", i), Form("l2bot_C%d", i), 256, 0, 256, 250, 0, 250);
  }

  bookVtx2D("nonburstGood");
  bookVtx2D("nonburstBad");
  bookVtx2D("burstGood");
  bookVtx2D("burstBad");
  bookVtx1D("burstGoodBitToT", 32, 0, 32);
  bookVtx1D("burstBadBitToT", 32, 0, 32);
  bookVtx1D("nonburstGoodBitToT", 32, 0, 32);
  bookVtx1D("nonburstBadBitToT", 32, 0, 32);

  bookVtx2DProfile("burstGoodBitToT");
  bookVtx2DProfile("burstBadBitToT");
  bookVtx2DProfile("nonburstGoodBitToT");
  bookVtx2DProfile("nonburstBadBitToT");
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
void anaFrameTree::bookVtx1D(string batch, int nbins, int lo, int hi) {
  cout << "==> anaFrameTree: Booking vtx1D histograms for batch -> " << batch << "<-" << endl;

  for (auto &chip: fAllChips) {
    string name = Form("vtx1D_%s_%d", batch.c_str(), chip);
    fVtx1D[name] = new TH1D(name.c_str(), name.c_str(), nbins, lo, hi);
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
      pixelHit pphit;
      for (int ihit = 0; ihit < hitN; ++ihit) {
        pphit.fPixelID = hitPixelID[ihit];
        pphit.fChipID = hitChipID[ihit];
        pphit.fCol = hitCol[ihit];
        pphit.fRow = hitRow[ihit];
        pphit.fTimeInt = hitTimeInt[ihit];
        pphit.fDebugSiData = hitDebugSiData[ihit];
        pphit.fStatus = hitStatus[ihit];
        // -- fill pixelHistograms
        int goodpixel = fpPixelHistograms->goodPixel(pphit);

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

      // -- vtx histograms
      for (int ihit = 0; ihit < hitN; ++ihit) {
          if (badHitN > 8) {
            if (hitValidHit[ihit]) {
              if (hitStatus[ihit] == 0) {
                fVtx1D[Form("vtx1D_burstGoodBitToT_%d", hitChipID[ihit])]->Fill(hitBitToT[ihit]);
                fVtx2D[Form("vtx2D_burstGood_%d", hitChipID[ihit])]->Fill(hitCol[ihit], hitRow[ihit]);
                fVtx2DProfile[Form("vtx2DProfile_burstGoodBitToT_%d", hitChipID[ihit])]->Fill(hitCol[ihit], hitRow[ihit], hitBitToT[ihit]);
              } else {
                fVtx1D[Form("vtx1D_burstBadBitToT_%d", hitChipID[ihit])]->Fill(hitBitToT[ihit]);
                fVtx2D[Form("vtx2D_burstBad_%d", hitChipID[ihit])]->Fill(hitCol[ihit], hitRow[ihit]);
                fVtx2DProfile[Form("vtx2DProfile_burstBadBitToT_%d", hitChipID[ihit])]->Fill(hitCol[ihit], hitRow[ihit], hitBitToT[ihit]);
              }
            }
          } else {
            if (hitValidHit[ihit]) {
              if (hitStatus[ihit] == 0) {
                fVtx1D[Form("vtx1D_nonburstGoodBitToT_%d", hitChipID[ihit])]->Fill(hitBitToT[ihit]);
                fVtx2D[Form("vtx2D_nonburstGood_%d", hitChipID[ihit])]->Fill(hitCol[ihit], hitRow[ihit]);
                fVtx2DProfile[Form("vtx2DProfile_nonburstGoodBitToT_%d", hitChipID[ihit])]->Fill(hitCol[ihit], hitRow[ihit], hitBitToT[ihit]);
              } else {
                fVtx1D[Form("vtx1D_nonburstBadBitToT_%d", hitChipID[ihit])]->Fill(hitBitToT[ihit]);
                fVtx2D[Form("vtx2D_nonburstBad_%d", hitChipID[ihit])]->Fill(hitCol[ihit], hitRow[ihit]);
                fVtx2DProfile[Form("vtx2DProfile_nonburstBadBitToT_%d", hitChipID[ihit])]->Fill(hitCol[ihit], hitRow[ihit], hitBitToT[ihit]);
              }
            }
          }
      }
      for (int i = 0; i < fTrkN; ++i) {
        addTrkGraph(i);
      }

      printFrame();  
    }
}

// ---------------------------------------------------------------------- 
void anaFrameTree::addTrkGraph(int trkIndex) {

  // -- track histograms
  fHistograms["trkPhi"]->Fill(fTrkPhi[trkIndex]);
  fHistograms["trkLambda"]->Fill(fTrkLambda[trkIndex]);
  fHistograms["trkChi2"]->Fill(fTrkChi2[trkIndex]);
  fHistograms2D["trkLambdaPhi"]->Fill(fTrkPhi[trkIndex], fTrkLambda[trkIndex]);
  fHistograms["trkT0SiRMS"]->Fill(fTrkT0SiRMS[trkIndex]);

  // -- track graph for overlay
  TGraph *gr = new TGraph();
  gr->SetName(Form("run_%d_frame_%lu_trk_%d", run, frameID, trkIndex));
  gr->SetTitle(Form("run_%d_frame_%lu_trk_%d", run, frameID, trkIndex));
  gr->SetMarkerSize(1);
  if (fTrkLambda[trkIndex] < 0) {
    gr->SetMarkerColor(kRed);
    gr->SetMarkerStyle(24);
  } else {
    gr->SetMarkerColor(kBlue);
    gr->SetMarkerStyle(20);
  }
  for (int j = 0; j < fTrkNhits[trkIndex]; ++j) {
    fHistograms["trkToT"]->Fill(hitBitToT[fTrkHitIndices[trkIndex][j]]);
    int layer, ladder, chip, station;
    cout << Form("%3d", fTrkHitIndices[trkIndex][j]) << " chip: ";
    station = getChipTopology(hitPixelID[fTrkHitIndices[trkIndex][j]], layer, ladder, chip);
    cout << Form("%4d c/r=%3d/%3d", hitChipID[fTrkHitIndices[trkIndex][j]], hitCol[fTrkHitIndices[trkIndex][j]], hitRow[fTrkHitIndices[trkIndex][j]]) << " ";
    gr->AddPoint(hitX[fTrkHitIndices[trkIndex][j]], hitY[fTrkHitIndices[trkIndex][j]]);
    module m = getModule(layer, hitChipID[fTrkHitIndices[trkIndex][j]]);
    fHistograms2D[Form("%s", getModuleString(m).c_str())]->Fill(hitCol[fTrkHitIndices[trkIndex][j]], hitRow[fTrkHitIndices[trkIndex][j]]);
    fHistograms2D[Form("%s_C%d", getModuleString(m).c_str(), chip)]->Fill(hitCol[fTrkHitIndices[trkIndex][j]], hitRow[fTrkHitIndices[trkIndex][j]]);
  }
  cout << endl;

  fTrkGraph.push_back(gr);
}

// ---------------------------------------------------------------------- 
int anaFrameTree::getChipTopology(int pixelID, int &layer, int &ladder, int &chip) {
  // Extract sensor ID from bits 16-31 (upper 16 bits)
  uint32_t sensorId = (pixelID >> 16) & 0xFFFF;
  
  // Extract individual components from sensor ID using mu3e::id::sensor logic:
  // |  15  | 14---12 | 11-10 | 9----5 | 4--0 |
  // | NULL | station | layer | ladder | chip |
  int station = (sensorId >> 12) & 0x7;
  layer = (sensorId >> 10) & 0x3;
  ladder = (sensorId >> 5) & 0x1F;
  chip = (sensorId >> 0) & 0x1F;
  
  return station;
}

// ---------------------------------------------------------------------- 
anaFrameTree::module anaFrameTree::getModule(int layer, int chip) {
  if (layer == 0) {
    if (chip <= 134) {
      return l1top;
    } else {
      return l1bot;
    }
  } else {
    if (chip <= 1158) {
      return l2top;
    } else {
      return l2bot;
    }
  }
}

// ---------------------------------------------------------------------- 
std::string anaFrameTree::getModuleString(module m) {
  if (m == l1top) return "l1top";
  if (m == l1bot) return "l1bot";
  if (m == l2top) return "l2top";
  if (m == l2bot) return "l2bot";
}

// ---------------------------------------------------------------------- 
bool anaFrameTree::isTrackHit(int hitIndex) {
  for (int i = 0; i < fTrkN; ++i) {
    for (int j = 0; j < fTrkNhits[i]; ++j) {
      if (fTrkHitIndices[i][j] == hitIndex) return true;
    }
  }
  return false;
}

// ---------------------------------------------------------------------- 
void anaFrameTree::printFrame() {
  if (fTrkN == 0) return;

  int layer, ladder, chip, station;
  if (hitN > 0 || fTrkN > 0) {
    cout << "----------------------------------------" << endl;
    cout << "==> anaFrameTree: " << frameID << " run = " << run << " hitN = " << hitN << " fTrkN = " << fTrkN 
         << (fTrkN > 0 ? "   ***************************" : "") 
         << endl;
    for (int i = 0; i < hitN; ++i) {
      station = getChipTopology(hitPixelID[i], layer, ladder, chip);
      cout << "==> anaFrameTree: Hit " << Form("%3d", i)
           << " chip: " << Form("%4d ST%1d LYR%1d LDR%1d CHP%4d", hitChipID[i], station, layer, ladder, chip)
           << " col: " << Form("%3d", hitCol[i])
           << " row: " << Form("%3d", hitRow[i])
           << " toT: " << Form("%3d", hitBitToT[i])
           << Form(" ns = %10.3f", hitTimeNs[i])
           << (isTrackHit(i) ? " *" : "")
           << endl;
    }

    for (int i = 0; i < fTrkN; ++i) {
      cout << "trk" << Form("%2d", i) << "(" << Form("%+3d", fTrkType[i])
      << " p=" << Form("%+8.3f", fTrkMomentum[i])
      << " chi2=" << Form("%+7.3f", fTrkChi2[i])
      << " angles = " << Form("%+7.3f/%7.3f", fTrkPhi[i], fTrkLambda[i])
      << " t0si/err/rms=" << Form("%+8.3f/%8.3f/%8.3f", fTrkT0Si[i], fTrkT0SiErr[i], fTrkT0SiRMS[i])
      << ": ";
      for (int j = 0; j < fTrkNhits[i]; ++j) {
        cout << Form("%3d", fTrkHitIndices[i][j]) << " ";
        // getChipTopology(hitPixelID[fTrkHitIndices[i][j]], layer, ladder, chip);
        // cout << Form("%4d c/r=%3d/%3d", hitChipID[fTrkHitIndices[i][j]], hitCol[fTrkHitIndices[i][j]], hitRow[fTrkHitIndices[i][j]]) << " ";
      }
      cout << endl;
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
    fpChain->SetBranchAddress("hitTimeInt", hitTimeInt, &b_hitTimeInt);
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
     
     // Track branches
     fpChain->SetBranchAddress("trkN", &fTrkN, &b_trkN);
     fpChain->SetBranchAddress("trkMomentum", fTrkMomentum, &b_trkMomentum);
     fpChain->SetBranchAddress("trkChi2", fTrkChi2, &b_trkChi2);
     fpChain->SetBranchAddress("trkType", fTrkType, &b_trkType);
     fpChain->SetBranchAddress("trkPhi", fTrkPhi, &b_trkPhi);
     fpChain->SetBranchAddress("trkLambda", fTrkLambda, &b_trkLambda);
     fpChain->SetBranchAddress("trkK", fTrkK, &b_trkK);
     fpChain->SetBranchAddress("trkKerr2", fTrkKerr2, &b_trkKerr2);
     fpChain->SetBranchAddress("trkT0", fTrkT0, &b_trkT0);
     fpChain->SetBranchAddress("trkT0Err", fTrkT0Err, &b_trkT0Err);
     fpChain->SetBranchAddress("trkT0RMS", fTrkT0RMS, &b_trkT0RMS);
     fpChain->SetBranchAddress("trkT0Si", fTrkT0Si, &b_trkT0Si);
     fpChain->SetBranchAddress("trkT0SiErr", fTrkT0SiErr, &b_trkT0SiErr);
     fpChain->SetBranchAddress("trkT0SiRMS", fTrkT0SiRMS, &b_trkT0SiRMS);
     fpChain->SetBranchAddress("trkDoca", fTrkDoca, &b_trkDoca);
     fpChain->SetBranchAddress("trkSegmentN", fTrkSegmentN, &b_trkSegmentN);
     fpChain->SetBranchAddress("trkNhits", fTrkNhits, &b_trkNhits);
     fpChain->SetBranchAddress("trkHitIndices", fTrkHitIndices, &b_trkHitIndices);
     
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
  
