#ifndef CALPIXELQUALITY_h
#define CALPIXELQUALITY_h

#include "calAbs.hh"

#include <string>
#include <map>

// ----------------------------------------------------------------------
// pixel quality class 
// ----------------------------------------------------------------------
class calPixelQuality : public calAbs {
public:
  
  calPixelQuality() = default;
  calPixelQuality(cdbAbs *db);
  calPixelQuality(cdbAbs *db, std::string tag);
  ~calPixelQuality();

  // -- direct accessors
  uint32_t id(uint32_t id) {return fMapConstants[id].id;}

  std::string getName() override {return fPixelQualityTag;}
  void        calculate(std::string hash) override;

  char        getStatus(unsigned int chipid, int icol, int irow);
  
  bool        getNextID(uint32_t &ID);
  
private:
  std::string fPixelQualityTag{"pixelquality_"};

  // -- local and private
  struct constants {
    uint32_t id; 
    std::array<std::array<char,256>,250> matrix{0};
  };

  std::map<uint32_t, constants> fMapConstants;
  std::map<uint32_t, constants>::iterator fMapConstantsIt{fMapConstants.end()};
};

#endif
