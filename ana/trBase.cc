#include "trBase.hh"
#include "trIncludes.hh"

#include <TChainElement.h>

// ----------------------------------------------------------------------
// Run with: see derived classes!
//
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
trBase::trBase(TChain *chain, string treeName): fVerbose(0), fMode(UNSET) {
  cout << "==> trBase: constructor..." << endl;
  if (chain == 0) {
    cout << "You need to pass a chain!" << endl;
  }
  fpChain = chain;
  fChainName = treeName;

  fNentries = fpChain->GetEntries();
  cout << "==> trBase: constructor fpChain: " << fpChain 
       << "/" << fpChain->GetName() << " entries = " << fNentries 
       << endl;

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




// ----------------------------------------------------------------------
int trBase::pixelLayer(uint32_t id) {
  int pixID = pixelID(id);
  
  if (pixID < 231) {
    return 1; 
  } else if (pixID < 1400) {
    return 2; 
  } else if (
             (6152 < pixID && pixID < 6904) 
             || (2056 < pixID && pixID < 2808)
             || (10248 < pixID && pixID < 11000)) {
    return 3; 
  } else if (
             (7175 < pixID && pixID < 8056)
             || (3079 < pixID) && (3960 < pixID) 
             || (11271 < pixID) && (pixID < 12152)) {
    return 4; 
  } 
  return -1;
}


// ----------------------------------------------------------------------
int trBase::trajIdx(uint32_t traj_ID) {
  cout << "  ftraj_ID->size() = " << ftraj_ID->size() << endl;
  for (unsigned int idx = 0; idx < ftraj_ID->size(); ++idx) {
    cout << "  trBase::trajId = " << ftraj_ID->at(idx) << endl;
    if (ftraj_ID->at(idx) == traj_ID) {
      cout << "  -> returning " << idx << endl;
      return idx; 
    }
  }
  return -1;
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


// ----------------------------------------------------------------------
void trBase::initAlignment() {
  TObjArray *fileElements = fpChain->GetListOfFiles();
  TIter next(fileElements);
  TChainElement *chEl(0);
  cout << "Opening the following file to read alignment/sensor" << endl;
  TFile *f(0); 
  while ((chEl=(TChainElement*)next())) {
    f = TFile::Open(chEl->GetTitle());
    break;
  }
  if (f->IsOpen()) {
    TTree *ta = (TTree*)f->Get("alignment/sensors");
    struct sensor a;
    ta->SetBranchAddress("id", &a.id);
    ta->SetBranchAddress("vx", &a.vx);
    ta->SetBranchAddress("vy", &a.vy);
    ta->SetBranchAddress("vz", &a.vz);
    ta->SetBranchAddress("rowx", &a.rowx);
    ta->SetBranchAddress("rowy", &a.rowy);
    ta->SetBranchAddress("rowz", &a.rowz);
    ta->SetBranchAddress("colx", &a.colx);
    ta->SetBranchAddress("coly", &a.coly);
    ta->SetBranchAddress("colz", &a.colz);
    int nbytes(0);
    for (int i = 0; i < ta->GetEntries(); ++i) {
      nbytes += ta->GetEntry(i);
      fSensors.insert({a.id, a});
    }
  }

  if (fVerbose > 0) {
    for (auto it: fSensors) {
      cout << it.second.id 
           << " v = " << it.second.vx << "/" << it.second.vy << "/" << it.second.vz
           << endl;
    }
  }
}


// ----------------------------------------------------------------------
void trBase::initMu3e() {
  cout << "==> trGen::initMu3e() ... " << endl;

  fpChain->SetBranchAddress("Header", &fHeader);
  initBranch("Weight", &fWeight);
  initBranch("RandomState", &fRandomState);
  initBranch("Nhit", &fNhit);

  initBranch("hit_pixelid", &fhit_pixelid);
  initBranch("hit_timestamp", &fhit_timestamp);
  initBranch("hit_mc_i", &fhit_mc_i);
  initBranch("hit_mc_n", &fhit_mc_n);

  initBranch("Ntrajectories", &fNtrajectories);
  initBranch("traj_ID", &ftraj_ID);
  initBranch("traj_mother", &ftraj_mother);
  initBranch("traj_fbhid", &ftraj_fbhid);
  initBranch("traj_tlhid", &ftraj_tlhid);
  initBranch("traj_PID", &ftraj_PID);
  initBranch("traj_type", &ftraj_type);
  initBranch("traj_time", &ftraj_time);
  initBranch("traj_vx", &ftraj_vx);
  initBranch("traj_vy", &ftraj_vy);
  initBranch("traj_vz", &ftraj_vz);
  initBranch("traj_px", &ftraj_px);
  initBranch("traj_py", &ftraj_py);
  initBranch("traj_pz", &ftraj_pz);
}


// ----------------------------------------------------------------------
void trBase::initMu3e_mchits() {
  fpChain2 = new TChain("mu3e_mchits");
  TObjArray *fileElements = fpChain->GetListOfFiles();
  TIter next(fileElements);
  TChainElement *chEl(0);
  cout << "Adding the following files to chain fTree2(\"mu3e_mchits\")" << endl;
  while ((chEl=(TChainElement*)next())) {
    cout << "   " << chEl->GetTitle() << endl;
    fpChain2->Add(chEl->GetTitle());
  }
        
  fpChain2->SetBranchAddress("det", &fdet);
  fpChain2->SetBranchAddress("tid", &ftid);
  fpChain2->SetBranchAddress("pdg", &fpdg);
  fpChain2->SetBranchAddress("hid", &fhid);
  fpChain2->SetBranchAddress("hid_g", &fhid_g);
  fpChain2->SetBranchAddress("edep", &fedep);
  fpChain2->SetBranchAddress("time", &ftime);
}


