#include "fillFrameTreeObjects.hh"

#include <iostream>
#include <string>

#include "watson/propagators/zhelix.hpp"
#include "watson/geometry/surfaces.hpp"


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

  Mu3eConditions *conditions = Mu3eConditions::instance();
  fpCalPixelQualityLM = dynamic_cast<calPixelQualityLM *>(conditions->getCalibration("pixelqualitylm_"));

  fpCalDetSetupV1 = dynamic_cast<calDetSetupV1 *>(conditions->getCalibration("detsetupv1_"));
  double fMagneticField = fpCalDetSetupV1->magnetFieldStrength();
  cout << "fillFrameTreeObjects::init() magnetic field: " << fMagneticField << endl;

  fpFrameTree->setRunAndFrameID(conditions->getRunNumber(), mu3e::conf.runtime.frame);

}

// ---------------------------------------------------------------------- 
void fillFrameTreeObjects::fillFrame() {
  cout << "fillFrameTreeObjects::fillFrame()" << endl;
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
void fillFrameTreeObjects::fillSegment(Segment::ptr_t segment) {
  track TRK;

  TRK.fTrkChi2 = segment->chi2;
  TRK.fTrkType = segment->type;

  // -- this is only pT
  double r = 1./segment->k;
  TRK.fTrkMomentum = r * double(mu3e::conf.B);
  TRK.fTrkK = segment->k;
  TRK.fTrkKerr2 = segment->kerr2;

  TRK.fTrkPhi = segment->tan(0);
  TRK.fTrkLambda = segment->lam(0);

  auto t_ = segment->time();
  auto t_si = segment->si_times();
  auto t_si2 = segment->si_times();

  TRK.fTrkT0 = t_.ns();
  TRK.fTrkT0Err = t_.ns_sigma();
  TRK.fTrkT0RMS = t_.ns_rms();
  TRK.fTrkT0Si = t_si.ns();
  TRK.fTrkT0SiErr = t_si.ns_sigma();
  TRK.fTrkT0SiRMS = t_si.ns_rms();

  TRK.fTrkDoca = calculateDCAtoDoubleCone(*segment, 19.0, 25.0, fMagneticField);

  for (int i = 0; i < segment->n; i++) {
    SiDet::Hit* hit_before = segment->hits[i][0];
    std::cout << "  hit " ;
    int idx = fpFrameTree->findHitIndex(hit_before->id.value());
    if (0)std::cout << idx << "  ";
    if (1) std::cout << " idx " << idx
             << "  chip/col/row = " << std::setw(4) << hit_before->id.sensorId.value() << "/"
             << std::setw(3) << hit_before->id.col << "/" << std::setw(3) << hit_before->id.row 
             << " time = " << hit_before->time.ns()
             << i << ": x,y,z = " << hit_before->x << ", " << hit_before->y << ", " << hit_before->z << " "
             << std::endl;
    //continue;
    bool ok = TRK.fillIndex(idx);
    if (!ok) {
        if (0) std::cout << "trirec - failed to fill hit index " << idx 
                  << " where segment-> n = " << segment->n 
                  << std::endl;
    }
 }

  fpFrameTree->fillTrack(TRK);



}

// ---------------------------------------------------------------------- 
// -- cursor generated
double fillFrameTreeObjects::calculateDCAtoDoubleCone(const Segment& segment, 
      double cone_radius, 
      double cone_half_length,
      double B_field) {

  // Get track parameters
  auto track_params = segment.params_before(0).to_global(1);

  // Create double cone target
  watson::ZDoubleCone doubleCone(cone_radius, cone_half_length);

  // Create propagator
  watson::ZHelixPropagator propagator(B_field);

  // Propagate to double cone
  auto result = propagator.propagate_to(track_params, doubleCone);

  if (result.error()) {
      return -1.0;  // Error case
  }

  auto intersection_params = result.get<0>();
  double path_length = result.get<1>();

  // Distance from original position to intersection
  double distance = (intersection_params.position() - track_params.position()).norm();

  return distance;
}

// ---------------------------------------------------------------------- 
void fillFrameTreeObjects::fillSegments(const Frame *frame) {
  for(auto& [_, segs] : frame->ttype2segs) {
  // [AK] TODO: frame->tracks[0] == MCTrack::FAKE
    for(auto& s : segs) {
        int tid = s->tid();
        if(tid == 0) continue;
        fillSegment(s);
        if(mu3e::conf.verbose >= 1 && frame->tracks.find(tid) == frame->tracks.end()) {
            MU3E_WARN("no MCTrack with tid = %d\n", tid);
        }
    }
  } 
}

