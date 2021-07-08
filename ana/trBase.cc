#include "trBase.hh"

#include <fstream>
#include <string>

#include "TRandom.h"

#define MMUON 105.658305

using namespace std;


// ----------------------------------------------------------------------
// Run with: see derived classes!
//
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
trBase::trBase(TChain *chain, string treeName) {
  cout << "==> trBase: constructor..." << endl;
  if (chain == 0) {
    cout << "You need to pass a chain!" << endl;
  }
  fpChain = chain;
  fChainName = treeName;

  fNentries = fpChain->GetEntries();
  cout << "==> trBase: constructor fpChain: " << fpChain << "/" << fpChain->GetName() << " entries = " <<   fNentries << endl;

  fNBranches = 0;
  fBranches.resize(1000);
  for (unsigned int it = 0; it < fBranches.size(); ++it) {
    fBranches[it] = 0;
  }

}

// ----------------------------------------------------------------------
void trBase::commonVar() {
  cout << "trBase::commonVar() wrong class " << endl;
}

// ----------------------------------------------------------------------
void trBase::startAnalysis() {
  cout << "trBase::startAnalysis() wrong class " << endl;
}

// ----------------------------------------------------------------------
void trBase::endAnalysis() {
  cout << "trBase::endAnalysis() wrong class " << endl;
}


// ----------------------------------------------------------------------
void trBase::eventProcessing() {
  cout << "trBase::eventProcessing() wrong class " << endl;
}



// ----------------------------------------------------------------------
void trBase::fillHist() {
  cout << "trBase::fillHist() wrong class " << endl;

}

// ----------------------------------------------------------------------
void trBase::bookHist() {
  cout << "==> trBase: bookHist> " << endl;

  // -- Reduced Tree
  fTree = new TTree("events", "events");
  fTree->Branch("run",      &fRun,       "run/I");
  fTree->Branch("evt",      &fEvt,       "evt/I");


}


// ----------------------------------------------------------------------
void trBase::initVariables() {
  cout << "trBase::initVariables() wrong class " << endl;
}




// ======================================================================
// -- Below is the icc material
// ======================================================================

// ----------------------------------------------------------------------
void trBase::initBranch(string name, int* pvar) {
  int i = fNBranches;
  fpChain->SetBranchAddress(name.c_str(), pvar, &(fBranches[i]));
  cout << "initBranch(" << name << ", int*), fNBranches = "
       << i
       << " &(fBranches[" << i << "]) = " << &(fBranches[i])
       << ", fBranches[" << i << "] = " << (fBranches[i])
       << endl;
  ++fNBranches;
}


// ----------------------------------------------------------------------
void trBase::initBranch(string name, float* pvar) {
  int i = fNBranches;
  fpChain->SetBranchAddress(name.c_str(), pvar, &(fBranches[i]));
  cout << "initBranch(" << name << ", float*), fNBranches = "
       << i
       << " &(fBranches[" << i << "]) = " << &(fBranches[i])
       << ", fBranches[" << i << "] = " << (fBranches[i])
       << endl;
  ++fNBranches;
}

// ----------------------------------------------------------------------
void trBase::initBranch(string name, double* pvar) {
  int i = fNBranches;
  fpChain->SetBranchAddress(name.c_str(), pvar, &(fBranches[i]));
  cout << "initBranch(" << name << ", double*), fNBranches = "
       << i
       << " &(fBranches[" << i << "]) = " << &(fBranches[i])
       << ", fBranches[" << i << "] = " << (fBranches[i])
       << endl;
  ++fNBranches;
}

// ----------------------------------------------------------------------
void trBase::initBranch(string name, string** pvar) {
  cout << "initBranch(" << name << "),  pvar = " << pvar << endl;
  fpChain->SetBranchAddress(name.c_str(), pvar);
}

// ----------------------------------------------------------------------
void trBase::initBranch(string name, vector<int>** pvect) {
  int i = fNBranches;
  fpChain->SetBranchAddress(name.c_str(), pvect, &(fBranches[i]));
  cout << "initBranch(" << name << ", vector<int>), fNBranches = "
       << i
       << " &(fBranches[" << i << "]) = " << &(fBranches[i])
       << ", fBranches[" << i << "] = " << (fBranches[i])
       << endl;
  ++fNBranches;
}

// ----------------------------------------------------------------------
void trBase::initBranch(string name, vector<unsigned int>** pvect) {
  int i = fNBranches;
  fpChain->SetBranchAddress(name.c_str(), pvect, &(fBranches[i]));
  cout << "initBranch(" << name << ", vector<unsigned int>), fNBranches = "
       << i
       << " &(fBranches[i]) = " << &(fBranches[i])
       << ", fBranches[i] = " << (fBranches[i])
       << endl;
  ++fNBranches;
}

// ----------------------------------------------------------------------
void trBase::initBranch(string name, vector<double>** pvect) {
  int i = fNBranches;
  fpChain->SetBranchAddress(name.c_str(), pvect, &(fBranches[i]));
  cout << "initBranch(" << name << ", vector<double>), fNBranches = "
       << i
       << " &(fBranches[" << i << "]) = " << &(fBranches[i])
       << ", fBranches[" << i << "] = " << fBranches[i]
       << endl;
  ++fNBranches;
}


