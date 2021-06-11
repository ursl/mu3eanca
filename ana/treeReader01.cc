#include "treeReader01.hh"

#include <fstream>
#include <string>

#include "TRandom.h"

#define MMUON 105.658305

using namespace std;


// ----------------------------------------------------------------------
// Run with: ./runTreeReader01 -c chains/bg-test -D root
//           ./runTreeReader01 -f test.root
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
void treeReader01::startAnalysis() {
  cout << "treeReader01: startAnalysis: ..." << endl;
  DBX = true;
}

// ----------------------------------------------------------------------
void treeReader01::endAnalysis() {
  cout << "treeReader01: endAnalysis: ..." << endl;
}


// ----------------------------------------------------------------------
void treeReader01::eventProcessing() {
  initVariables();

  cout << "evt " << fChainEvent << ": " << feventId << ": fmc_tid.size() = " << fpmc_tid->size() << endl;
  cout << "  ";
  for (unsigned int i = 0; i < fpmc_tid->size(); ++i) {
    cout << fpmc_tid->at(i) << ", ";
  }
  cout << endl;

  // -- generic rudimentary analysis
  if (DBX) {
    cout << "----------------------------------------------------------------------" << endl;
  }

}



// ----------------------------------------------------------------------
void treeReader01::fillHist() {
}

// ----------------------------------------------------------------------
void treeReader01::bookHist() {
  cout << "==> treeReader01: bookHist> " << endl;

  new TH1D("evts", "events", 40, 0., 40.);

  // -- Reduced Tree
  fTree = new TTree("events", "events");
  fTree->Branch("run",      &fRun,       "run/I");
  fTree->Branch("evt",      &fEvt,       "evt/I");


}


// ----------------------------------------------------------------------
void treeReader01::initVariables() {
  //  cout << "treeReader01: initVariables: for run = " << fRun << "/evt = " << fEvt << endl;

}




// ======================================================================
// -- Below is the icc material
// ======================================================================

// ----------------------------------------------------------------------
treeReader01::treeReader01(TChain *chain, TString para) {
  cout << "==> treeReader01: constructor..." << endl;
  if (chain == 0) {
    cout << "You need to pass a chain!" << endl;
  }
  fpChain = chain;
  fNentries = chain->GetEntries();
  init(para);
}

// ----------------------------------------------------------------------
void treeReader01::init(TString treeName) {
  cout << "==> treeReader01: init..." << endl;

  fbmc_tid = 0;
  fbmc_pt = 0;

  fpChain->SetBranchAddress("eventId", &feventId);
  fpChain->SetBranchAddress("runId", &frunId);
  fpChain->SetBranchAddress("mc_tid", &fpmc_tid, &fbmc_tid);

  initVariables();
}

// ----------------------------------------------------------------------
treeReader01::~treeReader01() {
  cout << "==> treeReader01: destructor ..." << endl;
  if (!fpChain) return;
  delete fpChain->GetCurrentFile();
}

// ----------------------------------------------------------------------
void treeReader01::openHistFile(TString filename) {
  fpHistFile = new TFile(filename.Data(), "RECREATE");
  fpHistFile->cd();
  cout << "==> treeReader01: Opened " << fpHistFile->GetName() << endl;
}

// ----------------------------------------------------------------------
void treeReader01::closeHistFile() {
  cout << "==> treeReader01: Writing " << fpHistFile->GetName() << endl;
  fpHistFile->cd();
  fpHistFile->Write();
  fpHistFile->Close();
  delete fpHistFile;

}

