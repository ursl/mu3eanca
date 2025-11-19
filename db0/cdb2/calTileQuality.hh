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
    DeclaredBad = 2,
    TurnedOff = 9
  };

  calTileQuality() = default;
  calTileQuality(cdbAbs *db);
  calTileQuality(cdbAbs *db, std::string tag);
  ~calTileQuality();

  // -- direct accessors
  int channelQuality(uint32_t id, int channelIdx) {return fMapConstants[id].quality;}

  std::string getName() override {return fTileQualityTag;}
  void        calculate(std::string hash) override;

  std::string makeBLOB() override;
  std::string makeBLOB(const std::map<unsigned int, std::vector<double>>&) override;
  std::map<unsigned int, std::vector<double> > decodeBLOB(std::string) override;
  void printBLOB(std::string, int verbosity = 1) override;
  void writeCsv(std::string filename);
  void readCsv(std::string filename);

  virtual Status getStatus(unsigned int chipid, int icol, int irow);

  bool        getNextID(uint32_t &ID);
  void        printChannelQuality(uint32_t id, int channelIdx);

  std::string getSchema() override {return fSchema;}

private:
  std::string fTileQualityTag{"tilequality_"};
  std::string fSchema{"ui_id,i_qual"};

  // -- local and private
  struct constants {
    uint32_t id;
    int quality;
  };

  std::map<uint32_t, constants> fMapConstants;
  std::map<uint32_t, constants>::iterator fMapConstantsIt{fMapConstants.end()};
};

#endif
