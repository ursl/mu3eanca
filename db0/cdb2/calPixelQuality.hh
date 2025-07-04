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
  enum Status {
    ChipNotFound = -1,
    Good = 0,
    Noisy = 1,
    Suspect = 2,
    DeclaredBad = 3,
    TurnedOff = 9
  };

  calPixelQuality() = default;
  calPixelQuality(cdbAbs *db);
  calPixelQuality(cdbAbs *db, std::string tag);
  ~calPixelQuality();

  // -- direct accessors
  uint32_t id(uint32_t id) {return fMapConstants[id].id;}

  std::string getName() override {return fPixelQualityTag;}
  void        calculate(std::string hash) override;

  std::string makeBLOB() override;
  std::string makeBLOB(const std::map<unsigned int, std::vector<double>>&) override;
  std::map<unsigned int, std::vector<double> > decodeBLOB(std::string) override;
  void printBLOB(std::string, int verbosity = 1) override;
  void writeCsv(std::string filename);
  void readCsv(std::string filename);

  virtual Status getStatus(unsigned int chipid, int icol, int irow);

  bool        getNextID(uint32_t &ID);
  void        printPixelQuality(unsigned int chipid, int minimumStatus = 0);

  std::string getSchema() override {return fSchema;}

private:
  std::string fPixelQualityTag{"pixelquality_"};
  std::string fSchema{"ui_id,i_npix,[i_col,i_row,ui_qual]"};

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
