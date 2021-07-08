#include "trBase.hh"

#include <fstream>
#include <string>

#include <TRandom.h>
#include "trGen.hh"

#include "util/massesMeV.hh"

using namespace std;

// ----------------------------------------------------------------------
// Run with: bin/runTreeReader -t mu3e -f data/mu3e_run_000779.root -D results/
// ----------------------------------------------------------------------


// Mu3eEvent.cc:
// if(ptype >= 0 && dtype >= 0) trajType = 10 * dtype + ptype;

// Mu3eTrajectory.h::Mu3eTrackInfo
// * 0     - gamma (photon)
// * 1,2   - e^+,-
// * 3,4   - mu^+,-
// * 5,6   - pi^+,-
// * 7     - nu_e/nu_mu/nu_tau

// * Decay (origin) type:
// * 1     - michel
// * 2     - radiative
// * 3     - internal conversion
// * 4-8   - qed (ioni, phot, compt, conv)
// * 9     - signal
// * 10    - familon signal
// * 11-13 - decay with dark photon




// ----------------------------------------------------------------------
trGen::trGen(TChain *chain, string treeName) : trBase(chain, treeName) {
  cout << "==> trGen: constructor..." << endl;

  initMu3e();
  initVariables();

}


// ----------------------------------------------------------------------
void trGen::commonVar() {
  fRun = fHeader.run;
  fEvt = fHeader.event;
}


// ----------------------------------------------------------------------
void trGen::startAnalysis() {
  cout << "trGen: startAnalysis: ..." << endl;
}

// ----------------------------------------------------------------------
void trGen::endAnalysis() {
  cout << "trGen: endAnalysis: ..." << endl;
}

// ----------------------------------------------------------------------
void trGen::closeHistFile() {
  cout << "==> trGen::closeHistFile() Writing " << fpHistFile->GetName() << endl;
  fpHistFile->cd();
  fpHistFile->Write();
  fpHistFile->Close();
  delete fpHistFile;
}


// ----------------------------------------------------------------------
void trGen::eventProcessing() {
  initVariables();

  // -- generic debug output
  if (fVerbose > 9) {
    printBranches();
  }

  fillHist();
  michelSpectrum();


}


// ----------------------------------------------------------------------
void trGen::michelSpectrum() {

  TH1D *h1 = (TH1D*)fpHistFile->Get("hmichel");
  TLorentzVector p4;
  for (unsigned int i = 0; i < fNtrajectories; ++i) {
    if ((-11 == ftraj_PID->at(i)) && (11 == ftraj_type->at(i))) {
      p4.SetXYZM(ftraj_px->at(i), ftraj_py->at(i), ftraj_pz->at(i), MMUON);
      h1->Fill(p4.Rho());
    }
  }
}

// ----------------------------------------------------------------------
void trGen::fillHist() {
  TH1D *hpx = (TH1D*)fpHistFile->Get("hpx");

  for (unsigned int i = 0; i < ftraj_px->size(); ++i) {
    hpx->Fill(ftraj_px->at(i));
  }

  fTree->Fill();
}

// ----------------------------------------------------------------------
void trGen::bookHist() {
  trBase::bookHist();
  cout << "==> trGen: bookHist> " << endl;

  new TH1D("hpx", "hpx", 100, -100., 100.);
  new TH1D("hmichel", "hmichel", 60, 0., 60.);
  new TH2D("vmichel", "vmichel (r vs. z)", 100, -200., 200., 100, -200., 200.);

}


// ----------------------------------------------------------------------
void trGen::initVariables() {
  //  cout << "trBase: initVariables: for run = " << fRun << "/evt = " << fEvt << endl;

}




// ----------------------------------------------------------------------
void trGen::initMu3e() {
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
void trGen::initMu3e_mchits() {
  fTree2->SetBranchAddress("det", &fdet);
  fTree2->SetBranchAddress("tid", &ftid);
  fTree2->SetBranchAddress("pdg", &fpdg);
  fTree2->SetBranchAddress("hid", &fhid);
  fTree2->SetBranchAddress("hid_g", &fhid_g);
  fTree2->SetBranchAddress("edep", &fedep);
  fTree2->SetBranchAddress("time", &ftime);
}



// ----------------------------------------------------------------------
void trGen::printBranches() {

  cout << "----------------------------------------------------------------------" << endl;
  cout << "mu3e evt: " << fChainEvent
       << " event: " << fHeader.event
       << " run: "  << fHeader.run
       << " type: " << fHeader.type
       << ": fNtrajectories = " << fNtrajectories << ":  "
       << ": fNhit = " << fNhit << ":  "
       << endl;
  for (unsigned int i = 0; i < ftraj_ID->size(); ++i) {
    cout << Form("trj %2d", i)
	 << Form(" ID = %7d", ftraj_ID->at(i))
	 << Form(" PID = %+4d", ftraj_PID->at(i))
	 << Form(" type = %3d", ftraj_type->at(i))
	 << Form(" mother ID = %7d", ftraj_mother->at(i))
	 << Form(" vz = %+9.3f", ftraj_vz->at(i))
	 << Form(" vr = %+8.3f", TMath::Sqrt(ftraj_vx->at(i)*ftraj_vx->at(i) + ftraj_vy->at(i)*ftraj_vy->at(i)))
	 << " time = " << ftraj_time->at(i)
	 << endl;
  }
  for (unsigned int i = 0; i < fNhit; ++i) {
    cout << Form("hit %3d", i)
	 << Form(" pxhitid = %4d", fhit_pixelid->at(i))
	 << Form(" timestamp = %4d", fhit_timestamp->at(i))
	 << Form(" hit_mc_i = %4d", fhit_mc_i->at(i))
	 << Form(" hit_mc_n = %4d", fhit_mc_n->at(i))
	 << endl;
  }
  cout << "----------------------------------------------------------------------" << endl;
}




// ----------------------------------------------------------------------
trGen::~trGen() {
  cout << "==> trGen: destructor ..." << endl;
  if (!fpChain) return;
  delete fpChain->GetCurrentFile();
}


// --------------------------------------------------------------------------------------------------
void trGen::readCuts(string filename, int dump) {
  char  buffer[200];
  fCutFile = filename;
  if (dump) cout << "==> trGen: Reading " << fCutFile << " for cut settings" << endl;
  sprintf(buffer, "%s", fCutFile.c_str());
  ifstream is(buffer);
  char CutName[100];
  float CutValue;
  int ok(0);

  string fn(fCutFile);

  if (dump) {
    cout << "====================================" << endl;
    cout << "==> trGen: Cut file  " << fCutFile << endl;
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
