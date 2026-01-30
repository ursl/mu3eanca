#include "fillFrameTreeObjects.hh"

#include <iostream>
#include <string>

#include "track.hh"
#include "pixelHit.hh"
#include "frameTree.hh"

using namespace std;

// ---------------------------------------------------------------------- 
fillFrameTreeObjects::fillFrameTreeObjects(frameTree *frameTree) {
  fpFrameTree = frameTree;
  init();
}

// ---------------------------------------------------------------------- 
fillFrameTreeObjects::fillFrameTreeObjects(std::string output) {
  size_t pos1 = output.rfind("/") + 1;
  size_t pos2 = output.rfind("_run");

  if (pos1 != string::npos && pos2 != string::npos) {
      output.replace(pos1, pos2 - pos1, "frameTree");
  }
  cout << "output: " << output << endl;
  fpFrameTree = frameTree::instance(output); 

  init();
}


// ---------------------------------------------------------------------- 
fillFrameTreeObjects::~fillFrameTreeObjects() {
  cout << "fillFrameTreeObjects::~fillFrameTreeObjects() dtor" << endl;
  if (fpFrameTree) {
    fpFrameTree->saveTree();
    fpFrameTree->closeFile();
  }
}


// ---------------------------------------------------------------------- 
void fillFrameTreeObjects::init() {
  cout << "fillFrameTreeObjects::init() fpFrameTree = " << fpFrameTree << endl;
  fPixelHitIndex = 0;
  fTrackIndex = 0;

  Mu3eConditions *conditions = Mu3eConditions::instance();
  fpCalPixelQualityLM = dynamic_cast<calPixelQualityLM *>(conditions->getCalibration("pixelqualitylm_"));

  fpFrameTree->setRunAndFrameID(conditions->getRunNumber(), 
                              mu3e::conf.runtime.frame);

}

// ---------------------------------------------------------------------- 
void fillFrameTreeObjects::fillFrame() {
  cout << "fillFrameTreeObjects::fillFrame() fPixelHitIndex = " << fPixelHitIndex << endl;
  cout << "fillFrameTreeObjects::fillFrame() fTrackIndex = " << fTrackIndex << endl;
  fpFrameTree->fillFrame();
}

// ---------------------------------------------------------------------- 
void fillFrameTreeObjects::fillPixelHit(const mu3e::sim::vars_sihit_t &sihit, int i, SiDet::Hit *hit) {

  pixelHit PH;

  mu3e::timestamp_t time = { sihit.time[i], 0 };
  mu3e::id::sensor::hit hitId(sihit.pixelId[i]);

  PH.fPixelID = sihit.pixelId[i];
  PH.fChipID = hitId.sensorId.value();
  PH.fCol = hitId.col;
  PH.fRow = hitId.row;
  PH.fHitToT = sihit.tot[i];
  PH.fTimeInt = sihit.time[i];
  PH.fTime = time.ns();
  PH.fTimeNs = time.ns();
  
  if (hit) {
    PH.fX = hit->x;
    PH.fY = hit->y;
    PH.fZ = hit->z;
    PH.fValidHit = true;
  } else {
    PH.fX = -999.;
    PH.fY = -999.;
    PH.fZ = -999.;
    PH.fValidHit = false;
  }

  calPixelQualityLM::Status statusPix = fpCalPixelQualityLM->getStatus(PH.fChipID, PH.fCol, PH.fRow);
  PH.fStatus = static_cast<int>(statusPix);

  fpFrameTree->fillPixelHit(PH);
}

// ---------------------------------------------------------------------- 
void fillFrameTreeObjects::fillTracks(Segment::ptr_t segment) {
}
