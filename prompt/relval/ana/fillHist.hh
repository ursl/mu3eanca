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
  void setupTrees();
  void bookHist(std::string annotation);
  void run();
  bool checkVectorSizes();
  int  getVtxL0Ladder(int sid0);
  
  private:
  struct TreeData {
    std::string name;
    TTree *tree;
    int nEvents;

    ULong64_t runId, frameId;
    unsigned int flags;

    std::vector<double>  *x0, *y0, *z0, *t0, *t0_err, *t0_rms, *t0_tl, *t0_fb, *t0_si;
    std::vector<double>  *t0_tl_rms, *t0_fb_rms, *t0_si_rms;
    std::vector<double>  *r, *rerr2, *p, *perr2, *chi2, *tan01, *lam01;
    std::vector<int>     *nhit, *ttype, *n_shared_hits, *n_shared_segs;
    std::vector<unsigned int> *sid0;
    std::vector<int>     *farm_status;

    int                  n, n4, n6, n8;
    std::vector<int>     *mc, *mc_prime, *mc_type;
    std::vector<int>     *mc_pid, *mc_tid, *mc_mid;
    std::vector<double>  *mc_weight, *mc_p, *mc_pt, *mc_phi, *mc_lam, *mc_theta;
    std::vector<double>  *mc_vx, *mc_vy, *mc_vz, *mc_vr, *mc_vt, *mc_t0;

    // -- print track
    void printTrack(int idx);

   // -- base
   bool baseTrackSelection(int idx);

    // -- reco
    bool goodReconstructedTrack(int idx, int trkType);
    // -- reco+sim
    bool goodReconstructibleTrack(int idx, int trkType);
  };

 std::vector<int> fTrackTypes;
 std::map<int, int> fTrackTypeCounts;


  TFile *fInFile,*fOutFile;
  std::string fOutFileName;

  std::map<std::string, TH1*> fHistograms;
  std::map<std::string, std::string> fConfigs;
  TreeData fFrames;
  TreeData fMcFrames;
  
  void resetBranches(TreeData &b);
  void bindTreeBranches(TreeData &data);
  virtual void       initBranch(TTree *tree, std::string name, int* var);
  virtual void       initBranch(TTree *tree, std::string name, ULong64_t* var);
  virtual void       initBranch(TTree *tree, std::string name, float* var);
  virtual void       initBranch(TTree *tree, std::string name, double* var);
  virtual void       initBranch(TTree *tree, std::string name, std::string** var);
  virtual void       initBranch(TTree *tree, std::string name, std::vector<int>** vect);
  virtual void       initBranch(TTree *tree, std::string name, std::vector<unsigned int>** vect);
  virtual void       initBranch(TTree *tree, std::string name, std::vector<double>** vect);
  bool checkVectorSizes(const TreeData &b);
};

#endif