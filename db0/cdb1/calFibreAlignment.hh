#ifndef CALFIBREALIGNMENT_h
#define CALFIBREALIGNMENT_h

#include "calAbs.hh"

#include <string>
#include <map>

// ----------------------------------------------------------------------
// fibres alignment class 
// ----------------------------------------------------------------------
class calFibreAlignment : public calAbs {
public:
  
  calFibreAlignment() = default;
  calFibreAlignment(cdbAbs *db);
  calFibreAlignment(cdbAbs *db, std::string tag);
  ~calFibreAlignment();

  // -- direct accessors
  uint32_t id(uint32_t id) {return fMapConstants[id].id;}
  double cx(uint32_t id) {return fMapConstants[id].cx;}
  double cy(uint32_t id) {return fMapConstants[id].cy;}
  double cz(uint32_t id) {return fMapConstants[id].cz;}
  double fx(uint32_t id) {return fMapConstants[id].fx;}
  double fy(uint32_t id) {return fMapConstants[id].fy;}
  double fz(uint32_t id) {return fMapConstants[id].fz;}
  bool   round(uint32_t id) {return fMapConstants[id].round;}
  bool   square(uint32_t id) {return fMapConstants[id].square;}
  double diameter(uint32_t id) {return fMapConstants[id].diameter;}

  std::string getName() override {return fFibresAlignmentTag;}
  void        calculate(std::string hash) override;

  std::string makeBLOB() override;
  std::string makeBLOB(std::map<unsigned int, std::vector<double> >) override;
  std::map<unsigned int, std::vector<double> > decodeBLOB(std::string) override;
  void printBLOB(std::string, int verbosity = 1) override;

  bool        getNextID(uint32_t &ID);

  std::string readCsv(std::string filename);
  
private:
  std::string fFibresAlignmentTag{"fibrealignment_"};

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
  
};

#endif
