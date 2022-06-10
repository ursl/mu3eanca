#ifndef HITDATABASE_H
#define HITDATABASE_H

#include <iostream>
#include <vector>

#include <TROOT.h>
#include <TVector3.h>
#include <TChain.h>
#include <TFile.h>
#include <TTree.h>
#include <TTimeStamp.h>


#define DR      57.29577951

class hitDataBase {
public:
  hitDataBase(TChain *tree, std::string para);
  virtual            ~hitDataBase();

  enum MODE {UNSET, NOISE, PIXEL};

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


  int  fVerbose;
  MODE fMode;

protected:
  int         fCurrent;        // current tree number in chain
  TChain      *fpChain;        // pointer to the analyzed TTree or TChain
  TFile       *fpHistFile;     // for output histograms and reduced trees
  std::string fChainName;      // the name of the chain file
  std::string fCutFile;        // contains file with the cut definitions

  // -- Pre-filled variables
  int          fNentries;      // number of events in chain; filled in trBase::trBase()
  int          fChainEvent;    // current sequential event number in chain; filled in trBase::loop()
  int          fEvt;           // current event number; filled in derived classes in commonVar()
  int          fRun;           // current run number; filled in derived classes in commonVar()

  // -- Output histogram/tree pointers
  TTree       *fTree;

};


#endif
