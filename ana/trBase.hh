#ifndef TRBASE_H
#define TRBASE_H

#include <iostream>
#include <vector>

#include <TROOT.h>
#include <TVector3.h>
#include <TChain.h>
#include <TFile.h>
#include <TTree.h>
#include <TTimeStamp.h>


#define DR      57.29577951

// -- what?
struct header {
  int event;
  int run;
  int type;
  int setup;
};


// -- pixel sensor alignment
struct sensor {
  uint32_t id;
  double vx, vy, vz;
  double rowx, rowy, rowz;
  double colx, coly, colz;
  int nrow, ncol;
  double width, length, thickness, pixelSize;
};


class trBase {
public:
  trBase(TChain *tree, std::string para);
  virtual            ~trBase();

  enum MODE {UNSET, SEGS, FRAMES};

  Long64_t           LoadTree(Long64_t entry);
  virtual void       commonVar();
  virtual void       initBranch(std::string name, int* var);
  virtual void       initBranch(std::string name, float* var);
  virtual void       initBranch(std::string name, double* var);
  virtual void       initBranch(std::string name, std::string** var);
  virtual void       initBranch(std::string name, std::vector<int>** vect);
  virtual void       initBranch(std::string name, std::vector<unsigned int>** vect);
  virtual void       initBranch(std::string name, std::vector<double>** vect);

  virtual void       initMu3e();
  virtual void       initAlignment();
  virtual void       initMu3e_mchits();


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


  int                   fNBranches;
  std::vector<TBranch*> fBranches;


  // -- pixel information in Mu3e tree
  // Nhit: Number of pixel hits
  // hit_pixelid: ID of the pixel hit (includes sensor ID (bit 16 and up) and row and column (lower 16 bits)
  // hit_timestamp: Hit time relative to run start
  // hit_mc_i: Index into the mu3e_mchits tree for looking up truth information
  // hit_mc_n: Number of truth information records in the mu3e_mchits tree for this hit
  int                        fNhit;
  std::vector<unsigned int>  *fhit_pixelid, *fhit_timestamp;
  std::vector<int>           *fhit_mc_i, *fhit_mc_n;

  // -- MC truth information in Mu3e information 
  // Ntrajectories: Number of truth trajectories (particles)
  // traj_ID: Run-unique ID of the particle
  // traj_mother: Run-unique ID of the mother particle of the current particle
  // traj_PID: Particle ID (PDG encoding) of the particle
  // traj_type: Type and source of the particle, see Numbering and naming schemes
  // traj_time: Creation time of the particle relative to the frame start time
  // traj_vx,vy,vz: Creation point of the particle
  // traj_px,py,pz: Momentum vector of the particle at creation
  // traj_fbhid: number of SciFi crossings
  // traj_tlhid: number of passages through tile volume
  // traj_edep_target: Enregy deposited in the target by this particle
  int                        fNtrajectories;
  std::vector<unsigned int>  *ftraj_ID, *ftraj_mother, *ftraj_fbhid, *ftraj_tlhid;
  std::vector<int>           *ftraj_PID, *ftraj_type;
  std::vector<double>        *ftraj_time, *ftraj_vx, *ftraj_vy, *ftraj_vz;
  std::vector<double>        *ftraj_px, *ftraj_py, *ftraj_pz;



  // -- mu3e_mchits tree
  // Linkage information between detector hits and truth trajectories, to be indexed by the _mc_i and _mc_n variables
  // det: Detector that was hit. Encoding 1) pixel, 2) fibre, 3) fibre.mppc 4) tile
  // tid: Unique trajectory ID
  // pdg: Particle type encoded using the PDG scheme
  // hid: Hit number (of corresponding detector) along trajectory, negative if trajectory is radially inwards moving
  // hid_g: Global hit id (pixel hid)
  // edep: Energy deposition of the hit
  // time: Time of the hit, relative to run start
  // -- mu3e_mchits
  TChain                     *fpChain2;
  int                        fdet, ftid, fpdg, fhid, fhid_g;
  double                     fedep, ftime;


  struct header                fHeader;
  std::map<uint32_t, struct sensor> fSensors;

  double                     fWeight;
  std::string                *fRandomState;


  // -- Output histogram/tree pointers
  TTree       *fTree;

  // -- Cut values
  double PTLO, PTHI;
  int TYPE;

};


#endif