// --------------------------------------------------------------------------------------------------
void treeReader01::readCuts(TString filename, int dump) {
  char  buffer[200];
  fCutFile = filename;
  if (dump) cout << "==> treeReader01: Reading " << fCutFile.Data() << " for cut settings" << endl;
  sprintf(buffer, "%s", fCutFile.Data());
  ifstream is(buffer);
  char CutName[100];
  float CutValue;
  int ok(0);

  TString fn(fCutFile.Data());

  if (dump) {
    cout << "====================================" << endl;
    cout << "==> treeReader01: Cut file  " << fCutFile.Data() << endl;
    cout << "------------------------------------" << endl;
  }

  TH1D *hcuts = new TH1D("hcuts", "", 1000, 0., 1000.);
  hcuts->GetXaxis()->SetBinLabel(1, fn.Data());
  int ibin;

  while (is.getline(buffer, 200, '\n')) {
    ok = 0;
    if (buffer[0] == '#') {continue;}
    if (buffer[0] == '/') {continue;}
    sscanf(buffer, "%s %f", CutName, &CutValue);

    if (!strcmp(CutName, "TYPE")) {
      TYPE = int(CutValue); ok = 1;
      if (dump) cout << "TYPE:           " << TYPE << endl;
    }

    if (!strcmp(CutName, "PTLO")) {
      PTLO = CutValue; ok = 1;
      if (dump) cout << "PTLO:           " << PTLO << " GeV" << endl;
      ibin = 11;
      hcuts->SetBinContent(ibin, PTLO);
      hcuts->GetXaxis()->SetBinLabel(ibin, "p_{T}^{min}(l) [GeV]");
    }

    if (!strcmp(CutName, "PTHI")) {
      PTHI = CutValue; ok = 1;
      if (dump) cout << "PTHI:           " << PTHI << " GeV" << endl;
      ibin = 12;
      hcuts->SetBinContent(ibin, PTHI);
      hcuts->GetXaxis()->SetBinLabel(ibin, "p_{T}^{max}(l) [GeV]");
    }


    if (!ok) cout << "==> treeReader01: ERROR: Don't know about variable " << CutName << endl;
  }

  if (dump)  cout << "------------------------------------" << endl;
}


// ----------------------------------------------------------------------
int treeReader01::loop(int nevents, int start) {
  int nb = 0, maxEvents(0);

  cout << "==> treeReader01: Chain has a total of " << fNentries << " events" << endl;

  // -- Setup for restricted running (not yet foolproof, i.e. bugfree)
  if (nevents < 0) {
    maxEvents = fNentries;
  } else {
    cout << "==> treeReader01: Running over " << nevents << " events" << endl;
    maxEvents = nevents;
  }
  if (start < 0) {
    start = 0;
  } else {
    cout << "==> treeReader01: Starting at event " << start << endl;
    if (maxEvents >  fNentries) {
      cout << "==> treeReader01: Requested to run until event " << maxEvents << ", but will run only to end of chain at ";
      maxEvents = fNentries;
      cout << maxEvents << endl;
    } else {
      cout << "==> treeReader01: Requested to run until event " << maxEvents << endl;
    }
  }

  // -- The main loop
  int step(50000);
  if (maxEvents < 1000000) step = 10000;
  if (maxEvents < 100000)  step = 1000;
  if (maxEvents < 10000)   step = 500;
  if (maxEvents < 1000)    step = 100;

  int treeNumber(0), oldTreeNumber(-1);
  fpChain->GetFile(); // without this, treeNumber initially will be -1.
  for (int jEvent = start; jEvent < maxEvents; ++jEvent) {
    treeNumber = fpChain->GetTreeNumber();
    if (treeNumber != oldTreeNumber) {
      cout << "    " << Form("      %8d", jEvent) << " " << fpChain->GetFile()->GetName() << endl;
      oldTreeNumber = treeNumber;
    }

    if (jEvent%step == 0) cout << Form(" .. Event %8d", jEvent);

    fChainEvent = jEvent;
    nb = fpChain->LoadTree(jEvent);
    fbmc_tid->GetEntry(nb);

    if (jEvent%step == 0) {
      TTimeStamp ts;
      cout  << " (run: " << Form("%8d", fRun)
	    << ", event: " << Form("%10d", fEvt)
	    << ", time now: " << ts.AsString("lc")
	    << ")" << endl;
    }

    eventProcessing();
  }
  return 0;

}
