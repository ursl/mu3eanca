#ifndef CALFIBREQUALITY_h
#define CALFIBREQUALITY_h

#include "calAbs.hh"

#include <iostream>
#include <string>
#include <map>

// ----------------------------------------------------------------------
// tile quality class
// ----------------------------------------------------------------------
class calFibreQuality : public calAbs {
public:
  enum Status {
    ChannelNotFound = -1,
    Good = 0,
    Noisy = 1,
    Dead = 2,
    Unlocked = 3,
    HasNoData = 4,
    Unset = 5
  };

  calFibreQuality() = default;
  calFibreQuality(cdbAbs *db);
  calFibreQuality(cdbAbs *db, std::string tag);
  ~calFibreQuality();

  // -- direct accessors
  Status getAsicStatus(uint32_t asicID);
  bool   getAsicLock(uint32_t asicID);
  bool   getAsicHasData(uint32_t asicID);
  int    getAsicQuality(uint32_t asicID);
  double getAsicThreshold(uint32_t asicID);
  double getAsicEfficiency(uint32_t asicID);

  std::string getName() override {return fFibreQualityTag;}
  void        calculate(std::string hash) override;

  std::string makeBLOB() override;
  // -- verbosity = <0 (not-good channels), 0 (all channels), >0 (good channels)
  void printBLOB(std::string, int verbosity = 1) override;
  std::string printBLOBString(std::string blob, int verbosity = 0) override;

  void writeCSV(std::string filename);
  void readCSV(std::string filename);

  void        resetIterator() {fMapConstantsIt = fMapConstants.begin();}
  bool        getNextID(uint32_t &ID);

  std::string getSchema() override {return fSchema;}

private:
  std::string fFibreQualityTag{"fibrequality_"};
  std::string fSchema{"ui_id,i_quality,i_lock,i_hasData,thr,eff"};
  int fRunNumber;

  // -- local and private
  struct constants {
    uint32_t id;
    int quality;
    int lock;
    int hasData;
    double threshold;
    double efficiency;
  };

  std::map<uint32_t, constants> fMapConstants;
  std::map<uint32_t, constants>::iterator fMapConstantsIt{fMapConstants.end()};
};

// ----------------------------------------------------------------------
// Stream output operator for Status enum
// ----------------------------------------------------------------------
inline std::ostream& operator<<(std::ostream& os, calFibreQuality::Status status) {
  switch (status) {
    case calFibreQuality::ChannelNotFound:
      os << "ChannelNotFound";
      break;
    case calFibreQuality::Good:
      os << "Good";
      break;
    case calFibreQuality::Noisy:
      os << "Noisy";
      break;
    case calFibreQuality::Dead:
      os << "Dead";
      break;
      case calFibreQuality::Unlocked:
      os << "Unlocked";
      break;
    case calFibreQuality::Unset:
      os << "Unset";
      break;
    default:
      os << "Unknown(" << static_cast<int>(status) << ")";
      break;
  }
  return os;
}

#endif
