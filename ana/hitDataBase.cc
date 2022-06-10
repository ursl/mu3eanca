#include "hitDataBase.hh"
#include "hitDataIncludes.hh"

// ----------------------------------------------------------------------
// Run with: see derived classes!
//
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
hitDataBase::hitDataBase(TChain *chain, string treeName): fVerbose(0) {
  cout << "==> hitDataBase: constructor..." << endl;
  if (chain == 0) {
    cout << "You need to pass a chain!" << endl;
  }
  fpChain = chain;
  fChainName = treeName;

  fNentries = fpChain->GetEntries();
  cout << "==> hitDataBase: constructor fpChain: " << fpChain << "/" << fpChain->GetName()
       << " entries = " <<   fNentries
       << endl;

  setupTree();
  
}


// ----------------------------------------------------------------------
hitDataBase::~hitDataBase() {
  cout << "==> hitDataBase: destructor ..." << endl;
  if (!fpChain) return;
  delete fpChain->GetCurrentFile();
}


// ----------------------------------------------------------------------
pair<int, int> hitDataBase::colrowFromIdx(int idx) {
  int col = idx/256;
  int row = idx%256;
  return make_pair(col, row);
}


// ----------------------------------------------------------------------
int hitDataBase::idxFromColRow(int col, int row) {
  int idx = col*256 + row;
  return idx; 
}


// ----------------------------------------------------------------------
void hitDataBase::startAnalysis() {
  cout << "hitDataBase::startAnalysis() wrong class " << endl;
}

// ----------------------------------------------------------------------
void hitDataBase::endAnalysis() {
  cout << "hitDataBase::endAnalysis() wrong class " << endl;
}


// ----------------------------------------------------------------------
void hitDataBase::eventProcessing() {
  cout << "hitDataBase::eventProcessing() wrong class " << endl;
}



// ----------------------------------------------------------------------
void hitDataBase::fillHist() {
  cout << "hitDataBase::fillHist() wrong class " << endl;
}

// ----------------------------------------------------------------------
void hitDataBase::bookHist(int i) {
  static bool first(true);
  cout << "==> hitDataBase: bookHist> " << i << endl;

  // -- Reduced Tree
  if (first) {
    fTree = new TTree("events", "events");
    fTree->Branch("run",      &fRun,       "run/I");
    fTree->Branch("evt",      &fEvt,       "evt/I");
    first = false;
  }

}


// ----------------------------------------------------------------------
void hitDataBase::initVariables() {
  cout << "hitDataBase::initVariables() wrong class " << endl;
}




// ----------------------------------------------------------------------
void hitDataBase::openHistFile(string filename) {
  fpHistFile = new TFile(filename.c_str(), "RECREATE");
  fpHistFile->cd();
  cout << "==> hitDataBase: Opened " << fpHistFile->GetName() << endl;
}

// ----------------------------------------------------------------------
void hitDataBase::closeHistFile() {
  if (fpHistFile) {
    cout << "==> hitDataBase: Writing " << fpHistFile->GetName() << endl;
    fpHistFile->cd();
    fpHistFile->Write();
    fpHistFile->Close();
    delete fpHistFile;
  } else {
    cout << "no output histogram file defined" << endl;
  }
}

// --------------------------------------------------------------------------------------------------
void hitDataBase::readCuts(string filename, int dump) {
  char  buffer[200];
  fCutFile = filename;
  if (dump) cout << "==> hitDataBase: Reading " << fCutFile << " for cut settings" << endl;
  sprintf(buffer, "%s", fCutFile.c_str());
  ifstream is(buffer);
  char CutName[100];
  float CutValue;
  int ok(0);

  string fn(fCutFile);

  if (dump) {
    cout << "====================================" << endl;
    cout << "==> hitDataBase: Cut file  " << fCutFile << endl;
    cout << "------------------------------------" << endl;
  }

  TH1D *hcuts = new TH1D("hcuts", "", 1000, 0., 1000.);
  hcuts->GetXaxis()->SetBinLabel(1, fn.c_str());
  int ibin;

  while (is.getline(buffer, 200, '\n')) {
    ok = 0;
    if (buffer[0] == '#') {continue;}
    if (buffer[0] == '/') {continue;}
    sscanf(buffer, "%s %f", CutName, &CutValue);

    if (!ok) cout << "==> hitDataBase: ERROR: Don't know about variable " << CutName << endl;
  }

  if (dump)  cout << "------------------------------------" << endl;
}


