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
frameTree* frameTree::instance(int mode, std::string filename) {
  if (0 == fInstance) {
    fInstance = new frameTree(mode, filename);
  }
  return fInstance;
}

// ----------------------------------------------------------------------
frameTree::frameTree(int mode, std::string filename) : fFilename(filename) {
  if (mode >= 0) {
    cout << "frameTree::frameTree() ctor" << endl;
    init(mode, filename); 
    return;
  } else {
    cout << "frameTree::frameTree(" << fFilename << ") ctor" << endl;
    init(mode, filename); 
  }

}

// ----------------------------------------------------------------------
void frameTree::init(int mode, std::string filename) {
  cout << "frameTree::init() mode = " << mode << endl;
  if (mode > 0) {
    fRun0 = mode;
    TDirectory *dir = gDirectory;
    fFile = TFile::Open((filename + "_run" + to_string(fRun0) + ".root").c_str(), "RECREATE");
    fFile->cd();
    fDirectory = gDirectory;
    fDirectory->ls();
 
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
    fHitsTree->Branch("hitRawToT", fHitRawToT, "hitRawToT[hitN]/I");
    fHitsTree->Branch("hitBitToT", fHitBitToT, "hitBitToT[hitN]/I");
    fHitsTree->Branch("hitStatus", fHitStatus, "hitStatus[hitN]/I");
    fHitsTree->Branch("hitStatusBits", fHitStatusBits, "hitStatusBits[hitN]/I");

    // -- initialize the hit tree variables
    fHitsN = -1;
    clearHitsTreeVariables();

    gDirectory = dir;
    gDirectory->ls();
  }
}


// ----------------------------------------------------------------------
frameTree::~frameTree() {
  cout << "frameTree::~frameTree() dtor" << endl;

  if (fFile) {
    fFile->Close();
    delete fFile;
  } else {
    
  }
}

// ---------------------------------------------------------------------- 
void frameTree::fillAnotherHit(pixelHit &hit) {
  fHitPixelID[fHitsN] = hit.fPixelID;
  fHitToT[fHitsN] = hit.fHitToT;
  fHitDebugSiData[fHitsN] = hit.fDebugSiData;
  fHitChipID[fHitsN] = hit.fChipID;
  fHitCol[fHitsN] = hit.fCol;
  fHitRow[fHitsN] = hit.fRow;
  fHitTime[fHitsN] = hit.fTime;
  fHitTimeNs[fHitsN] = hit.fTimeNs;
  fHitRawToT[fHitsN] = hit.fRawToT;
  fHitBitToT[fHitsN] = hit.fBitToT;
  fHitStatus[fHitsN] = hit.fStatus;
  fHitStatusBits[fHitsN] = hit.fStatusBits;
  fHitsN++;
}

// ---------------------------------------------------------------------- 
void frameTree::fillAnotherFrame(uint32_t frameID) {
  if (0) cout << "frameTree::fillAnotherFrame() fHitsN = " << fHitsN 
              << " frameID = " << frameID 
              << endl;
  fFrameID = frameID;
  fHitsTree->Fill();
  clearHitsTreeVariables();
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
  }
  fHitsN = 0;
}

// ---------------------------------------------------------------------- 
void frameTree::saveTree() {
  fHitsTree->SetDirectory(fDirectory);
  fHitsTree->Write();

  fFile->Close();
  delete fFile;

  if (fRun0 != fRun) {
    // Rename the output file from mode-based name to actual run number
    string oldName = fFilename + "_run" + to_string(fRun0) + ".root";
    string newName = fFilename + "_run" + to_string(fRun) + ".root";
    
    // Use system call to rename the file
    string renameCmd = "mv " + oldName + " " + newName;
    cout << "Renaming output file: " << renameCmd << endl;
    system(renameCmd.c_str());
  }
}
