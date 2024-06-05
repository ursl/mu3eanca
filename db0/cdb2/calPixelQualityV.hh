#ifndef CALPIXELQUALITYV_h
#define CALPIXELQUALITYV_h

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
  
  // -- these are identical to the code in calPixelQuality (but this does not derive from that)
  std::string makeBLOB(std::map<unsigned int, std::vector<double> >) override;
  std::map<unsigned int, std::vector<double> > decodeBLOB(std::string) override;
  void printBLOB(std::string, int verbosity = 1) override;
  
  int         getStatus(unsigned int chipid, int icol, int irow)  override;
  
  bool        getNextID(uint32_t &ID);
  void        printPixelQuality(unsigned int chipid, int minimumStatus = 0);
  
  std::string getSchema() override {return fSchema;}
  
private:
  std::string fPixelQualityTag{"pixelqualityv_"};
  
  std::string fSchema{"ui_id,i_npix,[i_col,i_row,ui_qual]"};
  
  struct pixel {
    int icol, irow;
    char iqual;
  };
  
  // -- local and private
  struct constants {
    uint32_t id;
    std::vector<pixel> vpixel;
  };
  
  std::map<uint32_t, constants> fMapConstants;
  std::map<uint32_t, constants>::iterator fMapConstantsIt{fMapConstants.end()};
};

#endif
