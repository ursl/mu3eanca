#ifndef SCITTHIT_HH
#define SCITTHIT_HH

#include <stdint.h>

struct scittHit {
  scittHit() : fID(0), fX(0), fY(0), fZ(0), fStatus(0), fTS(0), fFrameID(0) {}
  
  scittHit(uint32_t id, double x, double y, double z, uint32_t status, uint64_t ts, uint32_t frameID) : 
    fID(id), fX(x), fY(y), fZ(z), fStatus(status), fTS(ts), fFrameID(frameID) {}
    
  uint32_t row() const {
    return (fID >> 0) & 0xFF;
  }

  uint32_t col() const {
    return (fID >> 8) & 0xFF;
  }

  uint32_t chipID() const {
    return (fID >> 16) & 0xFFFF;
  }
       
  // -- hit input
  uint32_t fID;
  double fX, fY, fZ;
  uint32_t fStatus;
  uint64_t fTS;
  uint32_t fFrameID; /*lower 32bits*/
  
};
  
#endif
