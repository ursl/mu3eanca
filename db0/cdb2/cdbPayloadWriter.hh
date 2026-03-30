#ifndef CDBPAYLOADWRITER_H
#define CDBPAYLOADWRITER_H

#include <string>
#include <vector>

// ----------------------------------------------------------------------
// CDB payload writer: produces payload files for various calibration types
//                     also produces ideal bootstrap files (csv,json,...)
//                     allows filtering on specific setups 
//                       (in particular various pixel configurations 
//                        and US/DS tiles)
// Usage examples
// ./bin/cdbRunPayloadWriter -m pixelmask -u /Users/ursl/data/mu3e/test-cdb -a "tune1 pixel masks (tdac_files_bu_tune1_refined)" -d /Users/ursl/data/mu3e/masks/tdac_files_bu_tune1_refined -t datav6.6=2025V0 -r 4763
// ./bin/cdbRunPayloadWriter -m pixelmask -u /Users/ursl/data/mu3e/test-cdb -a "tune2 pixel masks (tdac_files_bu_06_21_bestsofar)" -d /Users/ursl/data/mu3e/masks/tdac_files_bu_06_21_bestsofar -t datav6.6=2025V0 -r 5592
//
//
//
// ----------------------------------------------------------------------
class cdbPayloadWriter {
public:
  cdbPayloadWriter();
  ~cdbPayloadWriter() = default;

  void writeAlignmentPayloads(std::string payloaddir, std::string gt, std::string type, std::string ifilename, std::string annotation, int iov);

  void writePixelQualityLMPayloads(std::string payloaddir, std::string gt, std::string filename, std::string annotation, int iov);
  void writePixelQualityLMIdealInput(std::string filename, std::string mode = "all");

  void writeFibreQualityPayloads(std::string payloaddir, std::string gt, std::string filename, std::string annotation, int iov);
  void writeFibreQualityIdealInput(std::string filename, std::string mode = "all");

  void writeTileQualityPayloads(std::string payloaddir, std::string gt, std::string filename, std::string annotation, int iov);
  void writeTileQualityIdealInput(std::string filename, std::string mode = "all");

  void writeDetSetupV1Payloads(std::string payloaddir, std::string gt, std::string filename, std::string annotation, int iov);

  void writePixelEfficiencyPayloads(std::string payloaddir, std::string gt, std::string filename, std::string annotation, int iov);
  void writePixelEfficiencyIdealInput(std::string filename, std::string mode = "all");

  void writeEventStuffV1Payloads(std::string payloaddir, std::string gt, std::string filename, std::string annotation, int iov);

  void writePixelTimeCalibrationPayloads(std::string payloaddir, std::string gt, std::string filename, std::string annotation, int iov);
  void writePixelTimeCalibrationIdealInput(std::string filename, std::string mode);

  void writePixelMaskPayloads(std::string payloaddir, std::string gt, std::string binmaskfiledir, std::string annotation, int iov);
  void writePixelMaskIdealPayload(std::string payloaddir, std::string gt, std::string annotation, std::string mode);

  void run(int argc, const char* argv[]);

  // -------------------
  // -- helper functions

  // -- create code (for copy-pasting) with definitions of chip IDs per layer and station
  void createSensorIDs(std::string inputfilename);

  // -- create code (for copy-pasting) with definitions of fibre IDs 
  void createFibreIDs(std::string inputfilename);

  // -- create code (for copy-pasting) with definitions of tile IDs
  void createTileIDs(std::string inputfilename);


  // -- fill vector with (pixel) chip IDs based on mode
  void fillChipIDs(std::vector<unsigned int> &vChipIDs, std::string mode);

  // -- fill vector with tile IDs based on mode (non-ideal: only DS possible for 2025/2026? running)
  void fillTileIDs(std::vector<unsigned int> &vTileIDs, std::string mode);

  // -- fill vector with fibre IDs based on mode (no non-ideal mode available currently)
  void fillFibreIDs(std::vector<unsigned int> &vFibreIDs, std::string mode);


  private:
  // -- pixels
  std::vector<unsigned int> fChipIDs, fCentral3LayerChipIDs, fCentral4LayerChipIDs, 
  fLayer1ChipIDs, fLayer2ChipIDs, 
  fLayer3Station0ChipIDs, fLayer3Station1ChipIDs, fLayer3Station2ChipIDs, 
  fLayer4Station0ChipIDs, fLayer4Station1ChipIDs, fLayer4Station2ChipIDs;
  // -- tiles
  std::vector<unsigned int> fTileIDs,
  fTileUS, fTileDS;
  // -- fibres
  std::vector<unsigned int> fFibreIDs;
};

#endif
