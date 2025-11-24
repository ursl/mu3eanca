#ifndef CALTILEQUALITY_h
#define CALTILEQUALITY_h

#include "calAbs.hh"

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
  Status getChannelQuality(uint32_t id) {return static_cast<Status>(fMapConstants[id].quality);}

  std::string getName() override {return fTileQualityTag;}
  void        calculate(std::string hash) override;

  std::string makeBLOB() override;
  void printBLOB(std::string, int verbosity = 1) override;
  void writeJSON(std::string filename);
  void readJSON(std::string filename);

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

#endif
