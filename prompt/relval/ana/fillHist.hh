#ifndef FILLHIST_HH
#define FILLHIST_HH

#include <string>
#include <map>
#include <vector>
#include <TH1D.h>
#include <TTree.h>
#include <TFile.h>

class fillHist {
  public:
  fillHist(const std::string &infile, const std::string &outfileName);
  ~fillHist();
  void setupTree(const std::string &treeName);
  void bookHist(std::string mode);
  void run(int nevents = -1);
  bool checkVectorSizes();
  
  private:
  TTree *fTree;
  TFile *fInFile,*fOutFile;
  std::string fTreeName;
  std::string fOutFileName;
  int fNevents;
  std::map<std::string, TH1*> fHistograms;
  

  // -- these refer to the tree/chain passed in as "tree" in c'tor
  virtual void       initBranch(std::string name, int* var);
  virtual void       initBranch(std::string name, float* var);
  virtual void       initBranch(std::string name, double* var);
  virtual void       initBranch(std::string name, std::string** var);
  virtual void       initBranch(std::string name, std::vector<int>** vect);
  virtual void       initBranch(std::string name, std::vector<unsigned int>** vect);
  virtual void       initBranch(std::string name, std::vector<double>** vect);

  // -- tree variables: frames
  ULong64_t            frunId, fframeId;
  unsigned int         fflags;

  std::vector<double>  *fx0, *fy0, *fz0, *ft0, *ft0_err, *ft0_rms, *ft0_tl, *ft0_fb, *ft0_si;
  std::vector<double>  *ft0_tl_rms, *ft0_fb_rms, *ft0_si_rms;
  std::vector<double>  *fr, *frerr2, *fp, *fperr2, *fchi2, *ftan01, *flam01;
  std::vector<int>     *fnhit, *fttype, *fn_shared_hits, *fn_shared_segs;
  std::vector<unsigned int> *fsid0;
  std::vector<int>     *ffarm_status;
 
  int                  fn, fn4, fn6, fn8;
  std::vector<int>     *fmc, *fmc_prime, *fmc_type;
  std::vector<int>     *fmc_pid, *fmc_tid, *fmc_mid;
  std::vector<double>  *fmc_weight, *fmc_p, *fmc_pt, *fmc_phi, *fmc_lam, *fmc_theta;
  std::vector<double>  *fmc_vx, *fmc_vy, *fmc_vz, *fmc_vr, *fmc_vt, *fmc_t0;
  
};

#endif