// ----------------------------------------------------------------------
trBase::~trBase() {
  cout << "==> trBase: destructor ..." << endl;
  if (!fpChain) return;
  delete fpChain->GetCurrentFile();
}

// ----------------------------------------------------------------------
void trBase::openHistFile(string filename) {
  fpHistFile = new TFile(filename.c_str(), "RECREATE");
  fpHistFile->cd();
  cout << "==> trBase: Opened " << fpHistFile->GetName() << endl;
}

// ----------------------------------------------------------------------
void trBase::closeHistFile() {
  cout << "==> trBase: Writing " << fpHistFile->GetName() << endl;
  fpHistFile->cd();
  //  fpHistFile->Write();
  fpHistFile->Close();
  delete fpHistFile;

}

// --------------------------------------------------------------------------------------------------
void trBase::readCuts(string filename, int dump) {
  char  buffer[200];
  fCutFile = filename;
  if (dump) cout << "==> trBase: Reading " << fCutFile << " for cut settings" << endl;
  sprintf(buffer, "%s", fCutFile.c_str());
  ifstream is(buffer);
  char CutName[100];
  float CutValue;
  int ok(0);

  string fn(fCutFile);

  if (dump) {
    cout << "====================================" << endl;
    cout << "==> trBase: Cut file  " << fCutFile << endl;
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


    if (!ok) cout << "==> trBase: ERROR: Don't know about variable " << CutName << endl;
  }

  if (dump)  cout << "------------------------------------" << endl;
}


// ----------------------------------------------------------------------
int trBase::loop(int nevents, int start) {
  int maxEvents(0);

  cout << "==> trBase: Chain " << fChainName << " has a total of " << fNentries << " events" << endl;

  // -- Setup for restricted running (not yet foolproof, i.e. bugfree)
  if (nevents < 0) {
    maxEvents = fNentries;
  } else {
    cout << "==> trBase: Running over " << nevents << " events" << endl;
    maxEvents = nevents;
  }
  if (start < 0) {
    start = 0;
  } else {
    cout << "==> trBase: Starting at event " << start << endl;
    if (maxEvents >  fNentries) {
      cout << "==> trBase: Requested to run until event " << maxEvents << ", but will run only to end of chain at ";
      maxEvents = fNentries;
      cout << maxEvents << endl;
    } else {
      cout << "==> trBase: Requested to run until event " << maxEvents << endl;
    }
  }

  // -- The main loop
  int step(50000);
  if (maxEvents < 1000000) step = 10000;
  if (maxEvents < 100000)  step = 1000;
  if (maxEvents < 10000)   step = 500;
  if (maxEvents < 1000)    step = 100;

  Long64_t nbytes = 0, nb = 0;
  for (Long64_t jentry = 0; jentry < maxEvents; ++jentry) {
    Long64_t ientry = LoadTree(jentry);
    if (ientry < 0) break;
    nb = fpChain->GetEntry(jentry);   nbytes += nb;

    // -- fill common variables (fEvt, fRun, etc)
    fChainEvent = jentry;
    this->commonVar();

    if (jentry%step == 0) {
      TTimeStamp ts;
      cout  << " (runId: " << Form("%8d", fRun)
            << ", eventId: " << Form("%8d", fEvt)
            << ", chainEvt: " << Form("%10d", fChainEvent)
            << ", time now: " << ts.AsString("lc")
            << ")" << endl;
    }


    eventProcessing();
  }


  // Long64_t nb(0);
  // int treeNumber(-1), oldTreeNumber(-1);
  // fpChain->GetFile(); // without this, treeNumber initially will be -1.
  // int mode(-1);


  // for (int jEvent = start; jEvent < maxEvents; ++jEvent) {
  //   treeNumber = fpChain->GetTreeNumber();
  //   if (treeNumber != oldTreeNumber) {
  //     cout << "    " << Form("      %8d", jEvent)
  // 	   << " with tree ->" << fChainName
  // 	   << "<- from "  << fpChain->GetFile()->GetName();
  //     if ("mu3e_mchits" == fTree2Name) {
  // 	fTree2 = (TTree*)fpChain->GetFile()->Get(fTree2Name.c_str());
  // 	initMu3e_mchits();
  // 	cout << " and tree2 ->" << fTree2Name << "<-";
  //     }
  //     cout << endl;
  //     oldTreeNumber = treeNumber;
  //   }

  //   if (jEvent%step == 0) cout << Form(" .. chain event %8d", jEvent);

  //   fChainEvent = jEvent;

  //   // -- complete tree reading:
  //   nb += fpChain->GetEvent(jEvent);

  //   if (1 == mode) {
  //     fRun = frunId;
  //     fEvt = feventId;
  //   } else if (2 == mode) {
  //     fRun = fHeader.run;
  //     fEvt = fHeader.event;
  //   }

  //   eventProcessing();
  // }

  return 0;

}

// ----------------------------------------------------------------------
Long64_t trBase::LoadTree(Long64_t entry) {
  // Set the environment to read one entry
  if (!fpChain) return -5;
  Long64_t centry = fpChain->LoadTree(entry);
  if (centry < 0) return centry;
  if (fpChain->GetTreeNumber() != fCurrent) {
    fCurrent = fpChain->GetTreeNumber();
  }
  return centry;
}
