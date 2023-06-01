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
  calPixel(cdb *db, std::string gt);
  calPixel(cdb *db, std::string gt, std::string tag);
  ~calPixel();

  std::string getPayload(int irun) override;
  
private:
  std::string fPixelTag{"pixel_"};
};


#endif
