#ifndef CDBPAYLOADWRITER_H
#define CDBPAYLOADWRITER_H

#include <string>

// ----------------------------------------------------------------------
// CDB payload writer: produces payload files for various calibration types
// ----------------------------------------------------------------------
class cdbPayloadWriter {
public:
  cdbPayloadWriter() = default;
  ~cdbPayloadWriter() = default;

  void writeAlignmentInformation(std::string payloaddir, std::string gt, std::string type, std::string ifilename, std::string annotation, int iov);

  // -- If the input files is a root file, then the contents will be "perfect" (the root file is simply used for the chipIDs)
  void writePixelQualityLM(std::string payloaddir, std::string gt, std::string filename, std::string annotation, int iov);
  void writeFibreQuality(std::string payloaddir, std::string gt, std::string filename, std::string annotation, int iov);
  void writeTileQuality(std::string payloaddir, std::string gt, std::string filename, std::string annotation, int iov);

  void writeDetSetupV1(std::string payloaddir, std::string gt, std::string filename, std::string annotation, int iov);

  void writePixelEfficiencyPayload(std::string payloaddir, std::string gt, std::string filename, std::string annotation, int iov);

  void writeEventStuffV1(std::string payloaddir, std::string gt, std::string filename, std::string annotation, int iov);

  void run(int argc, const char* argv[]);
};

#endif
