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

class trBase {
public:
  trBase(TChain *tree, std::string para);
  virtual            ~trBase();

  Long64_t           LoadTree(Long64_t entry);
  virtual void       commonVar();
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

  virtual void       startAnalysis();
  virtual void       endAnalysis();
  virtual int        loop(int nevents = 1, int start = -1);
  virtual TFile*     getFile() {return fpChain->GetCurrentFile();}
  virtual void       eventProcessing();
  virtual void       initVariables();
  virtual void       fillHist();
  virtual void       setVerbosity(int f) {std::cout << Form("setVerbosity(%d)", f) << std::endl;  fVerbose = f;}


  int fVerbose;

protected:
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


  int                   fNBranches;
  std::vector<TBranch*> fBranches;

  // -- Output histogram/tree pointers
  TTree       *fTree;

  bool DBX;

  // -- Cut values
  double PTLO, PTHI;
  int TYPE;

};


#endif
