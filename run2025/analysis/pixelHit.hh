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
 
  int rawToT() {
    return (fDebugSiData >> 27) & 0x1F;
  }
  int hitToT(int ckdivend2 = 31) {
    int ckdivend(0);
    return ( ( (0x1F+1) + rawToT() -  ( (fTimeNs % (1 << 11)) * (ckdivend + 1) / (ckdivend2 + 1) ) & 0x1F) );
  }
};
#endif
