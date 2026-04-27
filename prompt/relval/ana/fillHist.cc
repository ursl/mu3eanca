#include "fillHist.hh"

#include <iostream> 
#include <string>

using namespace std;


// ----------------------------------------------------------------------
fillHist::fillHist(const std::string &treeName, const std::string &outfileName) {
  fTreeName = treeName;
  fOutFileName = outfileName;
}


// ----------------------------------------------------------------------
fillHist::~fillHist() {

  for (auto &h : fHistograms) {
    h.second->SetDirectory(fOutFile);
    h.second->Write();
    delete h.second;
  }
  
  if (fOutFile) {
    delete fOutFile;
  }
}


// ----------------------------------------------------------------------
void fillHist::bookHist(string mode) {
  fOutFile = TFile::Open(fOutFileName.c_str(), "RECREATE");
  if ("relval" == mode) {
    fHistograms["hp"] = new TH1D("hp", "p", 100, -100., 100.);
    fHistograms["hp20"] = new TH1D("hp20", "p (|p| > 20)", 100, -100., 100.);
  }
}


// ----------------------------------------------------------------------
void fillHist::run(int nevents) {
  fNevents = nevents;
  if (fNevents < 0) fNevents = fTree->GetEntries();
  for (int i = 0; i < fNevents; ++i) {
    fTree->GetEntry(i);
    cout << "fillHist::run() i = " << i << " fp->size() = " << fp->size() << endl;
    for (unsigned int j = 0; j < fp->size(); ++j) {
      fHistograms["hp"]->Fill(fp->at(j));
      if (TMath::Abs(fp->at(j)) > 20.) fHistograms["hp20"]->Fill(fp->at(j));
    }
  }
}



// ----------------------------------------------------------------------
void fillHist::setupTree(std::string infile) {
  fTree = TFile::Open(infile.c_str())->Get<TTree>(fTreeName.c_str());
  
  // -- NB: The following lines are essential. Else there will be a crash when compiler does not do default zeroing!
  fx0 = fy0 = fz0 = ft0 = ft0_err = ft0_tl = ft0_fb = ft0_si = 0;
  ft0_tl_rms = ft0_fb_rms = ft0_si_rms = 0;
  fr = frerr2 = fp = fperr2 = fchi2 = ftan01 = flam01 = 0;
  fnhit = fttype = fn_shared_hits = fn_shared_segs = ffarm_status = 0;
  fmc = fmc_prime = fmc_type = 0;
  fmc_pid = fmc_tid = fmc_mid = 0;
  fmc_weight = fmc_p = fmc_pt = fmc_phi = fmc_lam = fmc_theta = 0;
  fmc_vx = fmc_vy = fmc_vz = fmc_vr = fmc_vt = fmc_t0 = 0;
  
  initBranch("x0", &fx0);
  initBranch("y0", &fy0);
  initBranch("z0", &fz0);
  initBranch("t0", &ft0);
  initBranch("t0_err", &ft0_err);
  initBranch("t0_tl", &ft0_tl);
  initBranch("t0_fb", &ft0_fb);
  initBranch("t0_si", &ft0_si);
  initBranch("t0_tl_rms", &ft0_tl_rms);
  initBranch("t0_fb_rms", &ft0_fb_rms);
  initBranch("t0_si_rms", &ft0_si_rms);
  initBranch("r", &fr);
  initBranch("rerr2", &frerr2);
  initBranch("p", &fp);
  initBranch("perr2", &fperr2);
  initBranch("chi2", &fchi2);
  initBranch("tan01", &ftan01);
  initBranch("lam01", &flam01);
}


// ----------------------------------------------------------------------
void fillHist::initBranch(string name, int* pvar) {
  fTree->SetBranchAddress(name.c_str(), pvar);
}

// ----------------------------------------------------------------------
void fillHist::initBranch(string name, float* pvar) {
  fTree->SetBranchAddress(name.c_str(), pvar);
}

// ----------------------------------------------------------------------
void fillHist::initBranch(string name, double* pvar) {
  fTree->SetBranchAddress(name.c_str(), pvar);
}

// ----------------------------------------------------------------------
void fillHist::initBranch(string name, string** pvar) {
  cout << "initBranch(" << name << "),  pvar = " << pvar << endl;
  fTree->SetBranchAddress(name.c_str(), pvar);
}

// ----------------------------------------------------------------------
void fillHist::initBranch(string name, vector<int>** pvect) {
  fTree->SetBranchAddress(name.c_str(), pvect);
}
// ----------------------------------------------------------------------
void fillHist::initBranch(string name, vector<unsigned int>** pvect) {
  fTree->SetBranchAddress(name.c_str(), pvect);
}

// ----------------------------------------------------------------------
void fillHist::initBranch(string name, vector<double>** pvect) {
  fTree->SetBranchAddress(name.c_str(), pvect);
}
