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

  std::string makeBLOB(std::map<unsigned int, std::vector<double> >) override;
  std::map<unsigned int, std::vector<double> > decodeBLOB(std::string) override;

  int         getStatus(unsigned int chipid, int icol, int irow) override;
  
  bool        getNextID(uint32_t &ID);
  void        printPixelQuality(unsigned int chipid, int minimumStatus = 0);
  
private:
  std::string fPixelQualityTag{"pixelquality_"};

  // -- local and private
  struct constants {
    uint32_t id; 
    //    std::array<std::array<char,256>,250> matrix;
    char matrix[256][250];
  };

  std::map<uint32_t, constants> fMapConstants;
  std::map<uint32_t, constants>::iterator fMapConstantsIt{fMapConstants.end()};
};

#endif
