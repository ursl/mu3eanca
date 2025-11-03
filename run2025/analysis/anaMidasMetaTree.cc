#include "anaMidasMetaTree.hh"
#include <algorithm>
#include <iostream>

#include <TProfile2D.h>

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
void anaMidasMetaTree::loop(Long64_t maxEntries) {
  if (!fChain) { std::cerr << "No TTree set\n"; return; }
  Long64_t nentries = fChain->GetEntries();
  if (maxEntries >= 0 && maxEntries < nentries) nentries = maxEntries;


  cout << "anaMidasMetaTree::loop() nentries = " << nentries << endl;

  int oldRunNumber = -1;
  int irun = -1;
  for (Long64_t jentry = 0; jentry < nentries; ++jentry) {
    loadTree(jentry);
    getEntry(jentry);
    // Example hook for user analysis
    // int eCount = std::count(linkMatrix.begin(), linkMatrix.end(), 'E');
    if (runNumber != oldRunNumber) {
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

    // -- check against duplicate linkMask entries
    int cnt0, cnt1, cnt2;
    cnt0 = cnt1 = cnt2 = 0;
    for (int i = 0; i < 3; ++i) {
      if (linkMatrix[i] == 0) cnt0++;
      if (linkMatrix[i] == 1) cnt1++;
      if (linkMatrix[i] == 2) cnt2++;
    }
    if (cnt0 > 1 || cnt1 > 1 || cnt2 > 1) {
      cout << "XXXXXXXXXXXXXXXXXXXXXXX duplicate linkMatrix = " 
      << linkMatrix[0] << " " << linkMatrix[1] << " " << linkMatrix[2] 
      << " globalChipID = " << globalChipID 
      << " runNumber = " << runNumber 
      << endl;
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

  delete c;
  c = nullptr;

}
