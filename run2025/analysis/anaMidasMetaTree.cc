#include "anaMidasMetaTree.hh"
#include <algorithm>
#include <iostream>

#include <TProfile2D.h>
#include <TStyle.h>

#include "calPixelQualityLM.hh"

using namespace std;

// ----------------------------------------------------------------------
anaMidasMetaTree::anaMidasMetaTree(TTree* tree) {
  if (tree) init(tree);
}

// ----------------------------------------------------------------------
void anaMidasMetaTree::bookHistograms() {

  fMapH1["runNumber"] = new TH1D("runNumber", "runNumber", 1000, 0, 10000);
  fMapH1["globalChipID"] = new TH1D("globalChipID", "globalChipID", 108, 0, 108);
  fMapH1["linkMask"] = new TH1D("linkMask", "linkMask", 100, 0, 100);
  fMapH1["linkMatrix"] = new TH1D("linkMatrix", "linkMatrix", 100, 0, 100);
  fMapH1["nlinks"] = new TH1D("nlinks", "nlinks", 100, 0, 100);
  fMapH1["abcLinkMask"] = new TH1D("abcLinkMask", "abcLinkMask", 100, 0, 100);
  fMapH1["abcLinkErrs"] = new TH1D("abcLinkErrs", "abcLinkErrs", 100, 0, 100);
  fMapTProfile["abcLinkErrs"] = new TProfile("abcLinkErrsProfile", "abcLinkErrsProfile", 324, 0, 324);

  fMapH1["lvdsErrRate0"] = new TH1D("lvdsErrRate0", "lvdsErrRate0", 100, 0, 100);
  fMapH1["lvdsErrRate1"] = new TH1D("lvdsErrRate1", "lvdsErrRate1", 100, 0, 100);
  fMapH1["lvdsErrRate2"] = new TH1D("lvdsErrRate2", "lvdsErrRate2", 100, 0, 100);

  fMapH1["ckdivend"] = new TH1D("ckdivend", "ckdivend", 256, 0, 256);
  fMapH1["ckdivend2"] = new TH1D("ckdivend2", "ckdivend2", 256, 0, 256);
  
  fMapH1["vdacBLPix"] = new TH1D("vdacBLPix", "vdacBLPix", 256, 0, 256);
  fMapTProfile["vdacBLPix"] = new TProfile("vdacBLPixProfile", "vdacBLPixProfile", 108, 0, 108);
  fMapH1["vdacThHigh"] = new TH1D("vdacThHigh", "vdacThHigh", 256, 0, 256);
  fMapTProfile["vdacThHigh"] = new TProfile("vdacThHighProfile", "vdacThHighProfile", 108, 0, 108);
  fMapH1["vdacThLow"] = new TH1D("vdacThLow", "vdacThLow", 256, 0, 256);
  fMapTProfile["vdacThLow"] = new TProfile("vdacThLowProfile", "vdacThLowProfile", 108, 0, 108);

  fMapH1["biasVNOutPix"] = new TH1D("biasVNOutPix", "biasVNOutPix", 256, 0, 256);
  fMapTProfile["biasVNOutPix"] = new TProfile("biasVNOutPixProfile", "biasVNOutPixProfile", 108, 0, 108);
  fMapH1["biasVPDAC"] = new TH1D("biasVPDAC", "biasVPDAC", 256, 0, 256);
  fMapTProfile["biasVPDAC"] = new TProfile("biasVPDACProfile", "biasVPDACProfile", 108, 0, 108);
  fMapH1["biasVNDcl"] = new TH1D("biasVNDcl", "biasVNDcl", 256, 0, 256);
  fMapTProfile["biasVNDcl"] = new TProfile("biasVNDclProfile", "biasVNDclProfile", 108, 0, 108);
  fMapH1["biasVNLVDS"] = new TH1D("biasVNLVDS", "biasVNLVDS", 256, 0, 256);
  fMapTProfile["biasVNLVDS"] = new TProfile("biasVNLVDSProfile", "biasVNLVDSProfile", 108, 0, 108);
  fMapH1["biasVNLVDSDel"] = new TH1D("biasVNLVDSDel", "biasVNLVDSDel", 256, 0, 256);
  fMapTProfile["biasVNLVDSDel"] = new TProfile("biasVNLVDSDelProfile", "biasVNLVDSDelProfile", 108, 0, 108);
  fMapH1["biasVPDcl"] = new TH1D("biasVPDcl", "biasVPDcl", 256, 0, 256);
  fMapTProfile["biasVPDcl"] = new TProfile("biasVPDclProfile", "biasVPDclProfile", 108, 0, 108);
  fMapH1["biasVPTimerDel"] = new TH1D("biasVPTimerDel", "biasVPTimerDel", 256, 0, 256);
  fMapTProfile["biasVPTimerDel"] = new TProfile("biasVPTimerDelProfile", "biasVPTimerDelProfile", 108, 0, 108);

  fMapH1["vdacBaseline"] = new TH1D("vdacBaseline", "vdacBaseline", 256, 0, 256);
  fMapTProfile["vdacBaseline"] = new TProfile("vdacBaselineProfile", "vdacBaselineProfile", 108, 0, 108);

  fMapH2["abcLinkErrors2D"] = new TH2D("abcLinkErrors2D", "abcLinkErrors2D", 324, 0, 324, 3500, 0., 3500.);
  fMapH2["abcLinkErrors2D"]->SetOption("colz");
  fMapH2["abcLinkErrors2D"]->SetTitle("Maximum Link Error Rate / Run");
  fMapH2["abcLinkErrors2D"]->SetXTitle("Global Chip ID");
  fMapH2["abcLinkErrors2D"]->SetYTitle("Run Number");
  fMapH2["abcLinkErrors2D"]->SetZTitle("Maximum Link Error Rate / Run");
  fMapH2["abcLinkErrors2D"]->SetStats(0);

  fMapH2["overflowVsRate"] = new TH2D("overflowVsRate", "overflowVsRate", 100, 0, 10000., 100, 0, 0.1);
  fMapH2["overflowVsRate"]->SetOption("colz");
  fMapH2["overflowVsRate"]->SetTitle("Overflow Rate vs LVDS Overflow Rate");
  fMapH2["overflowVsRate"]->SetXTitle("LVDS Error Rate");
  fMapH2["overflowVsRate"]->SetYTitle("Overflow Rate");
  fMapH2["overflowVsRate"]->SetZTitle("Overflow Rate");
  fMapH2["overflowVsRate"]->SetStats(0);

  fMapH1["abcErrs"] = new TH1D("abcErrs", "abcErrs", 100, 0, 100);
  fMapH1["abcMask"] = new TH1D("abcMask", "abcMask", 100, 0, 100);
  fMapH1["abcMatrix"] = new TH1D("abcMatrix", "abcMatrix", 100, 0, 100);
  fMapH1["abcMatrix"]->SetMinimum(0.5);
  fMapH1["abcMatrix"]->GetXaxis()->SetBinLabel(1, "LnkErr");
  fMapH1["abcMatrix"]->GetXaxis()->SetBinLabel(11, "!LnkErr + 2E");
  fMapH1["abcMatrix"]->GetXaxis()->SetBinLabel(21, "!LnkErr,!msk + 2E");
  fMapH1["abcMatrix"]->GetXaxis()->SetBinLabel(31, "!LnkErr,msk + 2E");
  fMapH1["abcMatrix"]->GetXaxis()->SetBinLabel(41, "LnkErr + 1E");
  fMapH1["abcMatrix"]->GetXaxis()->SetBinLabel(51, "LnkErr,!msk + 1E");
  fMapH1["abcMatrix"]->GetXaxis()->SetBinLabel(61, "LnkErr,msk + 1E");

}

