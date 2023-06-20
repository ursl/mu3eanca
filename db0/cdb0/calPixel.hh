#ifndef CALPIXEL_h
#define CALPIXEL_h

#include "calAbs.hh"

#include <string>
#include <vector>

// ----------------------------------------------------------------------
// (toy) pixel calibration class 
// ----------------------------------------------------------------------

class calPixel : public calAbs {
public:
  calPixel() = default;
  calPixel(cdb *db);
  calPixel(cdb *db, std::string tag);
  ~calPixel();

  std::string getName() override {return fPixelTag;}
  void        calculate() override;

private:
  std::string fPixelTag{"pixel_"};
};


#endif
