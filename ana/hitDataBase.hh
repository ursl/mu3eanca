#ifndef HITDATABASE_H
#define HITDATABASE_H

#include <iostream>
#include <vector>
#include <sstream>

#include <TROOT.h>
#include <TBranch.h>
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

  virtual void        setupTree();
  virtual void        openHistFile(std::string filename);
  virtual void        closeHistFile();
  virtual void        bookHist(int i);
  virtual void        readCuts(std::string filename, int dump = 1);

  virtual void        startAnalysis();
  virtual void        endAnalysis();
  virtual int         loop(int nevents = 1, int start = -1);
  virtual TFile*      getFile() {return fpChain->GetCurrentFile();}
  virtual void        eventProcessing();
  virtual void        initVariables();
  virtual void        fillHist();
  virtual void        setVerbosity(int f) {std::cout << Form("setVerbosity(%d)", f) << std::endl;  fVerbose = f;}

  std::pair<int, int> colrowFromIdx(int idx);
  int                 idxFromColRow(int col, int row);

  int  fVerbose;

protected:
  int         fCurrent;        // current tree number in chain
  TChain      *fpChain;        // pointer to the analyzed TTree or TChain
  TFile       *fpHistFile;     // for output histograms and reduced trees
  std::string fChainName;      // the name of the chain file
  std::string fCutFile;        // contains file with the cut definitions

  // -- Pre-filled variables
  int          fNentries;      // number of events in chain; filled in hitDataBase::hitDataBase()
  int          fChainEvent;    // current sequential event number in chain; filled in hitDataBase::loop()
  int          fEvt;           // current event number; filled in derived classes in hitDataBase::loop()
  int          fRun;           // current run number; filled in derived classes in hitDataBase::loop()

  // -- Output histogram/tree pointers
  TTree       *fTree;


  std::map<int, std::vector<std::pair<int, int> > > fChipNoisyPixels; 
  std::map<int, int> fBadChips; 


  std::vector<unsigned int>  *fv_runID, *fv_MIDASEventID, *fv_ts2, *fv_hitTime,
    *fv_headerTime, *fv_headerTimeMajor, *fv_subHeaderTime, *fv_trigger,
    *fv_isInCluster;
  std::vector<unsigned char>  *fv_col, *fv_row, *fv_chipID, *fv_fpgaID, *fv_chipIDRaw,
    *fv_tot, *fv_isMUSR, *fv_hitType, *fv_layer;
  TBranch *fb_runID, *fb_col, *fb_row, *fb_chipID, *fb_MIDASEventID,
    *fb_ts2, *fb_hitTime,
    *fb_headerTime, *fb_headerTimeMajor, *fb_subHeaderTime, *fb_trigger,
    *fb_isInCluster,
    *fb_fpgaID, *fb_chipIDRaw, *fb_tot, *fb_isMUSR, *fb_hitType, *fb_layer;

};


#endif
