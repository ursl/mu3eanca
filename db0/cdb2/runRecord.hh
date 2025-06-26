#ifndef RUNRECORD_h
#define RUNRECORD_h

#include <string>
#include <vector>
#include "DataQuality.hh"
#include "RunInfo.hh"
#include "Resource.hh"

class runRecord {
public:
  runRecord();
  ~runRecord();

  // -- query functions
  bool        isSignificant() const;
  std::string getRunInfoClass() const;
  std::string getRunInfoComments() const;
  RunInfo     getRunInfo() const;
  DataQuality getDQ() const;
  
  // -- print the run record to cout using printSummary()
  void print();
  // -- return a string with a summary of the run record
  std::string printSummary() const;
  // -- return a json string with raw JSON string from which the class was instantiated
  std::string json() const;
  // -- return a json string with interpreted values
  std::string jsonInterpreted() const;
  void fillFromJson(const std::string &jsonString);
  void corrupted(std::string );
  
  bool              fBOREORValid; 
  // -- index to last (most uptodate) instance
  int               fDataQualityIdx, fRunInfoIdx;

  long unsigned int fBORRunNumber;
  std::string       fBORStartTime;
  std::string       fBORShiftCrew;
  std::string       fBORRunClass;
  float             fBORMu3eMagnet;
  bool              fBORPixelReadout;
  bool              fBORSciFiReadout;
  bool              fBORSciTileReadout;
  bool              fBORBeamBlockerOpen;
  
  std::string       fEORStopTime;
  long unsigned int fEOREvents;
  double            fEORFileSize;
  double            fEORDataSize;
  std::string       fEORComments;
  
  std::string       fConfigurationKey;

  std::vector<DataQuality> fvDQ;
  std::vector<RunInfo>     fvRI;
  std::vector<Resource>    fvResources;

  std::string       fJSONString;
private:
};

#endif