// ----------------------------------------------------------------------
void anaMidasMetaTree::init(TTree* tree) {
  fChain = tree;
  fCurrent = -1;
  if (!fChain) return;

  fChain->SetMakeClass(0);

  fChain->SetBranchAddress("runNumber", &runNumber, &b_runNumber);
  fChain->SetBranchAddress("globalChipID", &globalChipID, &b_globalChipID);
  fChain->SetBranchAddress("linkMask", &linkMask, &b_linkMask);
  fChain->SetBranchAddress("linkMatrix", &linkMatrix, &b_linkMatrix);

  fChain->SetBranchAddress("nlinks", &nlinks, &b_nlinks);
  fChain->SetBranchAddress("abcLinkMask", abcLinkMask, &b_abcLinkMask);
  fChain->SetBranchAddress("abcLinkErrs", abcLinkErrs, &b_abcLinkErrs);
  fChain->SetBranchAddress("abcLinkMatrix", abcLinkMatrix, &b_abcLinkMatrix);

  fChain->SetBranchAddress("lvdsErrRate0", &lvdsErrRate0, &b_lvdsErrRate0);
  fChain->SetBranchAddress("lvdsErrRate1", &lvdsErrRate1, &b_lvdsErrRate1);
  fChain->SetBranchAddress("lvdsErrRate2", &lvdsErrRate2, &b_lvdsErrRate2);

  fChain->SetBranchAddress("ckdivend", &ckdivend, &b_ckdivend);
  fChain->SetBranchAddress("ckdivend2", &ckdivend2, &b_ckdivend2);

  fChain->SetBranchAddress("vdacBLPix", &vdacBLPix, &b_vdacBLPix);
  fChain->SetBranchAddress("vdacThHigh", &vdacThHigh, &b_vdacThHigh);
  fChain->SetBranchAddress("vdacThLow", &vdacThLow, &b_vdacThLow);

  fChain->SetBranchAddress("biasVNOutPix", &biasVNOutPix, &b_biasVNOutPix);
  fChain->SetBranchAddress("biasVPDAC", &biasVPDAC, &b_biasVPDAC);
  fChain->SetBranchAddress("biasVNDcl", &biasVNDcl, &b_biasVNDcl);
  fChain->SetBranchAddress("biasVNLVDS", &biasVNLVDS, &b_biasVNLVDS);
  fChain->SetBranchAddress("biasVNLVDSDel", &biasVNLVDSDel, &b_biasVNLVDSDel);
  fChain->SetBranchAddress("biasVPDcl", &biasVPDcl, &b_biasVPDcl);
  fChain->SetBranchAddress("biasVPTimerDel", &biasVPTimerDel, &b_biasVPTimerDel);

  fChain->SetBranchAddress("vdacBaseline", &vdacBaseline, &b_vdacBaseline);
}

