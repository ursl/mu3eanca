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
treeReader01::treeReader01(TChain *chain, string para) {
  cout << "==> treeReader01: constructor..." << endl;
  if (chain == 0) {
    cout << "You need to pass a chain!" << endl;
  }
  fpChain = chain;
  cout << "==> treeReader01: constructor fpChain: " << fpChain << "/" << endl;
  fpChain->ls();
  fNentries = chain->GetEntries();

  fNBranches = 0;
  fBranches.resize(1000);
  for (unsigned int it = 0; it < fBranches.size(); ++it) {
    fBranches[it] = 0;
  }
  cout << " &feventId = " << &feventId << " initialized with fBranches.size() = " << fBranches.size() << endl;

  init(para);
}

// ----------------------------------------------------------------------
void treeReader01::init(string treeName) {
  cout << "==> treeReader01: init..." << endl;

  if ("frames" == treeName) initFrames();

  initVariables();
}


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

  printBranches();

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
void treeReader01::initFrames() {
  cout << "==> treeReader01::initFrames() ... " << endl;

  // -- NB: The following lines are essential. Else there will be a crash when compiler does not do default zeroing!
  fmc = fmc_prime = fmc_type = 0;
  fmc_pid = fmc_tid = fmc_mid = 0;
  fmc_p = fmc_pt = fmc_phi = fmc_lam = fmc_theta = 0;
  fmc_vx = fmc_vy = fmc_vz = fmc_vr = fmc_vt = fmc_t0 = 0;
  fnhit = fhid0 = fsid0 = 0;
  fx0 = fy0 = fz0 = ft0 = ft0_err = 0;
  fdt = fdt_si = ft0_tl = ft0_fb = ft0_si = 0;
  fr = frerr2 = fp = fperr2 = fchi2 = ftan01 = flam01 = fn_shared_hits = fn_shared_segs = 0;

  initBranch("n", &fn);
  initBranch("n3", &fn3);
  initBranch("n4", &fn4);
  initBranch("n6", &fn6);
  initBranch("n8", &fn8);

  initBranch("geom_vertex_found", &fgeom_vertex_found);
  initBranch("true_vertex_found", &ftrue_vertex_found);

  initBranch("eventId", &feventId);
  initBranch("runId", &frunId);
  initBranch("weight", &fweight);

  initBranch("mc", &fmc);
  initBranch("mc_prime", &fmc_prime);
  initBranch("mc_type", &fmc_type);

  initBranch("mc_pid", &fmc_pid);
  initBranch("mc_tid", &fmc_tid);
  initBranch("mc_mid", &fmc_mid);

  initBranch("mc_p", &fmc_p);
  initBranch("mc_pt", &fmc_pt);
  initBranch("mc_phi", &fmc_phi);
  initBranch("mc_lam", &fmc_lam);
  initBranch("mc_theta", &fmc_theta);

  initBranch("mc_vx", &fmc_vx);
  initBranch("mc_vy", &fmc_vy);
  initBranch("mc_vz", &fmc_vz);
  initBranch("mc_vr", &fmc_vr);
  initBranch("mc_vt", &fmc_vt);
  initBranch("mc_t0", &fmc_t0);

  initBranch("nhit", &fnhit);
  initBranch("hid0", &fhid0);
  initBranch("sid0", &fsid0);

  initBranch("x0", &fx0);
  initBranch("y0", &fy0);
  initBranch("z0", &fz0);
  initBranch("t0", &ft0);
  initBranch("t0_err", &ft0_err);

  initBranch("dt", &fdt);
  initBranch("dt_si", &fdt_si);
  initBranch("t0_tl", &ft0_tl);
  initBranch("t0_fb", &ft0_fb);
  initBranch("t0_si", &ft0_si);

  initBranch("r", &fr);
  initBranch("rerr2", &frerr2);
  initBranch("p", &fp);
  initBranch("perr2", &fperr2);
  initBranch("chi2", &fchi2);
  initBranch("tan01", &ftan01);
  initBranch("lam01", &flam01);
  initBranch("n_shared_hits", &fn_shared_hits);
  initBranch("n_shared_segs", &fn_shared_segs);


  if (0) {
    cout << "fNBranches = " << fNBranches << endl;
    for (unsigned int it = 0; it < fNBranches; ++it) {
      cout << Form("%3d ", it) << fBranches[it]->GetName() << " at " << fBranches[it] << endl;
    }
  }
}

