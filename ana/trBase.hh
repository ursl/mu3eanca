#ifndef TRBASE_H
#define TRBASE_H

#include <iostream>
#include <vector>

#include <TROOT.h>
#include <TVector3.h>
#include <TLorentzVector.h>
#include <TChain.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TTree.h>
#include <TTimeStamp.h>


#define DR      57.29577951

struct header {
  int event;
  int run;
  int type;
  int setup;
  double weight;
};

class trBase {
public:
  trBase(TChain *tree, std::string para);
  virtual            ~trBase();

  enum MODE {UNSET, FRAMES, MU3E};

  Long64_t           LoadTree(Long64_t entry);
  virtual void       init(std::string treeName);
  virtual void       initFrames();
  virtual void       initMu3e();
  virtual void       initMu3e_mchits();
  virtual void       initBranch(std::string name, int* var);
  virtual void       initBranch(std::string name, float* var);
  virtual void       initBranch(std::string name, double* var);
  virtual void       initBranch(std::string name, std::string** var);
  virtual void       initBranch(std::string name, std::vector<int>** vect);
  virtual void       initBranch(std::string name, std::vector<unsigned int>** vect);
  virtual void       initBranch(std::string name, std::vector<double>** vect);

  virtual void       openHistFile(std::string filename);
  virtual void       closeHistFile();
  virtual void       bookHist();
  virtual void       readCuts(std::string filename, int dump = 1);
  virtual void       printBranches();

  virtual void       startAnalysis();
  virtual void       endAnalysis();
  virtual int        loop(int nevents = 1, int start = -1);
  virtual TFile*     getFile() {return fpChain->GetCurrentFile();}
  virtual void       eventProcessing();
  virtual void       initVariables();
  virtual void       fillHist();
  virtual void       setVerbosity(int f) {std::cout << Form("setVerbosity(%d)", f) << std::endl;  fVerbose = f;}

  // -- study


  int fVerbose;

protected:
  enum MODE   fMode;           // what type of tree?
  int         fCurrent;        // current tree number in chain
  TChain      *fpChain;        // pointer to the analyzed TTree or TChain
  TFile       *fpHistFile;     // for output histograms and reduced trees
  std::string fChainName;      // the name of the chain file
  std::string fCutFile;        // contains file with the cut definitions

  // -- Pre-filled variables
  int          fNentries;      // number of events in chain; filled in trBase::trBase()
  int          fChainEvent;    // current sequential event number in chain; filled in trBase::loop()
  int          fEvt;           // current event number; filled in trBase::loop()
  int          fRun;           // current run number; filled in trBase::loop()



  // -- tree variables: frames
  int                  fn, fn3, fn4, fn6, fn8;
  int                  fgeom_vertex_found, ftrue_vertex_found;
  int                  frunId, feventId;
  float                fweight;
  std::vector<int>     *fmc, *fmc_prime, *fmc_type;
  std::vector<int>     *fmc_pid, *fmc_tid, *fmc_mid;
  std::vector<double>  *fmc_p, *fmc_pt, *fmc_phi, *fmc_lam, *fmc_theta;
  std::vector<double>  *fmc_vx, *fmc_vy, *fmc_vz, *fmc_vr, *fmc_vt, *fmc_t0;
  std::vector<int>     *fnhit, *fhid0, *fsid0;
  std::vector<double>  *fx0, *fy0, *fz0, *ft0, *ft0_err;
  std::vector<double>  *fdt, *fdt_si, *ft0_tl, *ft0_fb, *ft0_si;
  std::vector<double>  *fr, *frerr2, *fp, *fperr2, *fchi2, *ftan01, *flam01, *fn_shared_hits, *fn_shared_segs;

  // -- tree variables: mu3e
  struct header              fHeader;
  double                     fWeight;
  std::string                *fRandomState;
  int                        fNhit;
  std::vector<unsigned int>  *fhit_pixelid, *fhit_timestamp;
  std::vector<int>           *fhit_mc_i, *fhit_mc_n;

  int                        fNtrajectories;
  std::vector<unsigned int>  *ftraj_ID, *ftraj_mother, *ftraj_fbhid, *ftraj_tlhid;
  std::vector<int>           *ftraj_PID, *ftraj_type;
  std::vector<double>        *ftraj_time, *ftraj_vx, *ftraj_vy, *ftraj_vz;
  std::vector<double>        *ftraj_px, *ftraj_py, *ftraj_pz;

  // -- mu3e_mchits
  int                        fdet, ftid, fpdg, fhid, fhid_g;
  double                     fedep, ftime;

  int                   fNBranches;
  std::vector<TBranch*> fBranches;

  std::string fTree2Name;
  TTree       *fTree2;

  // -- Output histogram/tree pointers
  TTree       *fTree;

  bool DBX;

  // -- Cut values
  double PTLO, PTHI;
  int TYPE;

};


#endif
