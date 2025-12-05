#ifndef CALPIXELCABLINGMAP_h
#define CALPIXELCABLINGMAP_h

#include "calAbs.hh"

#include <string>
#include <map>

// ----------------------------------------------------------------------
// pixel cabling map class
// ----------------------------------------------------------------------
class calPixelCablingMap : public calAbs {
public:

  calPixelCablingMap() = default;
  calPixelCablingMap(cdbAbs *db);
  calPixelCablingMap(cdbAbs *db, std::string tag);
  ~calPixelCablingMap();

  std::string getName() override {return fPixelCablingMapTag;}
  void        calculate(std::string hash) override;

  std::string makeBLOB() override;
  void printBLOB(std::string, int verbosity = 1) override;
  std::string printBLOBString(std::string blob, int verbosity = 1) override;

  unsigned int getOnline(unsigned int sensor);
  unsigned int getSensor(unsigned int online);

  bool         getNextID(unsigned int &ID);

  std::string  readJson(std::string filename);

  std::string getSchema() override {return fSchema;}

private:
  std::string fPixelCablingMapTag{"pixelcablingmap_"};
  std::string fSchema{"ui_sensor,ui_offsetA,ui_offsetB,ui_offsetC,ui_offsetM"};


  struct constants {
    unsigned int sensor;
    unsigned int offsetA;
    unsigned int offsetB;
    unsigned int offsetC;
    unsigned int offsetM;
  };
  // -- map<sensor, constants>
  std::map<unsigned int, constants> fMapConstants;
  std::map<unsigned int, constants>::iterator fMapConstantsIt{fMapConstants.end()};
};

#endif