// ----------------------------------------------------------------------
void treeReader01::initBranch(string name, int* pvar) {
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
void treeReader01::initBranch(string name, float* pvar) {
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
void treeReader01::initBranch(string name, vector<int>** pvect) {
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
void treeReader01::initBranch(string name, vector<double>** pvect) {
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
void treeReader01::printBranches() {
  cout << "evt: " << fChainEvent
       << " eventId: " << feventId
       << " run: "  << frunId
       << " w8: " << frunId
       << " geom_vertex_found: " << fgeom_vertex_found
       << " true_vertex_found: " << ftrue_vertex_found
       << " w8: " << frunId
       << endl;
  cout << ": fmc_tid->size() = " << fmc_tid->size() << ":  ";
  for (unsigned int i = 0; i < fmc_tid->size(); ++i) {
    cout << fmc_tid->at(i);
    if (i < fmc_tid->size() - 1) cout << ", ";
  }
  cout << endl;

  cout << ": fmc_type->size() = " << fmc_type->size() << ":  ";
  for (unsigned int i = 0; i < fmc_type->size(); ++i) {
    cout << fmc_type->at(i);
    if (i < fmc_type->size() - 1) cout << ", ";
  }
  cout << endl;

  cout << ": fmc_pt->size() = " << fmc_pt->size() << ":  ";
  for (unsigned int i = 0; i < fmc_pt->size(); ++i) {
    cout << fmc_pt->at(i) << "/"
	 << fmc_p->at(i)*TMath::Sin(fmc_theta->at(i));
    if (i < fmc_pt->size() - 1) cout << ", ";
  }
  cout << endl;

  cout << ": fmc_v->size() = " << fmc_vx->size() << ":  ";
  for (unsigned int i = 0; i < fmc_vx->size(); ++i) {
    cout << fmc_vx->at(i) << "/"
	 << fmc_vy->at(i) << "/"
	 << fmc_vz->at(i);
    if (i < fmc_vx->size() - 1) cout << ", ";
  }
  cout << endl;

}



// ----------------------------------------------------------------------
treeReader01::~treeReader01() {
  cout << "==> treeReader01: destructor ..." << endl;
  if (!fpChain) return;
  delete fpChain->GetCurrentFile();
}

// ----------------------------------------------------------------------
void treeReader01::openHistFile(string filename) {
  fpHistFile = new TFile(filename.c_str(), "RECREATE");
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
void treeReader01::readCuts(string filename, int dump) {
  char  buffer[200];
  fCutFile = filename;
  if (dump) cout << "==> treeReader01: Reading " << fCutFile << " for cut settings" << endl;
  sprintf(buffer, "%s", fCutFile.c_str());
  ifstream is(buffer);
  char CutName[100];
  float CutValue;
  int ok(0);

  string fn(fCutFile);

  if (dump) {
    cout << "====================================" << endl;
    cout << "==> treeReader01: Cut file  " << fCutFile << endl;
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


    if (!ok) cout << "==> treeReader01: ERROR: Don't know about variable " << CutName << endl;
  }

  if (dump)  cout << "------------------------------------" << endl;
}


// ----------------------------------------------------------------------
int treeReader01::loop(int nevents, int start) {
  int maxEvents(0);

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


  Long64_t nentries = fpChain->GetEntriesFast();

  Long64_t nbytes = 0, nb = 0;
  for (Long64_t jentry = 0; jentry < nentries; ++jentry) {
    Long64_t ientry = LoadTree(jentry);
    if (ientry < 0) break;
    nb = fpChain->GetEntry(jentry);   nbytes += nb;
    // if (Cut(ientry) < 0) continue;
    if (jentry%step == 0) {
      TTimeStamp ts;
      cout  << " (run: " << Form("%8d", fRun)
	    << ", event: " << Form("%10d", fEvt)
	    << ", time now: " << ts.AsString("lc")
	    << ")" << endl;
    }

    eventProcessing();
  }

  // // OLD OLD
  // int treeNumber(0), oldTreeNumber(-1);
  // fpChain->GetFile(); // without this, treeNumber initially will be -1.
  // for (Long64_t jEvent = start; jEvent < maxEvents; ++jEvent) {
  //   Long64_t ientry = LoadTree(jEvent);
  //   if (ientry < 0) break;
  //   treeNumber = fpChain->GetTreeNumber();
  //   if (treeNumber != oldTreeNumber) {
  //     cout << "    " << Form("      %8d", jEvent) << " " << fpChain->GetFile()->GetName() << endl;
  //     oldTreeNumber = treeNumber;
  //   }

  //   //    if (jEvent%step == 0) cout << Form(" .. Event %8d", jEvent);

  //   fChainEvent = jEvent;

  //   // -- complete tree reading:
  //   //    nb += fpChain->GetEvent(jEvent);
  //   nb += fpChain->GetEntry(jEvent);

  //   if (jEvent%step == 0) {
  //     TTimeStamp ts;
  //     cout  << " (run: " << Form("%8d", fRun)
  // 	    << ", event: " << Form("%10d", fEvt)
  // 	    << ", time now: " << ts.AsString("lc")
  // 	    << ")" << endl;
  //   }

  //   eventProcessing();
  // }
  return 0;

}

Long64_t treeReader01::LoadTree(Long64_t entry) {
  // Set the environment to read one entry
  if (!fpChain) return -5;
  Long64_t centry = fpChain->LoadTree(entry);
  if (centry < 0) return centry;
  if (fpChain->GetTreeNumber() != fCurrent) {
    fCurrent = fpChain->GetTreeNumber();
  }
  return centry;
}
