#ifndef CALPIXELALIGNMENT_h
#define CALPIXELALIGNMENT_h

#include "calAbs.hh"

#include <string>
#include <map>

// ----------------------------------------------------------------------
// pixel alignment class 
// ----------------------------------------------------------------------

class calPixelAlignment : public calAbs {
public:
  
  calPixelAlignment() = default;
  calPixelAlignment(cdb *db);
  calPixelAlignment(cdb *db, std::string tag);
  ~calPixelAlignment();

  // -- direct accessors
  int id(int id) {return fMapConstants[id].id;}
  int vx(int id) {return fMapConstants[id].vx;}
  int vy(int id) {return fMapConstants[id].vy;}
  int vz(int id) {return fMapConstants[id].vz;}
  int rowx(int id) {return fMapConstants[id].rowx;}
  int rowy(int id) {return fMapConstants[id].rowy;}
  int rowz(int id) {return fMapConstants[id].rowz;}
  int colx(int id) {return fMapConstants[id].colx;}
  int coly(int id) {return fMapConstants[id].coly;}
  int colz(int id) {return fMapConstants[id].colz;}

  void setVxAddr(double *ptr) {fpvx = ptr;}
  void fillVars(int id) {*fpvx = fMapConstants[id].vx;}
  
  std::string getName() override {return fPixelAlignmentTag;}
  void        calculate() override;

private:
  std::string fPixelAlignmentTag{"pixelalignment_"};
  // https://bitbucket.org/mu3e/mu3e/src/c6989561a0ca8b294aa3b1699ae5e7053ee3acff/mu3eTrirec/src/SiDet.cpp#lines-337:344
  // auto& sensor = sensors.emplace(std::piecewise_construct,
  //   std::forward_as_tuple(id),
  //   std::forward_as_tuple(id,
  //       make_float3(vx, vy, vz),
  //       make_float3(rowx, rowy, rowz),
  //       make_float3(colx, coly, colz)
  //  )).first->second;

  struct constants {
    int id; 
    double vx, vy, vz;
    double rowx, rowy, rowz;
    double colx, coly, colz; 
  };

  std::map<int, constants> fMapConstants;

  double *fpvx;
};

#endif
