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
  static bool first(true);
  hitDataBase::bookHist(runnumber);
  cout << "==> hitDataPixel: bookHist> run " << runnumber << endl;
 
  string n("");
  struct hID a; 
  if (first) {
    first = false;
    fTree->Branch("chipID",   &fChipID,  "chipID/I");
    fTree->Branch("col",      &fcol,     "col/I");
    fTree->Branch("row",      &frow,     "row/I");
    fTree->Branch("tot",      &ftot,     "tot/I");
    fTree->Branch("qual",     &fqual,    "qual/I");

    // -- create unified hitmap histo
    n = Form("hitmap_run%d", 0);
    a = hID(0, -1, "hitmap");
    fChipHistograms.insert(make_pair(a, new TH2D(n.c_str(), n.c_str(), 256, 0, 256, 250, 0, 250)));
    cout << "book hID: " << a << ": " << n << " ptr: " << fChipHistograms[a] << endl;
    
    n = Form("hittot_run%d", 0);
    a = hID(0, -1, "hit_tot");
    fChipHistograms.insert(make_pair(a, new TH1D(n.c_str(), n.c_str(), 32, 0, 256.)));
    cout << "book hID: " << a << ": " << n << " ptr: " << fChipHistograms[a] << endl;
    
    n = Form("badchiptot_run%d", 0);
    a = hID(0, -1, "badchiptot");
    fChipHistograms.insert(make_pair(a, new TH1D(n.c_str(), n.c_str(), 32, 0, 256.)));
    cout << "book hID: " << a << ": " << n << " ptr: " << fChipHistograms[a] << endl;
    
    n = Form("noisytot_run%d", 0);
    a = hID(0, -1, "noisytot");
    fChipHistograms.insert(make_pair(a, new TH1D(n.c_str(), n.c_str(), 32, 0, 256.)));
    cout << "book hID: " << a << ": " << n << " ptr: " << fChipHistograms[a] << endl;

    
  }

  for (int i = 0; i < 120; ++i) {
    n = Form("hittot_run%d_chipID%d", runnumber, i);
    a = hID(fRun, i, "hit_tot");
    fChipHistograms.insert(make_pair(a, new TH1D(n.c_str(), n.c_str(), 32, 0, 256.)));
    cout << "book hID: " << a << ": " << n << " ptr: " << fChipHistograms[a] << endl;
    n = Form("hitmap_run%d_chipID%d", runnumber, i);
    a = hID(fRun, i, "hitmap"); 
    fChipHistograms.insert(make_pair(a, new TH2D(n.c_str(), n.c_str(), 256, 0, 256, 250, 0, 250)));
    cout << "book hID: " << a << ": " << n << " ptr: " << fChipHistograms[a] << endl;
  }

}


// ----------------------------------------------------------------------
void hitDataPixel::eventProcessing() {
  //  TH1D *htotall = (TH1D*)fpHistFile->Get(Form("allchiptot_run%d", fRun));
  //  cout << "htotall = " << htotall << endl;

  struct hID ahm(fRun, 0, "hitmap");
  struct hID aht(fRun, 0, "hit_tot");
  struct hID ahm0(0, -1, "hitmap");
  struct hID aht0(0, -1, "hit_tot");
  struct hID ahn0(0, -1, "noisytot");

  for (int ihit = 0; ihit < fv_col->size(); ++ihit) {
    int chipID = fv_chipID->at(ihit); 
    ahm.chipID = chipID;
    aht.chipID = chipID;
    
    // -- weed out scintillator
    if (120 == chipID) {
      //      cout << "scintillator hit" << endl;
      continue;
    }  
    //    continue;

    int row    = fv_row->at(ihit);
    int col    = fv_col->at(ihit); 
    int tot    = fv_tot->at(ihit); 
    //    fChipHistograms[hid(0, -1, "hittot")]->Fill(tot);

    if (fChipQuality[chipID] > 0) {
      //      ((TH1D*)gROOT->Find("allchiptot"))->Fill(tot);
      continue;
    }
    
    // vector<pair<int, int> > vnoise = fChipNoisyPixels[make_pair(fRun, chipID)];
    // if (vnoise.end() != find(vnoise.begin(), vnoise.end(), make_pair(col, row))) {
    //   //        cout << "noisy pixel on chip = " << chipID << " at col/row = " << col << "/" << row << endl;
    //   noisytot->Fill(tot);
    //   continue;
    // }
    // hitmaps.at(chipID)->Fill(col, row);
    // hittots.at(chipID)->Fill(tot);

    fChipHistograms[ahm]->Fill(col, row);
    ((TH2D*)fChipHistograms[ahm0])->Fill(col, row);
    fChipHistograms[aht]->Fill(tot);
    ((TH1D*)fChipHistograms[aht0])->Fill(tot);
  }
}