// ----------------------------------------------------------------------
void anaMidasMetaTree::endAnalysis() {
  fOutputFile = new TFile(fOutputFileName.c_str(), "RECREATE");
  fOutputFile->cd();

  makePlots();

  for (auto &h: fMapH1) {
    h.second->SetDirectory(fOutputFile);
    h.second->Write();
  }
  for (auto &h: fMapTProfile) {
    h.second->SetDirectory(fOutputFile);
    h.second->Write();
  }

  for (auto &h: fMapH2) {
    h.second->SetDirectory(fOutputFile);
    h.second->Write();
  } 
  fOutputFile->Close();
  delete fOutputFile;
  fOutputFile = nullptr;
}

// ----------------------------------------------------------------------
Long64_t anaMidasMetaTree::getEntry(Long64_t entry) {
  if (!fChain) return 0;
  return fChain->GetEntry(entry);
}

// ----------------------------------------------------------------------
Long64_t anaMidasMetaTree::loadTree(Long64_t entry) {
  if (!fChain) return -5;
  Long64_t centry = fChain->LoadTree(entry);
  if (centry < 0) return centry;
  if (fChain->GetTreeNumber() != fCurrent) {
    fCurrent = fChain->GetTreeNumber();
  }
  return centry;
}

// ----------------------------------------------------------------------
string convertLinkUShortToTLAT(int linkMask[3], char offset) {
  string tla = "";
  for (size_t j = 0; j < 3; ++j) {
    tla += (char)(linkMask[j] + offset);
  }
  return tla;
}


