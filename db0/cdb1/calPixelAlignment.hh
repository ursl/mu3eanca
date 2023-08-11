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
  uint32_t id(uint32_t id) {return fMapConstants[id].id;}
  double vx(uint32_t id) {return fMapConstants[id].vx;}
  double vy(uint32_t id) {return fMapConstants[id].vy;}
  double vz(uint32_t id) {return fMapConstants[id].vz;}
  double rowx(uint32_t id) {return fMapConstants[id].rowx;}
  double rowy(uint32_t id) {return fMapConstants[id].rowy;}
  double rowz(uint32_t id) {return fMapConstants[id].rowz;}
  double colx(uint32_t id) {return fMapConstants[id].colx;}
  double coly(uint32_t id) {return fMapConstants[id].coly;}
  double colz(uint32_t id) {return fMapConstants[id].colz;}
  int    nrow(uint32_t id) {return fMapConstants[id].nrow;}
  int    ncol(uint32_t id) {return fMapConstants[id].ncol;}
  double width(uint32_t id) {return fMapConstants[id].width;}
  double length(uint32_t id) {return fMapConstants[id].length;}
  double thickness(uint32_t id) {return fMapConstants[id].thickness;}
  double pixelSize(uint32_t id) {return fMapConstants[id].pixelSize;}

  // void setVxAddr(double *ptr) {fpvx = ptr;}

  // void setVars(uint32_t id) {*fpvx = fMapConstants[id].vx;}
  // void fillVars(uint32_t id) {*fpvx = fMapConstants[id].vx;}
  
  std::string getName() override {return fPixelAlignmentTag;}
  void        calculate(std::string hash) override;
  std::string makeBLOB(std::map<unsigned int, std::vector<double> >) override {return "FIXME";}

  bool        getNextID(uint32_t &ID);
  
private:
  std::string fPixelAlignmentTag{"pixelalignment_"};

  // -- local and private
  struct constants {
    uint32_t id; 
    double vx, vy, vz;
    double rowx, rowy, rowz;
    double colx, coly, colz; 
    int nrow, ncol;
    double width, length, thickness, pixelSize;
  };

  std::map<uint32_t, constants> fMapConstants;
  std::map<uint32_t, constants>::iterator fMapConstantsIt{fMapConstants.end()};
  
};

#endif
