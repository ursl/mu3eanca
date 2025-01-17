#ifndef TRGEN_H
#define TRGEN_H

#include <iostream>
#include <vector>

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

  // -- studies
  void       genStudy();
  void       overlapHitsInVertex();

protected:

  struct header              fHeader;
  double                     fWeight;
  std::string                *fRandomState;

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

  // -- Cut values
  double PTLO, PTHI;
  int TYPE;

};


#endif
