#ifndef CALTILEQUALITY_h
#define CALTILEQUALITY_h

#include "calAbs.hh"

#include <iostream>
#include <string>
#include <map>

// ----------------------------------------------------------------------
// tile quality class
// ----------------------------------------------------------------------
class calTileQuality : public calAbs {
public:
  enum Status {
    ChannelNotFound = -1,
    Good = 0,
    Noisy = 1,
    Dead = 2,
    Unset = 3,
    DeclaredBad = 4,
    Masked = 9
  };

  calTileQuality() = default;
  calTileQuality(cdbAbs *db);
  calTileQuality(cdbAbs *db, std::string tag);
  ~calTileQuality();

  // -- direct accessors
  Status getChannelQuality(uint32_t id);

  std::string getName() override {return fTileQualityTag;}
  void        calculate(std::string hash) override;

  std::string makeBLOB() override;
  std::string makeBLOB(const std::map<unsigned int, std::vector<double>>&) override;
  // -- verbosity = <0 (not-good channels), 0 (all channels), >0 (good channels)
  void printBLOB(std::string, int verbosity = 1) override;
  std::string printBLOBString(std::string blob, int verbosity = 1) override;

  void writeJSON(std::string filename);
  void readJSON(std::string filename);

  void        resetIterator() {fMapConstantsIt = fMapConstants.begin();}
  bool        getNextID(uint32_t &ID);

  std::string getSchema() override {return fSchema;}

private:
  std::string fTileQualityTag{"tilequality_"};
  std::string fSchema{"ui_id,i_qual"};
  int fRunNumber;

  // -- local and private
  struct constants {
    uint32_t id;
    int quality;
  };

  std::map<uint32_t, constants> fMapConstants;
  std::map<uint32_t, constants>::iterator fMapConstantsIt{fMapConstants.end()};
};

// ----------------------------------------------------------------------
// Stream output operator for Status enum
// ----------------------------------------------------------------------
inline std::ostream& operator<<(std::ostream& os, calTileQuality::Status status) {
  switch (status) {
    case calTileQuality::ChannelNotFound:
      os << "ChannelNotFound";
      break;
    case calTileQuality::Good:
      os << "Good";
      break;
    case calTileQuality::Noisy:
      os << "Noisy";
      break;
    case calTileQuality::Dead:
      os << "Dead";
      break;
    case calTileQuality::Unset:
      os << "Unset";
      break;
    case calTileQuality::DeclaredBad:
      os << "DeclaredBad";
      break;
    case calTileQuality::Masked:
      os << "Masked";
      break;
    default:
      os << "Unknown(" << static_cast<int>(status) << ")";
      break;
  }
  return os;
}

#endif
