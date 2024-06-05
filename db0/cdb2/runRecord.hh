#ifndef RUNRECORD_h
#define RUNRECORD_h

#include <string>

class runRecord {
public:
  runRecord();
  void print();
  std::string printString();
  std::string json() const;
  
  long unsigned int fBORRunNumber;
  std::string  fBORStartTime;
  int          fBORSubsystems;
  double       fBORBeam;
  std::string  fBORShiftCrew;
  
  std::string  fEORStopTime;
  long unsigned int fEOREvents;
  double       fEORFileSize;
  double       fEORDataSize;
  std::string  fEORComments;
  
  std::string  fConfigurationKey;
  
  
};

#endif
