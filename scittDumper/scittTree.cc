#include "scittTree.hh"

#include <iostream>

#include <TFile.h>
#include <TTree.h>

scittTree::scittTree(const std::string& filename)
  : fFilename(filename)
{
  std::cout << "scittTree: writing " << fFilename << std::endl;
  fFile = TFile::Open(fFilename.c_str(), "RECREATE");
  if (!fFile || fFile->IsZombie()) {
    std::cerr << "scittTree: cannot create " << fFilename << std::endl;
    return;
  }
  fFile->cd();

  fHitsTree = new TTree("hits", "reconstructed Si hits (one row per pixel)");
  fTracksTree = new TTree("tracks", "truth particles with Si hits in the frame");
  initBranches();
  fHitsTree->SetAutoSave(1000000000);
  fTracksTree->SetAutoSave(1000000000);
}

void scittTree::initBranches() {
  fHitsTree->Branch("frame_id", &fHit.frame_id, "frame_id/l");
  fHitsTree->Branch("run", &fHit.run, "run/I");
  fHitsTree->Branch("id", &fHit.id, "id/i");
  fHitsTree->Branch("sensor_id", &fHit.sensor_id, "sensor_id/i");
  fHitsTree->Branch("row", &fHit.row, "row/i");
  fHitsTree->Branch("col", &fHit.col, "col/i");
  fHitsTree->Branch("layer", &fHit.layer, "layer/I");
  fHitsTree->Branch("tot", &fHit.tot, "tot/I");
  fHitsTree->Branch("ts", &fHit.ts, "ts/i");
  fHitsTree->Branch("x", &fHit.x, "x/D");
  fHitsTree->Branch("y", &fHit.y, "y/D");
  fHitsTree->Branch("z", &fHit.z, "z/D");
  fHitsTree->Branch("r", &fHit.r, "r/D");
  fHitsTree->Branch("phi", &fHit.phi, "phi/D");
  fHitsTree->Branch("time", &fHit.time, "time/D");
  fHitsTree->Branch("tid", &fHit.tid, "tid/I");
  fHitsTree->Branch("hid", &fHit.hid, "hid/I");
  fHitsTree->Branch("abs_hid", &fHit.abs_hid, "abs_hid/I");
  fHitsTree->Branch("n_mc", &fHit.n_mc, "n_mc/I");
  fHitsTree->Branch("mc_x", &fHit.mc_x, "mc_x/D");
  fHitsTree->Branch("mc_y", &fHit.mc_y, "mc_y/D");
  fHitsTree->Branch("mc_z", &fHit.mc_z, "mc_z/D");

  fTracksTree->Branch("frame_id", &fTrk.frame_id, "frame_id/l");
  fTracksTree->Branch("run", &fTrk.run, "run/I");
  fTracksTree->Branch("tid", &fTrk.tid, "tid/I");
  fTracksTree->Branch("pid", &fTrk.pid, "pid/I");
  fTracksTree->Branch("n_hits", &fTrk.n_hits, "n_hits/I");
  fTracksTree->Branch("px", &fTrk.px, "px/D");
  fTracksTree->Branch("py", &fTrk.py, "py/D");
  fTracksTree->Branch("pz", &fTrk.pz, "pz/D");
  fTracksTree->Branch("vx", &fTrk.vx, "vx/D");
  fTracksTree->Branch("vy", &fTrk.vy, "vy/D");
  fTracksTree->Branch("vz", &fTrk.vz, "vz/D");
}

void scittTree::fillHit(const scittHit& hit) {
  if (!fHitsTree) return;
  fHit = hit;
  fHitsTree->Fill();
  ++fNHits;
}

void scittTree::fillTrack(const scittTrack& trk) {
  if (!fTracksTree) return;
  fTrk = trk;
  fTracksTree->Fill();
  ++fNTracks;
}

void scittTree::write() {
  if (!fFile) return;
  fFile->cd();
  if (fHitsTree) fHitsTree->Write();
  if (fTracksTree) fTracksTree->Write();
  std::cout << "scittTree: wrote " << fNHits << " hits, " << fNTracks
            << " tracks to " << fFilename << std::endl;
}

void scittTree::close() {
  write();
  if (fFile) {
    fFile->Close();
    delete fFile;
    fFile = nullptr;
    fHitsTree = nullptr;
    fTracksTree = nullptr;
  }
}

scittTree::~scittTree() {
  if (fFile) close();
}
