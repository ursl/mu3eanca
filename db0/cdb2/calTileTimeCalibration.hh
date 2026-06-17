#ifndef CALTILETIMECALIBRATION_h
#define CALTILETIMECALIBRATION_h

#include "calAbs.hh"

#include <iostream>
#include <string>
#include <map>

// ----------------------------------------------------------------------
// tile time calibration class
// including
// - time walk
// - DNL (differential non-linearities)
// - time alignment
// ----------------------------------------------------------------------
class calTileTimeCalibration : public calAbs {
public:

  calTileTimeCalibration() = default;
  calTileTimeCalibration(cdbAbs *db);
  calTileTimeCalibration(cdbAbs *db, std::string tag);
  ~calTileTimeCalibration();

  std::string getName() override {return fTileTimeCalibrationTag;}
  void        calculate(std::string hash) override;

  std::string makeBLOB() override;
  std::string makeBLOB(const std::map<unsigned int, std::vector<double>>&) override;
  void printBLOB(std::string, int verbosity = 1) override;
  std::string printBLOBString(std::string blob, int verbosity = 1) override;

  void writeJSON(std::string filename);
  void readJSON(std::string filename);

  void        resetIterator() {fMapConstantsIt = fMapConstants.begin();}
  bool        getNextID(uint32_t &ID);

  std::string getSchema() override {return fSchema;}
  size_t      getPayloadSize() const override {return fMapConstants.size();}

private:
  std::string fTileTimeCalibrationTag{"tiletimecalibration_"};
  std::string fSchema{"ui_id,i_qual"};

  // -- local and private
  struct constants {
    uint32_t id;
    std::vector<double> dnl_corrected_time_fraction;
    double timeAlignment_offset_ns;
    std::vector<double> timeWalk_correction_ns;
    std::vector<double> timeWalk_correction_energy;
  };

  // -- Meta data
  int fRunNumber;
  int fTimestamp;

  std::map<uint32_t, constants> fMapConstants;
  std::map<uint32_t, constants>::iterator fMapConstantsIt{fMapConstants.end()};
};


#endif
