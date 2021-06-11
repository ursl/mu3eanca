#ifndef TREEREADER01_H
#define TREEREADER01_H

#include <iostream>

#include <TROOT.h>
#include <TString.h>
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
  treeReader01(TChain *tree, TString para);
  virtual      ~treeReader01();
  virtual void       init(TString treeName);

  virtual void       openHistFile(TString filename);
  virtual void       closeHistFile();
  virtual void       bookHist();
  virtual void       readCuts(TString filename, int dump = 1);

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
  TString      fChainFileName; // the name of the chain file
  TString      fCutFile;       // contains file with the cut definitions

  // -- Pre-filled variables
  int          fNentries;      // number of events in chain; filled in treeReader01::treeReader01()
  int          fChainEvent;    // current sequential event number in chain; filled in treeReader01::loop()
  int          fEvt;           // current event number; filled in treeReader01::loop()
  int          fRun;           // current run number; filled in treeReader01::loop()



  // -- tree variables
  int                  feventId, frunId;
  std::vector<int>     *fpmc_tid;
  std::vector<double>  *fpmc_pt;
  TBranch              *fbmc_tid, *fbmc_pt;

  // -- Histogram pointers
  TTree       *fTree;

  bool DBX;

  // -- Cut values
  double PTLO, PTHI;
  int TYPE;

};


#endif
