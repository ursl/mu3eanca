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
  std::string makeBLOB(std::map<unsigned int, std::vector<double> >) override;
  std::map<unsigned int, std::vector<double> > decodeBLOB(std::string) override;
  void printBLOB(std::string, int verbosity = 1) override;

  unsigned int getOnline(unsigned int sensor);
  unsigned int getSensor(unsigned int online);

  bool         getNextID(unsigned int &ID);

  std::string  readJson(std::string filename);

  std::string getSchema() override {return fSchema;}

private:
  std::string fPixelCablingMapTag{"pixelcablingmap_"};
  std::string fSchema{"ui_sensor,ui_online"};

  // -- map<sensor, online>
  std::map<unsigned int, unsigned int> fMapConstants;
  std::map<unsigned int, unsigned int>::iterator fMapConstantsIt{fMapConstants.end()};
};

#endif