// ----------------------------------------------------------------------
map<int, AsicInfo> anaMidasMetaTree::loadRunInfo(int run) {
  Long64_t nentries = fChain->GetEntries();
  map<int, AsicInfo> runInfo;
  cout << "anaMidasMetaTree::loop() nentries = " << nentries << endl;

  int oldRunNumber = -1;
  int irun = -1;
  for (Long64_t jentry = 0; jentry < nentries; ++jentry) {
    loadTree(jentry);
    getEntry(jentry);
    if (runNumber == run) {
      AsicInfo ai;
      ai.confId = run;
      ai.globalId = globalChipID;
      ai.fedID = 0;
      ai.idxInSection = 0;
      ai.FEBName = "unset";
      ai.FEBLinkName = "unset";
      ai.linkMask = convertLinkUShortToTLAT(linkMask, '0');
      ai.linkMatrix = convertLinkUShortToTLAT(linkMatrix, 'A');
      ai.abcLinkMask[0] = abcLinkMask[0];
      ai.abcLinkMask[1] = abcLinkMask[1];
      ai.abcLinkMask[2] = abcLinkMask[2];
      ai.abcLinkErrs[0] = abcLinkErrs[0];
      ai.abcLinkErrs[1] = abcLinkErrs[1];
      ai.abcLinkErrs[2] = abcLinkErrs[2];
      ai.abcLinkMatrix[0] = abcLinkMatrix[0];
      ai.abcLinkMatrix[1] = abcLinkMatrix[1];
      ai.abcLinkMatrix[2] = abcLinkMatrix[2];
      ai.lvdsErrRate0 = lvdsErrRate0;
      ai.lvdsErrRate1 = lvdsErrRate1;
      ai.lvdsErrRate2 = lvdsErrRate2;
      ai.ckdivend = ckdivend;
      ai.ckdivend2 = ckdivend2;
      ai.vdacBLPix = vdacBLPix;
      ai.vdacThHigh = vdacThHigh;
      ai.vdacThLow = vdacThLow;
      ai.biasVNOutPix = biasVNOutPix;
      ai.biasVPDAC = biasVPDAC;
      ai.biasVNDcl = biasVNDcl;
      ai.biasVNLVDS = biasVNLVDS;
      ai.biasVNLVDSDel = biasVNLVDSDel;
      ai.biasVPDcl = biasVPDcl;
      ai.biasVPTimerDel = biasVPTimerDel;
      ai.vdacBaseline = vdacBaseline;
      runInfo[globalChipID] = ai;
    }
  }
  return runInfo;
}

