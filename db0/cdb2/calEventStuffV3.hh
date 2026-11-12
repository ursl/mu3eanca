#ifndef CALEVENTSTUFFV3_h
#define CALEVENTSTUFFV3_h

#include "calAbs.hh"

#include <map>
#include <string>
#include <vector>

// ----------------------------------------------------------------------
// EventStuff class
// ----------------------------------------------------------------------
class calEventStuffV3 : public calAbs {
public:

  calEventStuffV3() = default;
  calEventStuffV3(cdbAbs *db);
  calEventStuffV3(cdbAbs *db, std::string tag);
  ~calEventStuffV3();

  // -- For this payload, the only IOVs that make sense are ONE PAYLOAD PER RUN (because the 
  //    payload contain per-run information, e.g. EventData:endFrameData)
  // 
  // -- IMPORTANT: Before using this payload make sure that it is valid for the run you are looking at!  
  bool        isValid(int run) override {return run == fIovRun;}

  // -- direct accessors
  uint64_t startFrameEventData() {return fConstants.eventData.startFrameData;}
  uint64_t endFrameEventData() {return fConstants.eventData.endFrameData;}
  uint64_t firstFrameWithFEBProblems() {return fConstants.eventData.firstFrameWithFEBProblems;}

  uint64_t startFrameGoodPixelData() {return fConstants.pixelData.startFrameGoodData;}
  uint64_t endFrameGoodPixelData() {return fConstants.pixelData.endFrameGoodData;}
  uint64_t firstFrameWithFEBUnsortedPixelData() {return fConstants.pixelData.firstFrameWithFEBUnsortedHitData;}

  uint64_t startFrameGoodTileData() {return fConstants.tileData.startFrameGoodData;}
  uint64_t endFrameGoodTileData() {return fConstants.tileData.endFrameGoodData;}
  uint64_t firstFrameWithFEBUnsortedTileData() {return fConstants.tileData.firstFrameWithFEBUnsortedHitData;}

  uint64_t startFrameGoodFibreData() {return fConstants.fibreData.startFrameGoodData;}
  uint64_t endFrameGoodFibreData() {return fConstants.fibreData.endFrameGoodData;}
  uint64_t firstFrameWithFEBUnsortedFibreData() {return fConstants.fibreData.firstFrameWithFEBUnsortedHitData;}

  uint64_t n_ts() {return fConstants.eventData.n_ts;}
  uint64_t n_unexpected_ts() {return fConstants.eventData.n_unexpected_ts;}
  uint64_t n_midasEvent() {return fConstants.eventData.n_midasEvent;}
  uint64_t n_inv_dsin() {return fConstants.eventData.n_inv_dsin;}

  bool pixel_febs_ok() {return fConstants.pixelData.pixel_febs_ok;}
  uint64_t n_phit() {return fConstants.pixelData.n_phit;}
  uint64_t n_phit_dup() {return fConstants.pixelData.n_phit_dup;}
  std::string maskdirname() {return fConstants.pixelData.maskdirname;}
  bool maskdirname_ok() {return fConstants.pixelData.maskdirname_ok;}

  uint64_t n_thit() {return fConstants.tileData.n_thit;}
  bool tile_febs_ok() {return fConstants.tileData.tile_febs_ok;}

  uint64_t n_fhit() {return fConstants.fibreData.n_fhit;}
  bool fibre_febs_ok() {return fConstants.fibreData.fibre_febs_ok;}

  int n_deff() {return fConstants.daqData.n_deff;}
  double deff(const std::string& name);
  const std::vector<std::string>& deffNames() {return fConstants.daqData.deff_names;}
  const std::vector<double>& deffValues() {return fConstants.daqData.deff_values;}

  int n_link() {return fConstants.daqData.n_link;}
  double lvde(const std::string& name);
  const std::vector<std::string>& lvdeNames() {return fConstants.daqData.lvde_names;}
  const std::vector<double>& lvdeValues() {return fConstants.daqData.lvde_values;}

