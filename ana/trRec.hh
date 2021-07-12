#ifndef TRREC_H
#define TRREC_H

#include <iostream>
#include <vector>

#include "trBase.hh"

class trRec: public trBase {
public:
  trRec(TChain *tree, std::string para);
  ~trRec();

  void       commonVar();
  void       initFrames();
  void       initSegs();
  void       printFramesBranches();
  void       printSegsBranches();

  void       bookHist();
  void       readCuts(std::string filename, int dump = 1);

  void       startAnalysis();
  void       endAnalysis();
  void       eventProcessing();
  void       initVariables();
  void       fillHist();
  void       closeHistFile();

  // -- study
  void       recStudy();

protected:

  // -- tree variables: frames
  int                  fn, fn3, fn4, fn6, fn8;
  int                  fgeom_vertex_found, ftrue_vertex_found;
  int                  frunId, feventId;
  float                fweight;
  std::vector<int>     *fmc, *fmc_prime, *fmc_type;
  std::vector<int>     *fmc_pid, *fmc_tid, *fmc_mid;
  std::vector<double>  *fmc_p, *fmc_pt, *fmc_phi, *fmc_lam, *fmc_theta;
  std::vector<double>  *fmc_vx, *fmc_vy, *fmc_vz, *fmc_vr, *fmc_vt, *fmc_t0;
  std::vector<int>     *fnhit, *fhid0, *fsid0;
  std::vector<double>  *fx0, *fy0, *fz0, *ft0, *ft0_err;
  std::vector<double>  *fdt, *fdt_si, *ft0_tl, *ft0_fb, *ft0_si;
  std::vector<double>  *fr, *frerr2, *fp, *fperr2, *fchi2, *ftan01, *flam01, *fn_shared_hits, *fn_shared_segs;

  // -- tree variables: segs
  static constexpr int SEGSN = 10;
  std::map<std::string, int> fSegsInt;
  std::map<std::string, float> fSegsFloat;
  std::map<std::string, float*> fSegsFloatV;

  // -- Cut values
  double PTLO, PTHI;
  int TYPE;


  struct redTreeData {
    double p;
  } fRTD;

};


#endif
