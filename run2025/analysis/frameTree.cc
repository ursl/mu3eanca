#include "frameTree.hh"

#include <_types/_uint32_t.h>
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
  fHitsTree->Branch("hitPixelID", fHitPixelID, "hitPixelID[hitN]/I");
  fHitsTree->Branch("hitToT", fHitToT, "hitToT[hitN]/I");
  fHitsTree->Branch("hitDebugSiData", fHitDebugSiData, "hitDebugSiData[hitN]/l");
  fHitsTree->Branch("hitChipID", fHitChipID, "hitChipID[hitN]/I");
  fHitsTree->Branch("hitCol", fHitCol, "hitCol[hitN]/I");
  fHitsTree->Branch("hitRow", fHitRow, "hitRow[hitN]/I");
  fHitsTree->Branch("hitTime", fHitTime, "hitTime[hitN]/I");
  fHitsTree->Branch("hitTimeNs", fHitTimeNs, "hitTimeNs[hitN]/I");
  fHitsTree->Branch("hitX", fHitX, "hitX[hitN]/F");
  fHitsTree->Branch("hitY", fHitY, "hitY[hitN]/F");
  fHitsTree->Branch("hitZ", fHitZ, "hitZ[hitN]/F");
  fHitsTree->Branch("hitRawToT", fHitRawToT, "hitRawToT[hitN]/I");
  fHitsTree->Branch("hitBitToT", fHitBitToT, "hitBitToT[hitN]/I");
  fHitsTree->Branch("hitStatus", fHitStatus, "hitStatus[hitN]/I");
  fHitsTree->Branch("hitStatusBits", fHitStatusBits, "hitStatusBits[hitN]/I");
  fHitsTree->Branch("hitValidHit", fHitValidHit, "hitValidHit[hitN]/O");

  // -- track tree
  fHitsTree->Branch("trkN", &fTrkN, "trkN/I");
  fHitsTree->Branch("trkMomentum", fTrkMomentum, "trkMomentum[trkN]/F");
  fHitsTree->Branch("trkChi2", fTrkChi2, "trkChi2[trkN]/F");
  fHitsTree->Branch("trkType", fTrkType, "trkType[trkN]/I");
  fHitsTree->Branch("trkPhi", fTrkPhi, "trkPhi[trkN]/F");
  fHitsTree->Branch("trkLambda", fTrkLambda, "trkLambda[trkN]/F");
  fHitsTree->Branch("trkK", fTrkK, "trkK[trkN]/F");
  fHitsTree->Branch("trkKerr2", fTrkKerr2, "trkKerr2[trkN]/F");
  fHitsTree->Branch("trkDoca", fTrkDoca, "trkDoca[trkN]/F");
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
  if (0) cout << "frameTree::fillPixelHit() fHitsN = " << fHitsN 
       << " fPixelID = " << hit.fPixelID 
       << " fHitToT = " << hit.fHitToT 
       << " fDebugSiData = " << hit.fDebugSiData 
       << " fChipID = " << hit.fChipID 
       << " fCol = " << hit.fCol 
       << " fRow = " << hit.fRow 
       << " fTime = " << hit.fTime 
       << " fTimeNs = " << hit.fTimeNs 
       << " fRawToT = " << hit.fRawToT 
       << " fBitToT = " << hit.fBitToT 
       << " fStatus = " << hit.fStatus 
       << " fStatusBits = " << hit.fStatusBits 
       << endl;

  if (fHitsN == NHITMAX) {
    cout << "frameTree::fillPixelHit() fHitsN == NHITMAX  ...  NOT FILLING HITS ANYMORE" << endl;
    return;
  }

  fHitPixelID[fHitsN] = hit.fPixelID;
  fHitToT[fHitsN] = hit.fHitToT;
  fHitDebugSiData[fHitsN] = hit.fDebugSiData;
  fHitChipID[fHitsN] = hit.fChipID;
  fHitCol[fHitsN] = hit.fCol;
  fHitRow[fHitsN] = hit.fRow;
  fHitTime[fHitsN] = hit.fTime;
  fHitTimeNs[fHitsN] = hit.fTimeNs;
  fHitX[fHitsN] = hit.fX;
  fHitY[fHitsN] = hit.fY;
  fHitZ[fHitsN] = hit.fZ;
  fHitRawToT[fHitsN] = hit.fRawToT;
  fHitBitToT[fHitsN] = hit.fBitToT;
  fHitStatus[fHitsN] = hit.fStatus;
  fHitStatusBits[fHitsN] = hit.fStatusBits;
  fHitValidHit[fHitsN] = hit.fValidHit;
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
  
  clearHitsTreeVariables();
  clearTrackTreeVariables();
}

// ---------------------------------------------------------------------- 
void frameTree::clearHitsTreeVariables() {
  if (fHitsN < 0) fHitsN = NHITMAX;
  for (int i = 0; i < fHitsN; ++i) {
    fHitPixelID[i] = 0;
    fHitToT[i] = 0;
    fHitDebugSiData[i] = 0;
    fHitChipID[i] = 0;
    fHitCol[i] = 0;
    fHitRow[i] = 0;
    fHitTime[i] = 0;
    fHitTimeNs[i] = 0;
    fHitRawToT[i] = 0;
    fHitBitToT[i] = 0;
    fHitStatus[i] = 0;
    fHitStatusBits[i] = 0;
    fHitValidHit[i] = false;
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
    fTrkDoca[i] = 0;
    fTrkSegmentN[i] = 0;
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
int frameTree::findHitIndex(uint32_t pixelID) {
  for (int i = 0; i < fHitsN; ++i) {
    if (fHitPixelID[i] == pixelID) {
      return i;
    }
  }
  return -1;
}
