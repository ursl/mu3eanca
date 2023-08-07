#ifndef CALMPPCLALIGNMENT_h
#define CALMPPCALIGNMENT_h

#include "calAbs.hh"

#include <string>
#include <map>

// ----------------------------------------------------------------------
// mppc alignment class 
// ----------------------------------------------------------------------
class calMppcAlignment : public calAbs {
public:
  
  calMppcAlignment() = default;
  calMppcAlignment(cdbAbs *db);
  calMppcAlignment(cdbAbs *db, std::string tag);
  ~calMppcAlignment();

  // -- direct accessors
  uint32_t mppc(uint32_t id) {return fMapConstants[id].mppc;}
  double vx(uint32_t id) {return fMapConstants[id].vx;}
  double vy(uint32_t id) {return fMapConstants[id].vy;}
  double vz(uint32_t id) {return fMapConstants[id].vz;}
  double colx(uint32_t id) {return fMapConstants[id].colx;}
  double coly(uint32_t id) {return fMapConstants[id].coly;}
  double colz(uint32_t id) {return fMapConstants[id].colz;}

  int ncol(uint32_t id) {return fMapConstants[id].ncol;}

  std::string getName() override {return fMppcAlignmentTag;}
  void        calculate(std::string hash) override;
  std::string makeBLOB(std::map<int, std::vector<double> >) {std::cout << "FIXME" << std::endl; }

  bool        getNextID(uint32_t &ID);
  
private:
  std::string fMppcAlignmentTag{"mppcalignment_"};

  // -- local and private
  struct constants {
    uint32_t mppc; 
    double vx, vy, vz;
    double colx, coly, colz;
    int ncol;
  };

  std::map<uint32_t, constants> fMapConstants;
  std::map<uint32_t, constants>::iterator fMapConstantsIt{fMapConstants.end()};
  
  double *fpvx;
};

#endif
