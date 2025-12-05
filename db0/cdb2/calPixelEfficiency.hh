#ifndef CALPIXELEFFICIENCY_h
#define CALPIXELEFFICIENCY_h

#include "calAbs.hh"

#include <string>
#include <map>

// ----------------------------------------------------------------------
// pixel efficiency class (one efficiency per chip)
// based on the code by Haris Avudaiyappan Murugan
// ----------------------------------------------------------------------
class calPixelEfficiency : public calAbs {
public:

  calPixelEfficiency() = default;
  calPixelEfficiency(cdbAbs *db);
  calPixelEfficiency(cdbAbs *db, std::string tag);
  ~calPixelEfficiency();

  // -- direct accessors
  uint32_t id(uint32_t id) {return fMapConstants[id].id;}
  double efficiency(uint32_t id) {return fMapConstants[id].efficiency;}

  std::string getName() override {return fPixelEfficiencyTag;}
  void        calculate(std::string hash) override;

  std::string makeBLOB() override;
  void printBLOB(std::string s, int verbosity = 1) override;

  void readCsv(std::string filename);
  void writeCsv(std::string filename);

  void        resetIterator() {fMapConstantsIt = fMapConstants.begin();}

  std::string getSchema() override {return fSchema;}

private:
  std::string fPixelEfficiencyTag{"pixelefficiency_"};

  // -- local and private
  struct constants {
    uint32_t id;
    double efficiency;
  };

  std::string fSchema{"ui_id,efficiency"};

  std::map<uint32_t, constants> fMapConstants;
  std::map<uint32_t, constants>::iterator fMapConstantsIt{fMapConstants.end()};

};

#endif