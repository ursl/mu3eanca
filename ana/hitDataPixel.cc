#include "hitDataPixel.hh"
#include "hitDataIncludes.hh"

// ----------------------------------------------------------------------
// Run with: see derived classes!
//
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
hitDataPixel::hitDataPixel(TChain *chain, string treeName): hitDataBase(chain, treeName) {
  cout << "==> hitDataPixel: constructor..." << endl;
  if (chain == 0) {
    cout << "You need to pass a chain!" << endl;
  }
  fpChain = chain;
  fChainName = treeName;

  fNentries = fpChain->GetEntries();
  cout << "==> hitDataPixel: constructor fpChain: " << fpChain << "/" << fpChain->GetName()
       << " entries = " <<   fNentries
       << endl;
  
  setupTree();
  
}


// ----------------------------------------------------------------------
hitDataPixel::~hitDataPixel() {
  cout << "==> hitDataPixel: destructor ..." << endl;
  if (!fpChain) return;
  delete fpChain->GetCurrentFile();
}


// ----------------------------------------------------------------------
void hitDataPixel::bookHist(int runnumber) {
  hitDataBase::bookHist(runnumber);
  stringstream ss;

  fpHistFile->cd();
  // -- create hitmap histo
  TH2D *hitmap = new TH2D(Form("hitmap_run%d", runnumber), Form("hits, run %d", runnumber), 256, 0, 256, 250, 0, 250);
  TH1D *hittot = new TH1D(Form("hittot_run%d", runnumber), Form("hits, run %d", runnumber), 32, 0, 256.);
  hittot->SetMinimum(0);
  TH1D *allchiptot = new TH1D(Form("allchiptot_run%d", runnumber), Form("all chips, run %d", runnumber), 32, 0, 256.);
  allchiptot->SetMinimum(0);
  TH1D *badchiptot = new TH1D(Form("badchiptot_run%d", runnumber), Form("bad chips, run %d", runnumber), 32, 0, 256.);
  badchiptot->SetMinimum(0);
  TH1D *noisytot = new TH1D(Form("noisytot_run%d", runnumber), Form("noisy pixels, run %d", runnumber), 32, 0, 256.);
  noisytot->SetMinimum(0);
  for (int i=0; i<120; i++) {
    ss.str("");
    ss << Form("hitmap_run%d_chipID%d", runnumber, i);
    new TH2D(ss.str().c_str(), ss.str().c_str(), 256, 0, 256, 250, 0, 250);
    // cout << "created " << ss.str().c_str() << endl;
    ss.str("");
    ss << Form("hittot_run%d_chipID%d", runnumber, i);
    new TH1D(ss.str().c_str(), ss.str().c_str(), 32, 0, 256.);
    // cout << "created " << ss.str().c_str() << endl;
    
  }

  
}


// ----------------------------------------------------------------------
void hitDataPixel::eventProcessing() {
  TH1D *htotall = (TH1D*)fpHistFile->Get(Form("allchiptot_run%d", fRun));
  //  cout << "htotall = " << htotall << endl;
  
  for (int ihit = 0; ihit < fv_col->size(); ++ihit) {
    int chipID = fv_chipID->at(ihit); 
    // -- weed out scintillator
    if (120 == chipID) {
      //      cout << "scintillator hit" << endl;
      continue;
    }  
    //    continue;

    int row    = fv_row->at(ihit);
    int col    = fv_col->at(ihit); 
    int tot    = fv_tot->at(ihit); 
    ((TH1D*)fpHistFile->Get("allchiptot_run0"))->Fill(tot);
    htotall->Fill(tot);

    if (fBadChips[chipID] > 0) {
      //      ((TH1D*)gROOT->Find("allchiptot"))->Fill(tot);
      continue;
    }
    
    // -- this must stay because chipID > 119 are NOT in gBadChips
    //FIXME    if (skipChip(chipID)) continue;
    
    // vector<pair<int, int> > vnoise = gChipNoisyPixels[chipID];
    // if (vnoise.end() != find(vnoise.begin(), vnoise.end(), make_pair(col, row))) {
    //   //        cout << "noisy pixel on chip = " << chipID << " at col/row = " << col << "/" << row << endl;
    //   noisytot->Fill(tot);
    //   continue;
    // }
    // //      cout << "filling chipID = " << chipID << " col/row = " << col << "/" << row << endl;
    // hitmaps.at(chipID)->Fill(col, row);
    // hittots.at(chipID)->Fill(tot);
    
    // hitmap->Fill(col, row);
    // hittot->Fill(tot);
  }    




}

