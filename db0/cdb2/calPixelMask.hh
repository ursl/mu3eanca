#ifndef CALPIXELMASK_h
#define CALPIXELMASK_h

#include "calAbs.hh"

#include <string>
#include <map>
#include <vector>

enum Masked {
  Unmasked = 0,
  Masked = 9, 
  Unknown = 10
};

// static std::string maskedToString(enum Masked maskedStatus) {
//   switch (maskedStatus) {
//     case Unmasked: return "Unmasked";
//     case Masked: return "Masked";
//     case Unknown: return "Unknown";
//     default: return "Unknown";
//   }
// }

// ----------------------------------------------------------------------
// pixel mask class 
// ----------------------------------------------------------------------
class calPixelMask : public calAbs {
public:
  calPixelMask() = default;
  calPixelMask(cdbAbs *db);
  calPixelMask(cdbAbs *db, std::string tag);
  ~calPixelMask();

  // -- direct accessors
  uint32_t id(uint32_t id) {return fMapConstants[id].id;}

  std::string getName() override {return fPixelMaskTag;}
  void        calculate(std::string hash) override;

  std::string makeBLOB() override;
  std::string makeBLOB(const std::map<unsigned int, std::vector<double>>&) override;
  void printBLOB(std::string s, int verbosity = 1) override;
  std::string printBLOBString(std::string blob, int verbosity = 1) override;

  void readAllMaskBinaryFiles(std::string directory);
  void readMaskBinaryFile(std::string filename); 
  void fillCompletelyUnmasked(unsigned int chipid);

  enum Masked getMasked(unsigned int chipid, int icol, int irow);

  bool        getNextID(uint32_t &ID);
  void        resetIterator() {fMapConstantsIt = fMapConstants.begin();}

  std::string getSchema() override {return fSchema;}
  size_t      getPayloadSize() const override {return fMapConstants.size();}

private:
  std::string fPixelMaskTag{"pixelmask_"};
  std::string fSchema{"ui_id,c_mask[64000]"};

  // -- local and private
  struct constants {
    uint32_t id;
    //std::array<std::array<uint32_t, 8>, 256> mask;
    //std::vector<uint32_t> mask;
    char mask[256*250];
  };

  std::map<uint32_t, constants> fMapConstants;

  std::map<uint32_t, constants>::iterator fMapConstantsIt{fMapConstants.end()};
};

#endif
