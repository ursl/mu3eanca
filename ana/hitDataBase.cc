#include "hitDataBase.hh"
#include "hitDataIncludes.hh"

// ----------------------------------------------------------------------
// Run with: see derived classes!
//
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
hitDataBase::hitDataBase(TChain *chain, string treeName): fVerbose(0), fMode(UNSET) {
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
void hitDataBase::bookHist() {
  cout << "==> hitDataBase: bookHist> " << endl;

  // -- Reduced Tree
  fTree = new TTree("events", "events");
  fTree->Branch("run",      &fRun,       "run/I");
  fTree->Branch("evt",      &fEvt,       "evt/I");


}


// ----------------------------------------------------------------------
void hitDataBase::initVariables() {
  cout << "hitDataBase::initVariables() wrong class " << endl;
}




// ----------------------------------------------------------------------
hitDataBase::~hitDataBase() {
  cout << "==> hitDataBase: destructor ..." << endl;
  if (!fpChain) return;
  delete fpChain->GetCurrentFile();
}

// ----------------------------------------------------------------------
void hitDataBase::openHistFile(string filename) {
  fpHistFile = new TFile(filename.c_str(), "RECREATE");
  fpHistFile->cd();
  cout << "==> hitDataBase: Opened " << fpHistFile->GetName() << endl;
}

// ----------------------------------------------------------------------
void hitDataBase::closeHistFile() {
  cout << "==> hitDataBase: Writing " << fpHistFile->GetName() << endl;
  fpHistFile->cd();
  //  fpHistFile->Write();
  fpHistFile->Close();
  delete fpHistFile;

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

    // if (!strcmp(CutName, "TYPE")) {
    //   TYPE = int(CutValue); ok = 1;
    //   if (dump) cout << "TYPE:           " << TYPE << endl;
    // }

    // if (!strcmp(CutName, "PTLO")) {
    //   PTLO = CutValue; ok = 1;
    //   if (dump) cout << "PTLO:           " << PTLO << " GeV" << endl;
    //   ibin = 11;
    //   hcuts->SetBinContent(ibin, PTLO);
    //   hcuts->GetXaxis()->SetBinLabel(ibin, "p_{T}^{min}(l) [GeV]");
    // }

    // if (!strcmp(CutName, "PTHI")) {
    //   PTHI = CutValue; ok = 1;
    //   if (dump) cout << "PTHI:           " << PTHI << " GeV" << endl;
    //   ibin = 12;
    //   hcuts->SetBinContent(ibin, PTHI);
    //   hcuts->GetXaxis()->SetBinLabel(ibin, "p_{T}^{max}(l) [GeV]");
    // }


    if (!ok) cout << "==> hitDataBase: ERROR: Don't know about variable " << CutName << endl;
  }

  if (dump)  cout << "------------------------------------" << endl;
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

  // Long64_t nbytes = 0, nb = 0;
  // for (Long64_t jentry = 0; jentry < maxEvents; ++jentry) {
  //   Long64_t ientry = LoadTree(jentry);
  //   if (ientry < 0) break;
  //   nb = fpChain->GetEntry(jentry);   nbytes += nb;

  //   // -- fill common variables (fEvt, fRun, etc)
  //   fChainEvent = jentry;
  //   this->commonVar();

  //   if (jentry%step == 0) {
  //     TTimeStamp ts;
  //     cout  << " (runId: " << Form("%8d", fRun)
  //           << ", eventId: " << Form("%8d", fEvt)
  //           << ", chainEvt: " << Form("%10d", fChainEvent)
  //           << ", time now: " << ts.AsString("lc")
  //           << ")" << endl;
  //   }


  //   eventProcessing();
  // }

  return 0;

}

