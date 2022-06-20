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

// -- index for histogram maps
struct hID {
  hID(int R = 0, int C = -1, std::string N = "nada"): run(R), chipID(C), name(N){};
  void setRunChip(int R, int C) {
    run = R;
    chipID = C;
  }
  // -- required for std::map
  bool operator<(const hID& h2) const {
    if (run < h2.run) {
      return true;
    } else if (chipID < h2.chipID) {
      return true;
    } else if (name < h2.name) {
      return true;
    }
    return false;      
  }

  int run, chipID;
  std::string name;
};

class hitDataBase {
public:
  hitDataBase(TChain *tree, std::string para);
  virtual ~hitDataBase();

  virtual void         openHistFile(std::string filename);
  virtual void         closeHistFile();
  virtual void         bookHist(int i);
  virtual void         readCuts(std::string filename, int dump = 1);

  virtual void         startAnalysis();
  virtual void         endAnalysis();
  virtual void         runEndAnalysis(int oldrun);
  virtual int          loop(int nevents = 1, int start = -1, bool readMaskFiles = false);
  virtual void         eventProcessing();
  virtual void         initVariables();
  virtual void         fillHist();

  void                 setupTree();
  TFile*               getFile() {return fpChain->GetCurrentFile();}
  void                 setVerbosity(int f) {std::cout << Form("setVerbosity(%d)", f) << std::endl;  fVerbose = f;}
  void                 setOutputDirectoryName(std::string oname) {fOutputDirectoryName = oname; }
  std::pair<int, int>  colrowFromIdx(int idx);
  int                  idxFromColRow(int col, int row);

  bool                 validNoise(const std::vector<uint8_t> &v);
  bool                 badLVDS(const std::vector<uint8_t> &v);
  bool                 unclean(const std::vector<uint8_t> &v, int maxNoise);
  bool                 skipChip(int idx);

  std::vector<uint8_t> readMaskFile(std::string filename);
  void                 fillNoisyPixels(int chipID, std::vector<uint8_t> &vnoise,
                                       std::map<int, std::vector<std::pair<int, int> > > &map1);
  void                 readNoiseMaskFiles(int runnumber, std::string dir = "nmf");


  int  fVerbose;

protected:
  int         fCurrent;        // current tree number in chain
  TChain      *fpChain;        // pointer to the analyzed TTree or TChain
  TFile       *fpHistFile;     // for output histograms and reduced trees
  std::string fChainName;      // the name of the chain file
  std::string fCutFile;        // contains file with the cut definitions
  std::string fOutputDirectoryName, fSuffix; 

  // -- Pre-filled variables
  int          fNentries;      // number of events in chain; filled in hitDataBase::hitDataBase()
  int          fChainEvent;    // current sequential event number in chain; filled in hitDataBase::loop()
  int          fEvt;           // current event number; filled in derived classes in hitDataBase::loop()
  int          fRun;           // current run number; filled in derived classes in hitDataBase::loop()

  // -- Output histogram/tree pointers
  TTree       *fTree;

  int fChipID, fcol, frow, ftot, ftot2, fqual, flayer; 

  
  // -- indexed with pair<run, chipID>
  std::map<struct hID, TH1*>      fChipHistograms;

  // -- index with chipID
  std::map<int, std::vector<std::pair<int, int> > > fChipNoisyPixels; 
  std::map<int, int> fChipQuality; 

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

std::ostream & operator << (std::ostream& , const hID&);

#endif
