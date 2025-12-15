#ifndef CALEVENTSTUFFV1_h
#define CALEVENTSTUFFV1_h

#include "calAbs.hh"

#include <string>
#include <map>

// ----------------------------------------------------------------------
// EventStuff class
// ----------------------------------------------------------------------
class calEventStuffV1 : public calAbs {
public:

  calEventStuffV1() = default;
  calEventStuffV1(cdbAbs *db);
  calEventStuffV1(cdbAbs *db, std::string tag);
  ~calEventStuffV1();

  // -- direct accessors
  std::string getName() override {return fEventStuffTag;}
  void        calculate(std::string hash) override;

  std::string makeBLOB() override;
  // -- verbosity = -1 (all), 0 (no elements), n (n elements)
  void printBLOB(std::string, int verbosity = -1) override;
  std::string printBLOBString(std::string blob, int verbosity = -1) override;
  std::string readJSON(std::string filename);
  std::string getSchema() override {return fSchema;}

private:
  std::string fEventStuffTag{"eventstuffv1_"};

  // -- local and private
  struct constants {
    struct PixelData {
      uint64_t startFrame;
      uint64_t endFrame;
    } pixelData;
  };

  std::string fSchema{"pixeldata.(ull_startframe,ull_endframe)"};

  constants fConstants;
};

#endif
