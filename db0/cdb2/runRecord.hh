#ifndef RUNRECORD_h
#define RUNRECORD_h

#include <string>

class runRecord {
public:
  runRecord();
  void print();
  std::string printString();
  std::string json() const;

  int          fBORRunNumber;
  std::string  fBORStartTime;
  int          fBORSubsystems;
  float        fBORBeam;
  std::string  fBORShiftCrew;

  std::string  fEORStopTime;
  unsigned int fEOREvents;
  int          fEORFileSize;
  int          fEORDataSize;
  std::string  fEORComments;

  std::string  fConfigurationKey;
  
  
};

#endif

