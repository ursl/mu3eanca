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
  calPixelAlignment(cdbAbs *db);
  calPixelAlignment(cdbAbs *db, std::string tag);
  ~calPixelAlignment();

  // -- direct accessors
  int id(int id) {return fMapConstants[id].id;}
  double vx(int id) {return fMapConstants[id].vx;}
  double vy(int id) {return fMapConstants[id].vy;}
  double vz(int id) {return fMapConstants[id].vz;}
  double rowx(int id) {return fMapConstants[id].rowx;}
  double rowy(int id) {return fMapConstants[id].rowy;}
  double rowz(int id) {return fMapConstants[id].rowz;}
  double colx(int id) {return fMapConstants[id].colx;}
  double coly(int id) {return fMapConstants[id].coly;}
  double colz(int id) {return fMapConstants[id].colz;}
  int    nrow(int id) {return fMapConstants[id].nrow;}
  int    ncol(int id) {return fMapConstants[id].ncol;}
  double width(int id) {return fMapConstants[id].width;}
  double length(int id) {return fMapConstants[id].length;}
  double thickness(int id) {return fMapConstants[id].thickness;}
  double pixelSize(int id) {return fMapConstants[id].pixelSize;}

  void setVxAddr(double *ptr) {fpvx = ptr;}

  void setVars(int id) {*fpvx = fMapConstants[id].vx;}
  void fillVars(int id) {*fpvx = fMapConstants[id].vx;}
  
  std::string getName() override {return fPixelAlignmentTag;}
  void        calculate() override;

private:
  std::string fPixelAlignmentTag{"pixelalignment_"};

  // -- local and private
  struct constants {
    int id; 
    double vx, vy, vz;
    double rowx, rowy, rowz;
    double colx, coly, colz; 
    int nrow, ncol;
    double width, length, thickness, pixelSize;
  };

  std::map<int, constants> fMapConstants;

  double *fpvx;
};

#endif
