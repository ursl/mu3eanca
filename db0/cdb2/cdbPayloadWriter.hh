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

  // -- If the input files is a root file, then the contents will be "perfect" (the root file is simply used for the chipIDs)
  void writePixelQualityLMPayloads(std::string payloaddir, std::string gt, std::string filename, std::string annotation, int iov);
  void writeFibreQualityPayloads(std::string payloaddir, std::string gt, std::string filename, std::string annotation, int iov);
  void writeTileQualityPayloads(std::string payloaddir, std::string gt, std::string filename, std::string annotation, int iov);

  void writeDetSetupV1Payloads(std::string payloaddir, std::string gt, std::string filename, std::string annotation, int iov);

  void writePixelEfficiencyPayloads(std::string payloaddir, std::string gt, std::string filename, std::string annotation, int iov);

  void writeEventStuffV1Payloads(std::string payloaddir, std::string gt, std::string filename, std::string annotation, int iov);

  void writePixelTimeCalibrationPayloads(std::string payloaddir, std::string gt, std::string filename, std::string annotation, int iov);
  void writePixelTimeCalibrationIdealInput(std::string filename);

  void run(int argc, const char* argv[]);

  // -- helper functions
  void createChipIDsPerLayer(std::string inputfilename);

private:
  std::vector<unsigned int> fChipIDs, fLayer1ChipIDs, fLayer2ChipIDs, fLayer3ChipIDs, fLayer4ChipIDs;
};

#endif
