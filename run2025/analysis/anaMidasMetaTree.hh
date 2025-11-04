#ifndef ANA_MIDAS_META_TREE_HH
#define ANA_MIDAS_META_TREE_HH

#include <TTree.h>
#include <TFile.h>
#include <TBranch.h>
#include <string>

#include <TH1D.h>
#include <TH2D.h>
#include <TProfile.h>
#include <map>

#include "mu3ePlotUtils.hh"

class anaMidasMetaTree {
public:
  anaMidasMetaTree(TTree* tree = nullptr);
  virtual ~anaMidasMetaTree() = default;

  void init(TTree* tree);
  Long64_t getEntry(Long64_t entry);
  Long64_t loadTree(Long64_t entry);
  void loop(Long64_t maxEntries = -1);

  void print(int run, int globalChipID);
  void bookHistograms();
  void makePlots();
  void endAnalysis();

  // Branch buffers
  int runNumber{};
  int globalChipID{};
  int linkMask[3]{};
  int linkMatrix[3]{};

  int nlinks{3};
  int abcLinkMask[3]{};
  long long abcLinkErrs[3]{};
  int abcLinkMatrix[3]{};

  long long lvdsErrRate0{};
  long long lvdsErrRate1{};
  long long lvdsErrRate2{};

  int ckdivend{};
  int ckdivend2{};

  int vdacBLPix{};
  int vdacThHigh{};
  int vdacThLow{};

  int biasVNOutPix{};
  int biasVPDAC{};
  int biasVNDcl{};
  int biasVNLVDS{};
  int biasVNLVDSDel{};
  int biasVPDcl{};
  int biasVPTimerDel{};
  int vdacBaseline{};

  std::string fOutputFileName{"anaMidasMetaTree.root"};

  private:
  TTree* fChain{nullptr};
  Long64_t fCurrent{-1};
  mu3ePlotUtils fPlotUtils;

  TBranch *b_runNumber{nullptr}, *b_globalChipID{nullptr};
  TBranch *b_linkMask{nullptr}, *b_linkMatrix{nullptr};
  TBranch *b_nlinks{nullptr}, *b_abcLinkMask{nullptr}, *b_abcLinkErrs{nullptr}, *b_abcLinkMatrix{nullptr};
  TBranch *b_lvdsErrRate0{nullptr}, *b_lvdsErrRate1{nullptr}, *b_lvdsErrRate2{nullptr};
  TBranch *b_ckdivend{nullptr}, *b_ckdivend2{nullptr};
  TBranch *b_vdacBLPix{nullptr}, *b_vdacThHigh{nullptr}, *b_vdacThLow{nullptr};
  TBranch *b_biasVNOutPix{nullptr}, *b_biasVPDAC{nullptr}, *b_biasVNDcl{nullptr};
  TBranch *b_biasVNLVDS{nullptr}, *b_biasVNLVDSDel{nullptr}, *b_biasVPDcl{nullptr}, *b_biasVPTimerDel{nullptr};
  TBranch *b_vdacBaseline{nullptr};

  TFile *fOutputFile{nullptr};
  std::map<std::string, TH1D*> fMapH1;
  std::map<std::string, TH2D*> fMapH2;
  std::map<std::string, TProfile*> fMapTProfile;
};

#endif


