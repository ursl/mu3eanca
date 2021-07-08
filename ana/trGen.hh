#ifndef TRGEN_H
#define TRGEN_H

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

#include "trBase.hh"

struct header {
  int event;
  int run;
  int type;
  int setup;
  double weight;
};

class trGen: public trBase {
public:
  trGen(TChain *tree, std::string para);
  ~trGen();

  void       commonVar();
  void       initMu3e();
  void       initMu3e_mchits();
  void       printBranches();

  void       bookHist();
  void       readCuts(std::string filename, int dump = 1);

  void       startAnalysis();
  void       endAnalysis();
  void       eventProcessing();
  void       initVariables();
  void       fillHist();
  void       closeHistFile();

  // -- study
  void       genStudy();


protected:

  struct header              fHeader;
  double                     fWeight;
  std::string                *fRandomState;
  int                        fNhit;
  std::vector<unsigned int>  *fhit_pixelid, *fhit_timestamp;
  std::vector<int>           *fhit_mc_i, *fhit_mc_n;

  int                        fNtrajectories;
  std::vector<unsigned int>  *ftraj_ID, *ftraj_mother, *ftraj_fbhid, *ftraj_tlhid;
  std::vector<int>           *ftraj_PID, *ftraj_type;
  std::vector<double>        *ftraj_time, *ftraj_vx, *ftraj_vy, *ftraj_vz;
  std::vector<double>        *ftraj_px, *ftraj_py, *ftraj_pz;

  // -- mu3e_mchits
  TTree                      *fTree2;
  int                        fdet, ftid, fpdg, fhid, fhid_g;
  double                     fedep, ftime;

  // -- Cut values
  double PTLO, PTHI;
  int TYPE;

};


#endif
