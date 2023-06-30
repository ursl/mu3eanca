#ifndef CALFIBRESLALIGNMENT_h
#define CALFIBRESALIGNMENT_h

#include "calAbs.hh"

#include <string>
#include <map>

// ----------------------------------------------------------------------
// fibres alignment class 
// ----------------------------------------------------------------------
class calFibresAlignment : public calAbs {
public:
  
  calFibresAlignment() = default;
  calFibresAlignment(cdbAbs *db);
  calFibresAlignment(cdbAbs *db, std::string tag);
  ~calFibresAlignment();

  // -- direct accessors
  uint32_t id(uint32_t id) {return fMapConstants[id].id;}
  double cx(uint32_t id) {return fMapConstants[id].cx;}
  double cy(uint32_t id) {return fMapConstants[id].cy;}
  double cz(uint32_t id) {return fMapConstants[id].cz;}
  double fx(uint32_t id) {return fMapConstants[id].fx;}
  double fy(uint32_t id) {return fMapConstants[id].fy;}
  double fz(uint32_t id) {return fMapConstants[id].fz;}

  bool round(uint32_t id) {return fMapConstants[id].round;}
  bool square(uint32_t id) {return fMapConstants[id].square;}
  double diameter(uint32_t id) {return fMapConstants[id].diameter;}

  void setVxAddr(double *ptr) {fpvx = ptr;}

  void setVars(uint32_t id) {*fpvx = fMapConstants[id].vx;}
  void fillVars(uint32_t id) {*fpvx = fMapConstants[id].vx;}
  
  std::string getName() override {return fFibresAlignmentTag;}
  void        calculate(std::string hash) override;

  bool        getNextID(uint32_t &ID);
  
private:
  std::string fFibresAlignmentTag{"fibresalignment_"};

  // -- local and private
  struct constants {
    uint32_t id; 
    double cx, cy, cz;
    double fx, fy, fz;
    bool round, square;
    double diameter;
  };

  std::map<uint32_t, constants> fMapConstants;
  std::map<uint32_t, constants>::iterator fMapConstantsIt{fMapConstants.end()};
  
  double *fpvx;
};

#endif
