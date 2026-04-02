#include "frameTree.hh"

#include <iostream>
#include <sys/stat.h>

#include <TDirectory.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TPad.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TStyle.h>
#include <TFile.h>

using namespace std;

frameTree* frameTree::fInstance = 0;

// ----------------------------------------------------------------------
frameTree::frameTree(std::string filename) : fFilename(filename) {
  cout << "frameTree::frameTree(" << fFilename << ") ctor" << endl;
  init(filename); 
}

// ----------------------------------------------------------------------
frameTree* frameTree::instance(std::string filename) {
  if (0 == fInstance) {
    fInstance = new frameTree(filename);
  }
  return fInstance;
}

// ----------------------------------------------------------------------
void frameTree::init(std::string filename) {
  cout << "frameTree::init() filename = " << filename << endl;
  TDirectory *dir = gDirectory;
  fFile = TFile::Open(filename.c_str(), "RECREATE");
  fFile->cd();
  fDirectory = gDirectory;
  // fDirectory->ls();

  // -- hit tree very brute force and simple-minded    
  fHitsTree = new TTree("frameTree", "frameTree");
  fHitsTree->Branch("run", &fRun, "run/I");
  fHitsTree->Branch("frameID", &fFrameID);
  // -- pixel hits
  fHitsTree->Branch("hitN", &fHitsN, "hitN/I");
  fHitsTree->Branch("hitID", fHitID, "hitID[hitN]/U");
  fHitsTree->Branch("hitTS", fHitTS, "hitTS[hitN]/U");
  fHitsTree->Branch("hitRawToT", fHitRawToT, "hitRawToT[hitN]/U");
  fHitsTree->Branch("hitStatus", fHitStatus, "hitStatus[hitN]/U");
  fHitsTree->Branch("hitFrameID", fHitFrameID, "hitFrameID[hitN]/U");
  fHitsTree->Branch("hitX", fHitX, "hitX[hitN]/D");
  fHitsTree->Branch("hitY", fHitY, "hitY[hitN]/D");
  fHitsTree->Branch("hitZ", fHitZ, "hitZ[hitN]/D");
  fHitsTree->Branch("hitTime", fHitTime, "hitTime[hitN]/D");
  // -- track tree
  fHitsTree->Branch("trkN", &fTrkN, "trkN/I");
  fHitsTree->Branch("trkMomentum", fTrkMomentum, "trkMomentum[trkN]/D");
  fHitsTree->Branch("trkChi2", fTrkChi2, "trkChi2[trkN]/D");
  fHitsTree->Branch("trkType", fTrkType, "trkType[trkN]/I");
  fHitsTree->Branch("trkPhi", fTrkPhi, "trkPhi[trkN]/D");
  fHitsTree->Branch("trkLambda", fTrkLambda, "trkLambda[trkN]/D");
  fHitsTree->Branch("trkK", fTrkK, "trkK[trkN]/D");
  fHitsTree->Branch("trkKerr2", fTrkKerr2, "trkKerr2[trkN]/D");
  fHitsTree->Branch("trkT0", fTrkT0, "trkT0[trkN]/D");
  fHitsTree->Branch("trkT0Err", fTrkT0Err, "trkT0Err[trkN]/D");
  fHitsTree->Branch("trkT0RMS", fTrkT0RMS, "trkT0RMS[trkN]/D");  
  fHitsTree->Branch("trkT0Si", fTrkT0Si, "trkT0Si[trkN]/D");
  fHitsTree->Branch("trkT0SiErr", fTrkT0SiErr, "trkT0SiErr[trkN]/D");
  fHitsTree->Branch("trkT0SiRMS", fTrkT0SiRMS, "trkT0SiRMS[trkN]/D");
  fHitsTree->Branch("trkDoca", fTrkDoca, "trkDoca[trkN]/D");
  fHitsTree->Branch("trkSegmentN", fTrkSegmentN, "trkSegmentN[trkN]/I");
  fHitsTree->Branch("trkHitOverflow", fTrkHitOverflow, "trkHitOverflow[trkN]/I");
  fHitsTree->Branch("trkNhits", fTrkNhits, "trkNhits[trkN]/I");
  fHitsTree->Branch("trkHitIndices", fTrkHitIndices, "trkHitIndices[trkN][20]/I");

  // -- initialize the hit tree variables
  fHitsN = -1;
  clearHitsTreeVariables();
  fTrkN = -1;
  clearTrackTreeVariables();

  gDirectory = dir;
  //  gDirectory->ls();
}


// ----------------------------------------------------------------------
frameTree::~frameTree() {
  cout << "frameTree::~frameTree() dtor" << endl;
  if (fFile) {
    fFile->Close();
    delete fFile;
  } 
}

// ---------------------------------------------------------------------- 
void frameTree::fillPixelHit(pixelHit &hit) {

  if (fHitsN == NHITMAX) {
    cout << "frameTree::fillPixelHit() fHitsN == NHITMAX  ...  NOT FILLING HITS ANYMORE" << endl;
    return;
  }

  fHitID[fHitsN] = hit.fID;
  fHitTime[fHitsN] = hit.fTime;
  fHitRawToT[fHitsN] = hit.fRawToT;
  fHitStatus[fHitsN] = hit.fStatus;
  fHitX[fHitsN] = hit.fX;
  fHitY[fHitsN] = hit.fY;
  fHitZ[fHitsN] = hit.fZ;
  fHitsN++;
}

