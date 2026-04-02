#ifndef PIXELHIT_HH
#define PIXELHIT_HH

#include <stdint.h>

struct pixelHit {
  pixelHit() : fID(0), fTS(0), fRawToT(0), fStatus(0), 
          fX(0), fY(0), fZ(0), fTime(0), fFrameID(0) {}
  pixelHit(uint32_t id, uint32_t ts, uint32_t rawToT, uint32_t status, 
          double x, double y, double z, double time, uint32_t frameID) : 
          fID(id), fTS(ts), fRawToT(rawToT), fStatus(status), 
          fX(x), fY(y), fZ(z), fTime(time), fFrameID(frameID) {}

  // -- hit input
  uint32_t fID, fTS, fRawToT, fStatus;
  uint32_t fFrameID; /*lower 32bits*/
  double fX, fY, fZ, fTime;


  uint32_t row() {
    return (fID >> 0) & 0xFF;
  }
  uint32_t col() {
    return (fID >> 8) & 0xFF;
  }
  uint32_t chipID() {
    return (fID >> 16) & 0xFFFF;
  }

};

#endif
