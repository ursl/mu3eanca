#ifndef SENSOR_H
#define SENSOR_H

#include <TVector3.h>

// ----------------------------------------------------------------------
struct sensor {
  int layer, localLadder, simLadder,  confLadder, simChip, runChip, ladderChip,  direction; 
  int status;
  TVector3 v;
};

#endif
