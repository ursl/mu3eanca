#ifndef CALEVENTSTUFFV2_h
#define CALEVENTSTUFFV2_h

#include "calAbs.hh"

#include <string>
#include <map>

// ----------------------------------------------------------------------
// EventStuff class
// ----------------------------------------------------------------------
class calEventStuffV2 : public calAbs {
public:

  calEventStuffV2() = default;
  calEventStuffV2(cdbAbs *db);
  calEventStuffV2(cdbAbs *db, std::string tag);
  ~calEventStuffV2();

  // -- direct accessors
  uint64_t startFrameEventData() {return fConstants.eventData.startFrame;}
  uint64_t endFrameEventData() {return fConstants.eventData.endFrame;}
  uint64_t startFrameGoodPixelData() {return fConstants.pixelData.startFrame;}
  uint64_t endFrameGoodPixelData() {return fConstants.pixelData.endFrame;}

  std::string getName() override {return fEventStuffTag;}
  void        calculate(std::string hash) override;

  std::string makeBLOB() override;
  std::string makeBLOB(const std::map<unsigned int, std::vector<double>>&) override;
  // -- verbosity = -1 (all), 0 (no elements), n (n elements)
  void printBLOB(std::string, int verbosity = -1) override;
  std::string printBLOBString(std::string blob, int verbosity = -1) override;
  std::string readJSON(std::string filename);
  std::string getSchema() override {return fSchema;}
  size_t      getPayloadSize() const override {return 0;}

private:
  std::string fEventStuffTag{"eventstuffv2_"};

  // -- local and private
  struct constants {
    struct EventData {
      uint64_t startFrameGoodData{0};
      uint64_t endFrameGoodData{0xffffffffffffffff};
      uint64_t firstFrameWithFEBProblems{0xffffffffffffffff};
    } eventData;
    struct PixelData {
      uint64_t startFrameGoodData{0};
      uint64_t endFrameGoodData{0xffffffffffffffff};
      uint64_t firstFrameWithFEBUnsortedHitData{0xffffffffffffffff};
    } pixelData;
    struct TileData {
      uint64_t startFrameGoodData{0};
      uint64_t endFrameGoodData{0xffffffffffffffff};
      uint64_t firstFrameWithFEBUnsortedHitData{0xffffffffffffffff};
    } tileData;
    struct FibreData {
      uint64_t startFrameGoodData{0};
      uint64_t endFrameGoodData{0xffffffffffffffff};
      uint64_t firstFrameWithFEBUnsortedHitData{0xffffffffffffffff};
    } fibreData;
  };

  std::string fSchema{"eventdata.(ull_startframegooddata,ull_endframegooddata,ull_firstframewithfebproblems),pixeldata.(ull_startframegooddata,ull_endframegooddata,ull_firstframewithfebunsortedhitdata),tiledata.(ull_startframegooddata,ull_endframegooddata,ull_firstframewithfebunsortedhitdata),fibredata.(ull_startframegooddata,ull_endframegooddata,ull_firstframewithfebunsortedhitdata)"};

  constants fConstants;
};

#endif
