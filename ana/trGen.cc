#include "trBase.hh"

#include <fstream>
#include <string>

#include <TRandom.h>
#include "trGen.hh"

#define MMUON 105.658305

using namespace std;


// ----------------------------------------------------------------------
// Run with: ./runTreeReader01 -c chains/bg-test -D root
//           ./runTreeReader01 -f test.root
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
trGen::trGen(TChain *chain, string treeName) : trBase(chain, treeName) {
  cout << "==> trGen: constructor..." << endl;

  initMu3e();
  initVariables();

}

// ----------------------------------------------------------------------
void trGen::init(string treeName) {
  cout << "==> trGen: init..." << endl;

}


// ----------------------------------------------------------------------
void trGen::startAnalysis() {
  cout << "trGen: startAnalysis: ..." << endl;
  DBX = true;
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

  //  printBranches();
  fillHist();

  // -- generic rudimentary analysis
  if (DBX) {
  }

}



// ----------------------------------------------------------------------
void trGen::fillHist() {
  TH1D *hpx = (TH1D*)fpHistFile->Get("hpx");

  for (unsigned int i = 0; i < ftraj_px->size(); ++i) {
    hpx->Fill(ftraj_px->at(i));
  }
}

// ----------------------------------------------------------------------
void trGen::bookHist() {
  cout << "==> trGen: bookHist> " << endl;

  new TH1D("hpx", "hpx", 100, -100., 100.);

  // -- Reduced Tree
  fTree = new TTree("events", "events");
  fTree->Branch("run",      &fRun,       "run/I");
  fTree->Branch("evt",      &fEvt,       "evt/I");


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

  cout << "mu3e evt: " << fChainEvent
       << " event: " << fHeader.event
       << " run: "  << fHeader.run
       << " type: " << fHeader.type
       << " RandomState: " << fRandomState->c_str()
       << endl;
  cout << ": ftraj_ID->size() = " << ftraj_ID->size() << ":  ";
  for (unsigned int i = 0; i < ftraj_ID->size(); ++i) {
    cout << ftraj_ID->at(i)
	 << "(" << ftraj_PID->at(i)
	 << "/" << ftraj_type->at(i)
	 << "/" << ftraj_mother->at(i)
	 << ")";
    if (i < ftraj_ID->size() - 1) cout << ", ";
  }
  cout << endl;

  cout << ": fmc_p->size() = " << fmc_p->size() << ":  ";
  for (unsigned int i = 0; i < fmc_p->size(); ++i) {
    cout << fmc_p->at(i) ;
    if (i < fmc_p->size() - 1) cout << ", ";
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