  double current_0() {return fConstants.magnetData.current_0;}
  double current_1() {return fConstants.magnetData.current_1;}

  double beam_shutter() {return fConstants.beamData.beam_shutter;}
  double proton_current() {return fConstants.beamData.proton_current;}
  std::string beamFilename() {return fConstants.beamData.filename;}

  int nInt() {return fConstants.reserveData.nInt;}
  int reserveInt(const std::string& name);
  const std::vector<std::string>& reserveIntNames() {return fConstants.reserveData.int_names;}
  const std::vector<int>& reserveIntValues() {return fConstants.reserveData.int_values;}

  int nUint64() {return fConstants.reserveData.nUint64;}
  uint64_t reserveUint64(const std::string& name);
  const std::vector<std::string>& reserveUint64Names() {return fConstants.reserveData.uint64_names;}
  const std::vector<uint64_t>& reserveUint64Values() {return fConstants.reserveData.uint64_values;}

  int nDouble() {return fConstants.reserveData.nDouble;}
  double reserveDouble(const std::string& name);
  const std::vector<std::string>& reserveDoubleNames() {return fConstants.reserveData.double_names;}
  const std::vector<double>& reserveDoubleValues() {return fConstants.reserveData.double_values;}

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
  std::string fEventStuffTag{"eventstuffv3_"};

  // -- local and private
  struct constants {
    struct EventData {
      uint64_t startFrameData{0};
      uint64_t endFrameData{0xffffffffffffffff};
      uint64_t firstFrameWithFEBProblems{0xffffffffffffffff};
      uint64_t n_ts{0xffffffffffffffff};
      uint64_t n_unexpected_ts{0xffffffffffffffff};
      uint64_t n_midasEvent{0xffffffffffffffff};
      uint64_t n_inv_dsin{0xffffffffffffffff};
      } eventData;
    struct PixelData {
      uint64_t startFrameGoodData{0};
      uint64_t endFrameGoodData{0xffffffffffffffff};
      uint64_t firstFrameWithFEBUnsortedHitData{0xffffffffffffffff};
      bool pixel_febs_ok{false};
      uint64_t n_phit{0xffffffffffffffff};
      uint64_t n_phit_dup{0xffffffffffffffff};
      std::string maskdirname{"nada"};
      bool maskdirname_ok{false};
    } pixelData;
    struct TileData {
      uint64_t startFrameGoodData{0};
      uint64_t endFrameGoodData{0xffffffffffffffff};
      uint64_t firstFrameWithFEBUnsortedHitData{0xffffffffffffffff};
      uint64_t n_thit{0xffffffffffffffff};
      bool tile_febs_ok{false};
    } tileData;
    struct FibreData {
      uint64_t startFrameGoodData{0};
      uint64_t endFrameGoodData{0xffffffffffffffff};
      uint64_t firstFrameWithFEBUnsortedHitData{0xffffffffffffffff};
      uint64_t n_fhit{0xffffffffffffffff};
      bool fibre_febs_ok{false};
    } fibreData;
    struct DAQData {
      int n_deff{0};
      std::vector<std::string> deff_names;
      std::vector<double> deff_values;
      int n_link{0};
      std::vector<std::string> lvde_names;
      std::vector<double> lvde_values;
    } daqData;

    struct MagnetData {
      double current_0{-1.};
      double current_1{-1.};
    } magnetData;

    struct BeamData {
      double beam_shutter{-1.0};
      double proton_current{-1.0};
      std::string filename{"nada"};
    } beamData;

    struct ReserveData {
      int nInt{0};
      std::vector<std::string> int_names;
      std::vector<int> int_values;
      int nUint64{0};
      std::vector<std::string> uint64_names;
      std::vector<uint64_t> uint64_values;
      int nDouble{0};
      std::vector<std::string> double_names;
      std::vector<double> double_values;
    } reserveData;
  };

  void readBLOB(std::vector<char>::iterator& it, constants& c);

  std::string fSchema;

  constants fConstants;
};

#endif
