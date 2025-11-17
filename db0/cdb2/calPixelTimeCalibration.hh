#ifndef CALPIXELTIMECALIBRATION_h
#define CALPIXELTIMECALIBRATION_h

#include "calAbs.hh"

#include <string>
#include <map>
#include <array>
#include <vector>


// ----------------------------------------------------------------------
// pixel timing calibration class (based on Nik Berger's "one_pass_calibration")
// ----------------------------------------------------------------------
class calPixelTimeCalibration : public calAbs {
public:
  calPixelTimeCalibration() = default;
  calPixelTimeCalibration(cdbAbs *db);
  calPixelTimeCalibration(cdbAbs *db, std::string tag);
  ~calPixelTimeCalibration();

  std::string getName() override {return fPixelTimeCalibrationTag;}
  void        calculate(std::string hash) override;

  std::string makeBLOB() override;
  void printBLOB(std::string s, int verbosity = 1) override;

  void readTxtFile(std::string filename); 
  void writeTxtFile(std::string filename);

  // -- struct for constants
  struct constants {
    double mean;
    double meanerr;
    double sigma;
    double sigmaerr;
  };

  // -- get constants indexed by ichip, isector, itotbin
  const constants& getConstants(int ichip, int isector, int itotbin) const;
  double getMean(int ichip, int isector, int itotbin) const;
  double getMeanErr(int ichip, int isector, int itotbin) const;
  double getSigma(int ichip, int isector, int itotbin) const;
  double getSigmaErr(int ichip, int isector, int itotbin) const;

  bool        getNextID(uint32_t &ID);

  std::string getSchema() override {return fSchema;}

private:
  std::string fPixelTimeCalibrationTag{"pixeltimecalibration_"};
  std::string fSchema{"ui_id,bla"};

  // -- constants
  static const int NCALIBRATIONCHIPS{108};
  static const int NSECTOR{6};
  static const int NTOTBINS{32};

  // -- array of constants (vector for first dimension, arrays for nested dimensions)
  std::map<int, std::array<std::array<constants, NTOTBINS>, NSECTOR>> fMapConstants;

};

#endif
