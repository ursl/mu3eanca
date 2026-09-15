#ifndef SCITTTREE_HH
#define SCITTTREE_HH

#include "scittHit.hh"
#include "scittTrack.hh"

#include <string>
#include <cstdint>

class TFile;
class TTree;

// Flat ROOT dump: one `hits` row per reconstructed pixel, one `tracks`
// row per labelled truth particle. Group by frame_id in the loader.
class scittTree {
public:
  explicit scittTree(const std::string& filename);
  ~scittTree();

  scittTree(const scittTree&) = delete;
  scittTree& operator=(const scittTree&) = delete;

  void fillHit(const scittHit& hit);
  void fillTrack(const scittTrack& trk);
  void write();
  void close();

  const std::string& filename() const { return fFilename; }
  int64_t nHits() const { return fNHits; }
  int64_t nTracks() const { return fNTracks; }

private:
  void initBranches();

  std::string fFilename;
  TFile* fFile = nullptr;
  TTree* fHitsTree = nullptr;
  TTree* fTracksTree = nullptr;
  scittHit fHit;
  scittTrack fTrk;
  int64_t fNHits = 0;
  int64_t fNTracks = 0;
};

#endif
