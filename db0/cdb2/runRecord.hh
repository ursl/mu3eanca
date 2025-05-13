#ifndef RUNRECORD_h
#define RUNRECORD_h

#include <string>

class runRecord {
public:
  runRecord();
  void print();
  std::string printString();
  std::string json() const;
  void fillFromJson(const std::string &jsonString);
  void corrupted(std::string );
  
  bool              fBOREORValid, fDataQualityValid, fRunInfoValid;

  long unsigned int fBORRunNumber;
  std::string       fBORStartTime;
  int               fBORSubsystems;
  double            fBORBeam;
  std::string       fBORShiftCrew;
  
  std::string       fEORStopTime;
  long unsigned int fEOREvents;
  double            fEORFileSize;
  double            fEORDataSize;
  std::string       fEORComments;
  
  std::string       fConfigurationKey;

  int               fDQMu3e;
  int               fDQBeam;
  int               fDQVertex;
  int               fDQPixel;
  int               fDQTiles;
  int               fDQFibres;
  int               fDQCalibration;
  int               fDQGoodLinks;
  std::string       fDQVersion;

  std::string       fRIClass;
  std::string       fRISignificant;
  std::string       fRIComments;
  std::string       fRIComponents;
  std::string       fRIComponentsOut;
  std::string       fRIMidasVersion;
  std::string       fRIMidasGitRevision;
  std::string       fRIDAQVersion;
  std::string       fRIDAQGitRevision;
  std::string       fRIVtxVersion;
  std::string       fRIVtxGitRevision; 
  std::string       fRIPixVersion;
  std::string       fRIPixGitRevision;
  std::string       fRITilVersion;
  std::string       fRITilGitRevision;
  std::string       fRIFibVersion;
  std::string       fRIFibGitRevision;
  std::string       fRIVersion;  
};

#endif
