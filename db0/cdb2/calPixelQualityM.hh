#ifndef CALPIXELQUALITYM_h
#define CALPIXELQUALITYM_h

#include "calAbs.hh"

#include <string>
#include <map>


// ----------------------------------------------------------------------
// pixel quality class using a map (keyed with icol*250 + irow)
// ----------------------------------------------------------------------
class calPixelQualityM : public calAbs {
public:

  calPixelQualityM() = default;
  calPixelQualityM(cdbAbs *db);
  calPixelQualityM(cdbAbs *db, std::string tag);
  ~calPixelQualityM();

  // -- direct accessors
  uint32_t id(uint32_t id) {return fMapConstants[id].id;}

  std::string getName() override {return fPixelQualityTag;}
  void        calculate(std::string hash) override;

  // -- these are identical to the code in calPixelQuality (but this does not derive from that)
  std::string makeBLOB(std::map<unsigned int, std::vector<double> >) override;
  std::map<unsigned int, std::vector<double> > decodeBLOB(std::string) override;
  void printBLOB(std::string s, int verbosity = 1) override;


  int         getStatus(unsigned int chipid, int icol, int irow)  override;

  bool        getNextID(uint32_t &ID);
  void        printPixelQuality(unsigned int chipid, int minimumStatus = 0);

private:
  std::string fPixelQualityTag{"pixelqualitym_"};


  // -- local and private
  struct constants {
    uint32_t id;
    std::map<int, char> mpixel;
  };

  std::map<uint32_t, constants> fMapConstants;
  std::map<uint32_t, constants>::iterator fMapConstantsIt{fMapConstants.end()};
};

#endif
