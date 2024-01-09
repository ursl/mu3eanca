#ifndef RUNRECORD_h
#define RUNRECORD_h

#include <string>

class runRecord {
public:
  runRecord();
  void print();
  std::string printString();
  std::string json() const;

  int         fRun;
  std::string fRunStart, fRunEnd;
  std::string fRunDescription, fRunOperators;
  int         fNFrames;
  int         fBeamMode;
  float       fBeamCurrent;
  float       fMagnetCurrent;
  std::string fConfigurationKey;
  
};

#endif

