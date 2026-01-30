#ifndef FILLFRAMETREEOBJECTS_HH
#define FILLFRAMETREEOBJECTS_HH

#include "Segment.h"
#include "Frame.h"

#include "frameTree.hh"
#include "track.hh"
#include "pixelHit.hh"

#include "mu3e/cdb/Mu3eConditions.hh"
#include "mu3e/cdb/calPixelQualityLM.hh"
#include "mu3e/cdb/calDetSetupV1.hh"

// ---------------------------------------------------------------------- 
// The purpose of this class is to completely minimize interference in 
// trirec.h for writing my own frameTree.
// 
// This class is NOT to be compiled in mu3eanca, but is to be sym-linked to 
// from .../mu3eTrirec/src/mu3e/rec with ../scripts/mkAnaLinks
// ---------------------------------------------------------------------- 

// ---------------------------------------------------------------------- 
class fillFrameTreeObjects {
public:
  fillFrameTreeObjects(frameTree *frameTree);
  fillFrameTreeObjects(std::string output);
  ~fillFrameTreeObjects();
  void init();
  void fillPixelHit(const mu3e::sim::vars_sihit_t &sihit, int i, SiDet::Hit *hit);

  void fillSegments(const Frame *frame);
  void fillSegment(Segment::ptr_t segment);

  void fillFrame();

  double calculateDCAtoDoubleCone(const Segment& segment, double cone_r, double cone_hl, double B);
  

private:
  frameTree *fpFrameTree;
  calPixelQualityLM *fpCalPixelQualityLM;
  calDetSetupV1 *fpCalDetSetupV1;

  std::vector<pixelHit> fPixelHits;
  std::vector<track> fTracks;
  double fMagneticField;
};

#endif
