#ifndef TREEREADER01_H
#define TREEREADER01_H

#include <iostream>

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

class treeReader01 {
public:
  treeReader01(TChain *tree, std::string para);
  virtual      ~treeReader01();
  virtual void       init(std::string treeName);
  virtual void       initFrames();
  virtual void       initBranch(std::string name, int* var);
  virtual void       initBranch(std::string name, float* var);
  virtual void       initBranch(std::string name, std::vector<int>** vect);
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

  TChain      *fpChain;        // pointer to the analyzed TTree or TChain
  TFile       *fpHistFile;     // for output histograms and reduced trees
  std::string fChainFileName; // the name of the chain file
  std::string fCutFile;       // contains file with the cut definitions

  // -- Pre-filled variables
  int          fNentries;      // number of events in chain; filled in treeReader01::treeReader01()
  int          fChainEvent;    // current sequential event number in chain; filled in treeReader01::loop()
  int          fEvt;           // current event number; filled in treeReader01::loop()
  int          fRun;           // current run number; filled in treeReader01::loop()



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
  std::vector<double>  *fp, *fperr2, *fchi2, *ftan01, *flam01, *fn_shared_hits, *fn_shared_segs;
  // TBranch              *fbmc, *fbmc_prime, *fbmc_type;
  // TBranch	       *fbmc_pid, *fbmc_tid, *fbmc_mid;
  // TBranch	       *fbmc_p, *fbmc_pt, *fbmc_phi, *fbmc_lam, *fbmc_theta;
  // TBranch	       *fbmc_vx, *fbmc_vy, *fbmc_vz, *fbmc_vr, *fbmc_vt, *fbmc_t0;
  // TBranch	       *fbnhit, *fbhid0, *fbsid0;
  // TBranch	       *fbx0, *fby0, *fbz0, *fbt0, *fbt0_err;
  // TBranch	       *fbdt, *fbdt_si, *fbt0_tl, *fbt0_fb, *fbt0_si;
  // TBranch	       *fbp, *fbperr2, *fbchi2, *fbtan01, *fblam01, *fbn_shared_hits, *fbn_shared_segs;

  int                   fNBranches;
  std::vector<TBranch*> fBranches;


  // -- Histogram pointers
  TTree       *fTree;

  bool DBX;

  // -- Cut values
  double PTLO, PTHI;
  int TYPE;

};


#endif
