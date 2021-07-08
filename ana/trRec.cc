#include "trBase.hh"

#include <fstream>
#include <string>

#include <TRandom.h>
#include "trRec.hh"

#define MMUON 105.658305

using namespace std;


// ----------------------------------------------------------------------
// Run with: bin/runTreeReader01 -t frames -f data/mu3e_trirec_000779.root -D results/
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
trRec::trRec(TChain *chain, string treeName) : trBase(chain, treeName) {
  cout << "==> trRec: constructor..." << endl;

  initFrames();
  initVariables();

}

// ----------------------------------------------------------------------
void trRec::commonVar() {
  fRun = frunId;
  fEvt = feventId;
}

// ----------------------------------------------------------------------
void trRec::startAnalysis() {
  cout << "trRec: startAnalysis: ..." << endl;
  DBX = true;
}

// ----------------------------------------------------------------------
void trRec::endAnalysis() {
  cout << "trRec: endAnalysis: ..." << endl;
}

// ----------------------------------------------------------------------
void trRec::closeHistFile() {
  cout << "==> trRec::closeHistFile() Writing " << fpHistFile->GetName() << endl;
  fpHistFile->cd();
  fpHistFile->Write();
  fpHistFile->Close();
  delete fpHistFile;
}


// ----------------------------------------------------------------------
void trRec::eventProcessing() {
  initVariables();

  //  printBranches();
  fillHist();

  // -- generic rudimentary analysis
  if (DBX) {
  }

}



// ----------------------------------------------------------------------
void trRec::fillHist() {
  TH1D *h1 = (TH1D*)fpHistFile->Get("hp");
  for (unsigned int i = 0; i < fp->size(); ++i) {
    h1->Fill(fp->at(i));
  }
}

// ----------------------------------------------------------------------
void trRec::bookHist() {
  trBase::bookHist();
  cout << "==> trRec: bookHist> " << endl;

  new TH1D("hp", "hp", 100, -100., 100.);



}


// ----------------------------------------------------------------------
void trRec::initVariables() {
  //  cout << "trBase: initVariables: for run = " << fRun << "/evt = " << fEvt << endl;

}


// ----------------------------------------------------------------------
void trRec::printBranches() {

  cout << "frames evt: " << fChainEvent
       << " eventId: " << fEvt
       << " run: "  << fRun
       << " geom_vertex_found: " << fgeom_vertex_found
       << " true_vertex_found: " << ftrue_vertex_found
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
trRec::~trRec() {
  cout << "==> trRec: destructor ..." << endl;
  if (!fpChain) return;
  delete fpChain->GetCurrentFile();
}


// ----------------------------------------------------------------------
void trRec::initFrames() {
  cout << "==> trRec::initFrames() ... " << endl;
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
}



// --------------------------------------------------------------------------------------------------
void trRec::readCuts(string filename, int dump) {
  char  buffer[200];
  fCutFile = filename;
  if (dump) cout << "==> trRec: Reading " << fCutFile << " for cut settings" << endl;
  sprintf(buffer, "%s", fCutFile.c_str());
  ifstream is(buffer);
  char CutName[100];
  float CutValue;
  int ok(0);

  string fn(fCutFile);

  if (dump) {
    cout << "====================================" << endl;
    cout << "==> trRec: Cut file  " << fCutFile << endl;
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
