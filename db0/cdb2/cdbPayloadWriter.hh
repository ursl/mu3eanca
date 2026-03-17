#ifndef CDBPAYLOADWRITER_H
#define CDBPAYLOADWRITER_H

#include <string>
#include <vector>

// ----------------------------------------------------------------------
// CDB payload writer: produces payload files for various calibration types
//                     also produces ideal bootstrap files (csv,json,...)
// ----------------------------------------------------------------------
class cdbPayloadWriter {
public:
  cdbPayloadWriter();
  ~cdbPayloadWriter() = default;

  void writeAlignmentPayloads(std::string payloaddir, std::string gt, std::string type, std::string ifilename, std::string annotation, int iov);

  void writePixelQualityLMPayloads(std::string payloaddir, std::string gt, std::string filename, std::string annotation, int iov);
  void writePixelQualityLMIdealInput(std::string filename, std::string mode = "all");

  void writeFibreQualityPayloads(std::string payloaddir, std::string gt, std::string filename, std::string annotation, int iov);
  void writeTileQualityPayloads(std::string payloaddir, std::string gt, std::string filename, std::string annotation, int iov);

  void writeDetSetupV1Payloads(std::string payloaddir, std::string gt, std::string filename, std::string annotation, int iov);

  void writePixelEfficiencyPayloads(std::string payloaddir, std::string gt, std::string filename, std::string annotation, int iov);

  void writeEventStuffV1Payloads(std::string payloaddir, std::string gt, std::string filename, std::string annotation, int iov);

  void writePixelTimeCalibrationPayloads(std::string payloaddir, std::string gt, std::string filename, std::string annotation, int iov);
  void writePixelTimeCalibrationIdealInput(std::string filename);

  void run(int argc, const char* argv[]);

  // -------------------
  // -- helper functions

  // -- create code (for copy-pasting) with definitions of chip IDs per layer and station
  void createChipIDsPerLayer(std::string inputfilename);

  // -- fill vector with chip IDs based on mode
  void fillChipIDs(std::vector<unsigned int> &vChipIDs, std::string mode);

  // -- create code (for copy-pasting) with definitions of fibre IDs 
  void createFibreIDs(std::string inputfilename);

  // -- create code (for copy-pasting) with definitions of tile IDs
  void createTileIDs(std::string inputfilename);

private:
  std::vector<unsigned int> fChipIDs, fCentral3LayerChipIDs, fCentral4LayerChipIDs, 
  fLayer1ChipIDs, fLayer2ChipIDs, 
  fLayer3Station0ChipIDs, fLayer3Station1ChipIDs, fLayer3Station2ChipIDs, 
  fLayer4Station0ChipIDs, fLayer4Station1ChipIDs, fLayer4Station2ChipIDs;
};

#endif
