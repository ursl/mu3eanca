#ifndef TRACK_HH
#define TRACK_HH

#include <iostream>

// -- maximum number of hits (not clusters) associated with a track
static const int TRKHITMAX = 20;

struct track {
  track() {
    // -- clear stuff that is not set externally
    fTrkNhits = 0;
    fTrkHitOverflow = 0;
    for (int i = 0; i < TRKHITMAX; ++i) {
      fTrkHitIndices[i] = -1;
    }
  }

  double fTrkMomentum;
  double fTrkChi2;
  int   fTrkType;
  double fTrkPhi;
  double fTrkLambda;  
  double fTrkK, fTrkKerr2;
  double fTrkT0, fTrkT0Err, fTrkT0RMS;
  double fTrkT0Si, fTrkT0SiErr, fTrkT0SiRMS;
  double fTrkDoca;


  // -- segment->n :-)
  int   fTrkSegmentN;    
  // -- indicates how many more hits are associated with the segment that can be stored
  int   fTrkHitOverflow;
  // -- number of hits associated with the track
  int   fTrkNhits;       
  int   fTrkHitIndices[TRKHITMAX];

  bool fillIndex(int index) {
    bool OK(false);
    if (fTrkNhits < TRKHITMAX) {
      fTrkHitIndices[fTrkNhits] = index;
      fTrkNhits++;
      OK = true;
    }
    if (!OK) {
      fTrkHitOverflow++;
      std::cout << "track::fillIndex() ERROR: TRKHITMAX reached, index " << index << " not added" << std::endl;
    }
    return OK;
  }
};
#endif
