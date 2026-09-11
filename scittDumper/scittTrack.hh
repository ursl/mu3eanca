#ifndef SCITTTRACK_HH
#define SCITTTRACK_HH

#include <stdint.h>

// One truth particle that left at least one labelled Si hit in the frame.
// Used to filter training targets, not as transformer input.
struct scittTrack {
  scittTrack() = default;

  uint64_t frame_id = 0;
  int32_t run = 0;
  int32_t tid = 0;
  int32_t pid = 0;
  int32_t n_hits = 0;
  double px = 0, py = 0, pz = 0;
  double vx = 0, vy = 0, vz = 0;
};

#endif
