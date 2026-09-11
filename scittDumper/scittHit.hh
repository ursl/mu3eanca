#ifndef SCITTHIT_HH
#define SCITTHIT_HH

#include <stdint.h>

// One reconstructed silicon pixel. Model input is the reconstructed
// quantities; tid/hid/mc_* are training labels / debug, not features.
struct scittHit {
  scittHit() = default;

  uint64_t frame_id = 0;
  int32_t run = 0;
  uint32_t id = 0;          // packed sensor/row/col
  uint32_t sensor_id = 0;
  uint32_t row = 0;
  uint32_t col = 0;
  int32_t layer = -1;
  int32_t tot = 0;
  uint32_t ts = 0;
  double x = 0, y = 0, z = 0;
  double r = 0, phi = 0;
  double time = 0;

  // labels (v1: mcs.front(); tid==0 is noise / unmatched)
  int32_t tid = 0;
  int32_t hid = 0;
  int32_t abs_hid = 0;
  int32_t n_mc = 0;

  // truth position of the labelled MC hit; not a model input
  double mc_x = 0, mc_y = 0, mc_z = 0;
};

#endif
