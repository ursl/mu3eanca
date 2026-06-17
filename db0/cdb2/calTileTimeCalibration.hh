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

  // -- read JSON file - either the one by Erik/Tiles or the result of writeJSON("bla.json")
  void readJSON(std::string filename);
  // -- NOTE: The resulting JSON is NOT the full JSON as provided by Erik/Tiles 
  //          but only the part that is needed for the CDB/payload!
  void writeJSON(std::string filename);

  void        resetIterator() {fMapConstantsIt = fMapConstants.begin();}
  bool        getNextID(uint32_t &ID);

  std::string getSchema() override {return fSchema;}
  size_t      getPayloadSize() const override {return fMapConstants.size();}

private:
  std::string fTileTimeCalibrationTag{"tiletimecalibration_"};
  std::string fSchema{"ui_id,d_dnl[32],d_timeAlignment,i_timeWalk_nbins[,d_timeWalk_ns][,d_timeWalk_energy]"};

  // -- local and private
  struct constants {
    uint32_t id;
    std::vector<double> dnl_corrected_time_fraction;
    double timeAlignment_offset_ns;
    std::vector<double> timeWalk_correction_ns;
    std::vector<double> timeWalk_correction_energy;
  };

  std::map<uint32_t, constants> fMapConstants;
  std::map<uint32_t, constants>::iterator fMapConstantsIt{fMapConstants.end()};
};


#endif