// ----------------------------------------------------------------------
void anaMidasMetaTree::loop(Long64_t maxEntries) {
  if (!fChain) { std::cerr << "No TTree set\n"; return; }
  Long64_t nentries = fChain->GetEntries();
  if (maxEntries >= 0 && maxEntries < nentries) nentries = maxEntries;


  cout << "anaMidasMetaTree::loop() nentries = " << nentries << endl;

  int oldRunNumber = -1;
  int irun = -1;
  calPixelQualityLM *cpq = new calPixelQualityLM();
  string gt = "datav6.3=2025V0";
  string hash = string("tag_pixelqualitylm_") + gt + string("_iov_") + to_string(irun);
  string pdir = string("/Users/ursl/data/mu3e/cdb/payloads/");
  for (Long64_t jentry = 0; jentry < nentries; ++jentry) {
    loadTree(jentry);
    getEntry(jentry);
    // Example hook for user analysis
    // int eCount = std::count(linkMatrix.begin(), linkMatrix.end(), 'E');
    if (runNumber < 3000) continue;
    if (runNumber != oldRunNumber) {
      hash = string("tag_pixelqualitylm_") + gt + string("_iov_") + to_string(runNumber);
      cpq->readPayloadFromFile(hash, pdir);
      cpq->calculate(hash);
      oldRunNumber = runNumber;
      cout << "==> anaMidasMetaTree::loop() new runNumber = " << runNumber << endl;
      fMapH1["runNumber"]->Fill(runNumber);
      for (int i = 0; i < 3; ++i) {
        fMapTProfile["abcLinkErrs"]->Fill(fPlotUtils.vtxLinkIndex(globalChipID, i), abcLinkErrs[i]);
      }
      cout << "globalChipID = " << globalChipID << " abcLinkMask = " << abcLinkMask[0] << " " << abcLinkMask[1] << " " << abcLinkMask[2]
          << " abcLinkErrs = " << abcLinkErrs[0] << " " << abcLinkErrs[1] << " " << abcLinkErrs[2] << endl;
      ++irun;
      if (irun%100 == 0) {
        fMapH2["abcLinkErrors2D"]->GetYaxis()->SetBinLabel(irun+1, Form("%d", runNumber));
      }

      fMapH2["abcLinkErrors2D"]->Fill(fPlotUtils.vtxLinkIndex(globalChipID, 0), irun, abcLinkErrs[0]);

      fMapH2["abcLinkErrors2D"]->Fill(fPlotUtils.vtxLinkIndex(globalChipID, 1), irun, abcLinkErrs[1]);
      fMapH2["abcLinkErrors2D"]->Fill(fPlotUtils.vtxLinkIndex(globalChipID, 2), irun, abcLinkErrs[2]);

      fMapH1["globalChipID"]->Fill(globalChipID);
      fMapH1["nlinks"]->Fill(nlinks);
      fMapH1["abcLinkMask"]->Fill(abcLinkMask[0]);
      fMapH1["abcLinkErrs"]->Fill(abcLinkErrs[0]);
      fMapH1["lvdsErrRate0"]->Fill(lvdsErrRate0);
      fMapH1["lvdsErrRate1"]->Fill(lvdsErrRate1);
      fMapH1["lvdsErrRate2"]->Fill(lvdsErrRate2);
      fMapH1["ckdivend"]->Fill(ckdivend);
      fMapH1["ckdivend2"]->Fill(ckdivend2);
      fMapH1["vdacBLPix"]->Fill(vdacBLPix);
      fMapTProfile["vdacBLPix"]->Fill(fPlotUtils.vtxChipIndex(globalChipID), vdacBLPix);
      fMapH1["vdacThHigh"]->Fill(vdacThHigh);
      fMapTProfile["vdacThHigh"]->Fill(fPlotUtils.vtxChipIndex(globalChipID), vdacThHigh);
      fMapH1["vdacThLow"]->Fill(vdacThLow);
      fMapTProfile["vdacThLow"]->Fill(fPlotUtils.vtxChipIndex(globalChipID), vdacThLow);
      fMapH1["biasVNOutPix"]->Fill(biasVNOutPix);
      fMapTProfile["biasVNOutPix"]->Fill(fPlotUtils.vtxChipIndex(globalChipID), biasVNOutPix);
      fMapH1["biasVPDAC"]->Fill(biasVPDAC);
      fMapTProfile["biasVPDAC"]->Fill(fPlotUtils.vtxChipIndex(globalChipID), biasVPDAC);
      fMapH1["biasVNDcl"]->Fill(biasVNDcl);
      fMapTProfile["biasVNDcl"]->Fill(fPlotUtils.vtxChipIndex(globalChipID), biasVNDcl);
      fMapH1["biasVNLVDS"]->Fill(biasVNLVDS);
      fMapTProfile["biasVNLVDS"]->Fill(fPlotUtils.vtxChipIndex(globalChipID), biasVNLVDS);
      fMapH1["biasVNLVDSDel"]->Fill(biasVNLVDSDel);
      fMapTProfile["biasVNLVDSDel"]->Fill(fPlotUtils.vtxChipIndex(globalChipID), biasVNLVDSDel);
      fMapH1["biasVPDcl"]->Fill(biasVPDcl);
      fMapTProfile["biasVPDcl"]->Fill(fPlotUtils.vtxChipIndex(globalChipID), biasVPDcl);
      fMapH1["biasVPTimerDel"]->Fill(biasVPTimerDel);
      fMapTProfile["biasVPTimerDel"]->Fill(fPlotUtils.vtxChipIndex(globalChipID), biasVPTimerDel);
      fMapH1["vdacBaseline"]->Fill(vdacBaseline);
      fMapTProfile["vdacBaseline"]->Fill(fPlotUtils.vtxChipIndex(globalChipID), vdacBaseline);
    }
    // -- check against duplicate linkMatrix entries
    int cnt0, cnt1, cnt2;
    cnt0 = cnt1 = cnt2 = 0;
    for (int i = 0; i < 3; ++i) {
      if (linkMatrix[i] == 0) cnt0++;
      if (linkMatrix[i] == 1) cnt1++;
      if (linkMatrix[i] == 2) cnt2++;
    }
    if (cnt0 > 1 || cnt1 > 1 || cnt2 > 1) {
      if (0) cout << "XXXXXXXXXXXXXXXXXXXXXXX duplicate linkMatrix = " 
      << linkMatrix[0] << " " << linkMatrix[1] << " " << linkMatrix[2] 
      << " globalChipID = " << globalChipID 
      << " runNumber = " << runNumber 
      << endl;
    }

    double overflowRate = cpq->getLVDSOverflowRate(globalChipID);
    double meanErrRate(0.);
    int nrate(0);
    if (abcLinkMask[0] > 0) {
      meanErrRate += abcLinkErrs[0];
      nrate++;
    }
    if (abcLinkMask[1] > 0) {
      meanErrRate += abcLinkErrs[1];
      nrate++;
    }
    if (abcLinkMask[2] > 0) {
      meanErrRate += abcLinkErrs[2];
      nrate++;
    }
    meanErrRate /= nrate;
    fMapH2["overflowVsRate"]->Fill(meanErrRate, overflowRate);

    // -- check against 9/'E' without corresponding mask!
    cnt0 = cnt1 = cnt2 = 0;
    for (int i = 0; i < 3; ++i) {
      if (linkMatrix[i] == 4) {
        if (linkMask[i] == 1) cnt0++;
      }
    }
    if (cnt0 > 0) {
      if (0) cout << "XXXXXXXXXXXXXXXXXXXXXXX 'E' without corresponding mask. matrix = " 
      << linkMatrix[0] << " " << linkMatrix[1] << " " << linkMatrix[2] 
      << " cnt = " << cnt0
      << " mask = " << linkMask[0] << " " << linkMask[1] << " " << linkMask[2]
      << " globalChipID = " << globalChipID 
      << " runNumber = " << runNumber 
      << endl;
    }
    

    // -- check abcLinkMatrix status
    if (35 == globalChipID || 161 == globalChipID || 1126 == globalChipID) {    
      cout << "globalChipID = " << globalChipID << " abcLinkMatrix = " << abcLinkMatrix[0] << " " << abcLinkMatrix[1] << " " << abcLinkMatrix[2] << endl;
    } else {
      cnt0 = cnt1 = cnt2 = 0;
      for (int i = 0; i < 3; ++i) {
        if (4 == abcLinkMatrix[i]) cnt0++;
        if (0 == abcLinkMask[i]) cnt1++;
      }

      if (4 == abcLinkMatrix[0]) fMapH1["abcMatrix"]->Fill(0.0);
      if (4 == abcLinkMatrix[1]) fMapH1["abcMatrix"]->Fill(1.0);
      if (4 == abcLinkMatrix[2]) fMapH1["abcMatrix"]->Fill(2.0);

      if (2 == cnt0) {
        if (4 != abcLinkMatrix[0]) fMapH1["abcMatrix"]->Fill(10.0);
        if (4 != abcLinkMatrix[1]) {
          fMapH1["abcMatrix"]->Fill(11.0);
          if (1) cout << "ZZZZZZZZZZZZZZZZZZZZZ 'EE' for working B found. matrix = " 
          << " runNumber = " << runNumber 
          << " globalChipID = " << setw(4) << globalChipID 
          << " linkMatrix = " << linkMatrix[0] << " " << linkMatrix[1] << " " << linkMatrix[2]
          << " linkMask = " << linkMask[0] << " " << linkMask[1] << " " << linkMask[2]
          << " abcLinkMatrix = " << abcLinkMatrix[0] << abcLinkMatrix[1] << abcLinkMatrix[2]
          << " abcLinkMask = " << abcLinkMask[0] << abcLinkMask[1] << abcLinkMask[2]
          << endl;
        }
        if (4 != abcLinkMatrix[2]) fMapH1["abcMatrix"]->Fill(12.0);

        // -- check mask status 0 
        if (4 != abcLinkMatrix[0] && 0 == abcLinkMask[0]) fMapH1["abcMatrix"]->Fill(20.0);
        if (4 != abcLinkMatrix[1] && 0 == abcLinkMask[1]) fMapH1["abcMatrix"]->Fill(21.0);
        if (4 != abcLinkMatrix[2] && 0 == abcLinkMask[2]) fMapH1["abcMatrix"]->Fill(22.0);

        // -- check mask status 1 
        if (4 != abcLinkMatrix[0] && 1 == abcLinkMask[0]) fMapH1["abcMatrix"]->Fill(30.0);
        if (4 != abcLinkMatrix[1] && 1 == abcLinkMask[1]) fMapH1["abcMatrix"]->Fill(31.0);
        if (4 != abcLinkMatrix[2] && 1 == abcLinkMask[2]) fMapH1["abcMatrix"]->Fill(32.0);
      }

      if (2 == cnt0) {
        if (4 == abcLinkMatrix[0]) fMapH1["abcMatrix"]->Fill(40.0);
        if (4 == abcLinkMatrix[1]) fMapH1["abcMatrix"]->Fill(41.0);
        if (4 == abcLinkMatrix[2]) fMapH1["abcMatrix"]->Fill(42.0);

        // -- check mask status 0 
        if (4 == abcLinkMatrix[0] && 0 == abcLinkMask[0]) fMapH1["abcMatrix"]->Fill(50.0);
        if (4 == abcLinkMatrix[1] && 0 == abcLinkMask[1]) fMapH1["abcMatrix"]->Fill(51.0);
        if (4 == abcLinkMatrix[2] && 0 == abcLinkMask[2]) fMapH1["abcMatrix"]->Fill(52.0);

        // -- check mask status 1 
        if (4 == abcLinkMatrix[0] && 1 == abcLinkMask[0]) fMapH1["abcMatrix"]->Fill(60.0);
        if (4 == abcLinkMatrix[1] && 1 == abcLinkMask[1]) fMapH1["abcMatrix"]->Fill(61.0);
        if (4 == abcLinkMatrix[2] && 1 == abcLinkMask[2]) fMapH1["abcMatrix"]->Fill(62.0);
      }

      if (0 == abcLinkMask[0]) fMapH1["abcMask"]->Fill(0.0);
      if (0 == abcLinkMask[1]) fMapH1["abcMask"]->Fill(1.0);
      if (0 == abcLinkMask[2]) fMapH1["abcMask"]->Fill(2.0);

    }
  }

  // -- add decorations
  fPlotUtils.resetNextGlobalChipID();
  fMapH2["abcLinkErrors2D"]->GetXaxis()->SetLabelSize(0.02);
  for (int i = 0; i < fPlotUtils.nAllChips(); ++i) {
    fMapH2["abcLinkErrors2D"]->GetXaxis()->SetBinLabel(3*i+1, Form("%d", fPlotUtils.getNextGlobalChipID()));
  }
}


