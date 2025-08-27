#ifndef PIXELHIT_HH
#define PIXELHIT_HH

struct pixelHit {
  // -- input
  int fPixelID; 
  int fHitToT;  
  unsigned long fDebugSiData;
  int fChipID, fCol, fRow, fTimeInt;
  double fTime, fTimeNs;
  double fX, fY, fZ;
  
  // -- not calculated in trirec
  uint32_t fRawToT, fBitToT; 
  int fStatus; // directly from pixelQualityLM
  int fStatusBits; // from pixelHistograms
  bool fValidHit;
 
  
  uint32_t calcToT(int ckdivend2 = 31) {
    int ckdivend(0);
    uint32_t localTime = fTimeInt % (1 << 11);  // local pixel time is first 11 bits of the global time
    fRawToT = ((fDebugSiData >> 27) & 0x1F);
    fBitToT = ( ( (0x1F+1) + fRawToT -  ( (localTime * (ckdivend + 1) / (ckdivend2 + 1) ) & 0x1F) ) & 0x1F);
    return fBitToT;
  }
};
#endif