// ---------------------------------------------------------------------- 
void frameTree::fillTrack(track &trk) {
  if (fTrkN == NTRKMAX) {
    cout << "frameTree::fillTrack() fTrkN == NTRKMAX  ...  NOT FILLING TRACKS ANYMORE" << endl;
    return;
  }

  fTrkMomentum[fTrkN] = trk.fTrkMomentum;
  fTrkChi2[fTrkN] = trk.fTrkChi2;
  fTrkType[fTrkN] = trk.fTrkType;
  fTrkPhi[fTrkN] = trk.fTrkPhi;
  fTrkLambda[fTrkN] = trk.fTrkLambda;
  fTrkK[fTrkN] = trk.fTrkK;
  fTrkKerr2[fTrkN] = trk.fTrkKerr2;
  fTrkT0[fTrkN] = trk.fTrkT0;
  fTrkT0Err[fTrkN] = trk.fTrkT0Err;
  fTrkT0RMS[fTrkN] = trk.fTrkT0RMS;
  fTrkT0Si[fTrkN] = trk.fTrkT0Si;
  fTrkT0SiErr[fTrkN] = trk.fTrkT0SiErr;
  fTrkT0SiRMS[fTrkN] = trk.fTrkT0SiRMS;
  if (0) std::cout << "frameTree::fillTrack() sit0 = " << fTrkT0Si[fTrkN] 
                   << "  sit0err = " << fTrkT0SiErr[fTrkN] 
                   << "  sit0rms = " << fTrkT0SiRMS[fTrkN] 
                   << "  Momentum = " << fTrkMomentum[fTrkN]
                   << std::endl;
  fTrkDoca[fTrkN] = trk.fTrkDoca;
  fTrkNhits[fTrkN] = trk.fTrkNhits;
  for (int i = 0; i < fTrkNhits[fTrkN]; ++i) {
    fTrkHitIndices[fTrkN][i] = trk.fTrkHitIndices[i];
  }
  fTrkN++;
}

// ---------------------------------------------------------------------- 
void frameTree::fillFrame() {
  if (0) cout << "frameTree::fillFrame() fHitsN = " << fHitsN 
              << " frameID = " << fFrameID 
              << endl;
  fHitsTree->Fill();
  if (fTrkN > 0) cout << "frameTree::fillFrame() fHitsN = " << fHitsN 
       << " fTrkN = " << fTrkN 
       << endl;
  clearHitsTreeVariables();
  clearTrackTreeVariables();
}

// ---------------------------------------------------------------------- 
void frameTree::clearHitsTreeVariables() {
  if (fHitsN < 0) fHitsN = NHITMAX;
  for (int i = 0; i < fHitsN; ++i) {
    fHitID[i] = 0;
    fHitRawToT[i] = 0;
    fHitTime[i] = 0;
    fHitStatus[i] = 0;
    fHitX[i] = 0;
    fHitY[i] = 0;
    fHitZ[i] = 0;
  }
  fHitsN = 0;
}

// ---------------------------------------------------------------------- 
void frameTree::clearTrackTreeVariables() {
  if (fTrkN < 0) fTrkN = NTRKMAX;
  for (int i = 0; i < fTrkN; ++i) {
    fTrkMomentum[i] = 0;
    fTrkChi2[i] = 0;
    fTrkType[i] = 0;
    fTrkPhi[i] = 0;
    fTrkLambda[i] = 0;
    fTrkK[i] = 0;
    fTrkKerr2[i] = 0;
    fTrkT0[i] = 0;
    fTrkT0Err[i] = 0;
    fTrkT0RMS[i] = 0;
    fTrkT0Si[i] = 0;
    fTrkT0SiErr[i] = 0;
    fTrkT0SiRMS[i] = 0;
    fTrkDoca[i] = 0;
    fTrkSegmentN[i] = 0;
    fTrkHitOverflow[i] = 0;
    fTrkNhits[i] = 0;
    for (int j = 0; j < TRKHITMAX; ++j) {
      fTrkHitIndices[i][j] = -1;
    }
  }
  fTrkN = 0;
}

// ---------------------------------------------------------------------- 
void frameTree::saveTree() {
  cout << "frameTree::saveTree() fDirectory->GetName() = " << fDirectory->GetName() << endl;
  fFile->cd();
  cout << "frameTree::saveTree() fDirectory->ls(): " << endl;
  //fDirectory->ls();
  fFile->cd(fDirectory->GetName());
  cout << "frameTree::saveTree() fDirectory->ls(): " << endl;
  //fDirectory->ls();
  fFile->cd();
  cout << "frameTree::saveTree() fDirectory->ls(): " << endl;
  //fDirectory->ls();
  fHitsTree->SetDirectory(fDirectory);
  fHitsTree->Write();
}

// ---------------------------------------------------------------------- 
void frameTree::closeFile() {
  fFile->Close();
  delete fFile;
}

// ---------------------------------------------------------------------- 
int frameTree::findHitIndex(uint32_t hitID) {
  for (int i = 0; i < fHitsN; ++i) {
    if (fHitID[i] == hitID) {
      return i;
    }
  }
  return -1;
}
