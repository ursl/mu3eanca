#ifndef PIXELHIT_HH
#define PIXELHIT_HH

struct pixelHit {
  // -- input
  int fPixelID; 
  int fHitToT;  
  unsigned long fDebugSiData;
  int fChipID, fCol, fRow, fTime, fTimeNs;

  // -- calculated locally
  int fRawToT, fBitToT; 
  int fStatus; // 0 = good, 1 = rj, 2 = invalid
  int fStatusBits; // 0 = edge, 1 = low
 
};
#endif
