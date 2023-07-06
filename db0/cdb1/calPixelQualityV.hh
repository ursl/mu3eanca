#ifndef CALPIXELQUALITY_h
#define CALPIXELQUALITY_h

#include "calAbs.hh"

#include <string>
#include <map>

// ----------------------------------------------------------------------
// pixel quality class 
// ----------------------------------------------------------------------
class calPixelQualityV : public calAbs {
public:
  
  calPixelQualityV() = default;
  calPixelQualityV(cdbAbs *db);
  calPixelQualityV(cdbAbs *db, std::string tag);
  ~calPixelQualityV();

  // -- direct accessors
  uint32_t id(uint32_t id) {return fMapConstants[id].id;}

  std::string getName() override {return fPixelQualityTag;}
  void        calculate(std::string hash) override;

  bool        getNextID(uint32_t &ID);
  
private:
  std::string fPixelQualityTag{"pixelquality_"};

  // -- local and private
  struct constants {
    uint32_t id; 
    std::vector<char> matrix;
  };

  std::map<uint32_t, constants> fMapConstants;
  std::map<uint32_t, constants>::iterator fMapConstantsIt{fMapConstants.end()};
};

#endif