// ----------------------------------------------------------------------
void anaMidasMetaTree::print(int run, int chipID) {
  if (!fChain) { std::cerr << "No TTree set\n"; return; }
  Long64_t nentries = fChain->GetEntries();
  for (Long64_t jentry = 0; jentry < nentries; ++jentry) {
    loadTree(jentry);
    getEntry(jentry);
    // -- print all chip information for one run
    if ((run == runNumber) && (chipID < 0)) {
      cout << "  run =  " << run << " globalId = " << globalChipID;
      cout << " linkMatrix = " << linkMatrix[0] << linkMatrix[1] << linkMatrix[2];
      cout << " linkMask = " << linkMask[0] << linkMask[1] << linkMask[2];
      cout << " abcLinkMask = " << abcLinkMask[0] << abcLinkMask[1] << abcLinkMask[2];
      cout << " abcLinkMatrix = " << abcLinkMatrix[0] << abcLinkMatrix[1] << abcLinkMatrix[2];
      cout << " abcLinkErrs = " << abcLinkErrs[0] << "," << abcLinkErrs[1] << "," << abcLinkErrs[2] << endl;
      continue;
    }
    // -- print one chip information for one run
    if ((run == runNumber) && (chipID == globalChipID)) {
      cout << "  run =  " << run << " globalId = " << globalChipID;
      cout << " linkMatrix = " << linkMatrix[0] << linkMatrix[1] << linkMatrix[2];
      cout << " linkMask = " << linkMask[0] << linkMask[1] << linkMask[2];
      cout << " abcLinkMask = " << abcLinkMask[0] << abcLinkMask[1] << abcLinkMask[2];
      cout << " abcLinkMatrix = " << abcLinkMatrix[0] << abcLinkMatrix[1] << abcLinkMatrix[2];
      cout << " abcLinkErrs = " << abcLinkErrs[0] << "," << abcLinkErrs[1] << "," << abcLinkErrs[2] << endl;
      continue;
    }
    // -- print one chip information for all runs
    if ((run < 0) && (chipID == globalChipID)) {
      cout << "  run =  " << runNumber << " globalId = " << globalChipID;
      cout << " linkMatrix = " << linkMatrix[0] << linkMatrix[1] << linkMatrix[2];
      cout << " linkMask = " << linkMask[0] << linkMask[1] << linkMask[2];
      cout << " abcLinkMask = " << abcLinkMask[0] << abcLinkMask[1] << abcLinkMask[2];
      cout << " abcLinkMatrix = " << abcLinkMatrix[0] << abcLinkMatrix[1] << abcLinkMatrix[2];
      cout << " abcLinkErrs = " << abcLinkErrs[0] << "," << abcLinkErrs[1] << "," << abcLinkErrs[2] << endl;
      continue;
    }
  }
}

