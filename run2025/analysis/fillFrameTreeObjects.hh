#ifndef FILLFRAMETREEOBJECTS_HH
#define FILLFRAMETREEOBJECTS_HH

#include "Segment.h"

#include "frameTree.hh"
#include "track.hh"
#include "pixelHit.hh"

#include "mu3e/cdb/Mu3eConditions.hh"
#include "mu3e/cdb/calPixelQualityLM.hh"

class fillFrameTreeObjects {
public:
  fillFrameTreeObjects(frameTree *frameTree);
  fillFrameTreeObjects(std::string output);
  ~fillFrameTreeObjects();
  void init();
  void fillPixelHit(const mu3e::sim::vars_sihit_t &sihit, int i, SiDet::Hit *hit);
  void fillTracks(Segment::ptr_t segment);
  void fillFrame();
  
private:
  frameTree *fpFrameTree;
  calPixelQualityLM *fpCalPixelQualityLM;

  std::vector<pixelHit> fPixelHits;
  std::vector<track> fTracks;
  int fPixelHitIndex;
  int fTrackIndex;
};

#endif