// ----------------------------------------------------------------------
void hitDataBase::setupTree() {
  fv_runID = 0, fv_MIDASEventID = 0, fv_ts2 = 0, fv_hitTime = 0,
    fv_headerTime = 0, fv_headerTimeMajor = 0, fv_subHeaderTime = 0, fv_trigger = 0,
    fv_isInCluster = 0;
  fv_col = 0, fv_row = 0, fv_chipID = 0, fv_fpgaID = 0, fv_chipIDRaw = 0,
    fv_tot = 0, fv_isMUSR = 0, fv_hitType = 0, fv_layer = 0;
  fb_runID = 0, fb_col = 0, fb_row = 0, fb_chipID = 0, fb_MIDASEventID = 0,
    fb_ts2 = 0, fb_hitTime = 0,
    fb_headerTime = 0, fb_headerTimeMajor = 0, fb_subHeaderTime = 0, fb_trigger = 0,
    fb_isInCluster = 0,
    fb_fpgaID = 0, fb_chipIDRaw = 0, fb_tot = 0, fb_isMUSR = 0, fb_hitType = 0, fb_layer = 0;


  fpChain->SetBranchAddress("runID", &fv_runID, &fb_runID);
  fpChain->SetBranchAddress("MIDASEventID", &fv_MIDASEventID, &fb_MIDASEventID);
  fpChain->SetBranchAddress("col", &fv_col, &fb_col);
  fpChain->SetBranchAddress("row", &fv_row, &fb_row);
  fpChain->SetBranchAddress("fpgaID", &fv_fpgaID, &fb_fpgaID);
  fpChain->SetBranchAddress("chipID", &fv_chipID, &fb_chipID);
  fpChain->SetBranchAddress("chipIDRaw", &fv_chipIDRaw, &fb_chipIDRaw);
  fpChain->SetBranchAddress("tot", &fv_tot, &fb_tot);
  fpChain->SetBranchAddress("ts2", &fv_ts2, &fb_ts2);
  fpChain->SetBranchAddress("hitTime", &fv_hitTime, &fb_hitTime);
  fpChain->SetBranchAddress("headerTime", &fv_headerTime, &fb_headerTime);
  fpChain->SetBranchAddress("headerTimeMajor", &fv_headerTimeMajor, &fb_headerTimeMajor);
  fpChain->SetBranchAddress("subHeaderTime", &fv_subHeaderTime, &fb_subHeaderTime);
  fpChain->SetBranchAddress("isMUSR", &fv_isMUSR, &fb_isMUSR);
  fpChain->SetBranchAddress("hitType", &fv_hitType, &fb_hitType);
  fpChain->SetBranchAddress("trigger", &fv_trigger, &fb_trigger);
  fpChain->SetBranchAddress("layer", &fv_layer, &fb_layer);
  fpChain->SetBranchAddress("isInCluster", &fv_isInCluster, &fb_isInCluster);
}


// ----------------------------------------------------------------------
int hitDataBase::loop(int nevents, int start) {
  int maxEvents(0);

  cout << "==> hitDataBase: Chain " << fChainName << " has a total of " << fNentries << " events" << endl;

  // -- Setup for restricted running (not yet foolproof, i.e. bugfree)
  if (nevents < 0) {
    maxEvents = fNentries;
  } else {
    cout << "==> hitDataBase: Running over " << nevents << " events" << endl;
    maxEvents = nevents;
  }
  if (start < 0) {
    start = 0;
  } else {
    cout << "==> hitDataBase: Starting at event " << start << endl;
    if (maxEvents >  fNentries) {
      cout << "==> hitDataBase: Requested to run until event " << maxEvents << ", but will run only to end of chain at ";
      maxEvents = fNentries;
      cout << maxEvents << endl;
    } else {
      cout << "==> hitDataBase: Requested to run until event " << maxEvents << endl;
    }
  }

  // -- The main loop
  int step(50000);
  if (maxEvents < 1000000) step = 10000;
  if (maxEvents < 100000)  step = 1000;
  if (maxEvents < 10000)   step = 500;
  if (maxEvents < 1000)    step = 100;

  uint64_t fnentries = fpChain->GetEntries();
  cout << "----------------------------------------------------------------------" << endl;
  int VERBOSE(0), oldRun(0);
  for (Long64_t ievt = 0; ievt < fnentries; ++ievt) {
    VERBOSE = 0; 
    if (0 == ievt%1000) VERBOSE = 1;
    Long64_t tentry = fpChain->LoadTree(ievt);
    fb_runID->GetEntry(tentry);  
    fb_MIDASEventID->GetEntry(tentry);  
    fb_col->GetEntry(tentry);  
    fb_row->GetEntry(tentry);  
    fb_fpgaID->GetEntry(tentry);  
    fb_chipID->GetEntry(tentry);  
    fb_chipIDRaw->GetEntry(tentry);  
    fb_tot->GetEntry(tentry);  
    fb_ts2->GetEntry(tentry);  
    fb_hitTime->GetEntry(tentry);  
    fb_headerTime->GetEntry(tentry);  
    fb_headerTimeMajor->GetEntry(tentry);  
    fb_subHeaderTime->GetEntry(tentry);  
    fb_isMUSR->GetEntry(tentry);  
    fb_hitType->GetEntry(tentry);  
    fb_trigger->GetEntry(tentry);  
    fb_layer->GetEntry(tentry);  
    fb_isInCluster->GetEntry(tentry);  
    if (VERBOSE) cout << "processing event .. " << ievt << " with nhit = " << fv_col->size()
                      << " tentry = " << tentry
                      <<  " MIDASEventID = "
                      << (fv_MIDASEventID->size() > 0? Form("%d", fv_MIDASEventID->at(0)): "n/a")
                      << " sizes = " << fv_MIDASEventID->size() << "/" << fv_col->size()
                      << endl;
    
    fRun =  (fv_runID->size() > 0? fv_runID->at(0): 0);
    fEvt = (fv_MIDASEventID->size() > 0? fv_MIDASEventID->at(0): -1);
    if (fRun != oldRun) {
      oldRun = fv_runID->at(0);
      bookHist(fv_runID->at(0));
    }
    
    eventProcessing();

  }

  return 0;

}