// ----------------------------------------------------------------------
void anaMidasMetaTree::makePlots() {
  TCanvas *c = new TCanvas("c", "c", 1600, 800);
  c->cd();
  c->SetLogz(1);
  c->SetRightMargin(0.2);
  fMapH2["abcLinkErrors2D"]->Draw("colz");
  c->SaveAs("abcLinkErrors2D.pdf");

  c->Clear();
  c->SetLogz(1);
  c->SetRightMargin(0.2);
  fMapH2["abcLinkErrors2D"]->SetMinimum(10.);
  fMapH2["abcLinkErrors2D"]->Draw("colz");
  c->SaveAs("abcLinkErrors2D_min10.pdf");

  c->Clear();
  c->SetLogz(1);
  c->SetRightMargin(0.2);
  fMapH2["abcLinkErrors2D"]->SetMinimum(100.);
  fMapH2["abcLinkErrors2D"]->Draw("colz");
  c->SaveAs("abcLinkErrors2D_min100.pdf");

  c->Clear();
  c->SetLogz(1);
  c->SetRightMargin(0.2);
  fMapH2["abcLinkErrors2D"]->SetMinimum(1000.);
  fMapH2["abcLinkErrors2D"]->Draw("colz");
  c->SaveAs("abcLinkErrors2D_min1000.pdf");


  c->cd();
  c->SetLogy(1);
  c->SetRightMargin(0.1);
  c->SetBottomMargin(0.2);
  gStyle->SetOptStat(0);
  fMapH1["abcMatrix"]->Draw("");
  c->SaveAs("abcMatrix.pdf");

  delete c;
  c = nullptr;

}
