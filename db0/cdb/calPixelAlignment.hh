#ifndef CALPIXELALIGNMENT_h
#define CALPIXELALIGNMENT_h

#include "calAbs.hh"

#include <string>
#include <vector>

// ----------------------------------------------------------------------
// (toy) pixel alignment class 
// ----------------------------------------------------------------------

class calPixelAlignment : public calAbs {
public:
  calPixelAlignment() = default;
  calPixelAlignment(cdb *db);
  calPixelAlignment(cdb *db, std::string gt);
  calPixelAlignment(cdb *db, std::string gt, std::string tag);
  ~calPixelAlignment();

private:
  std::string fPixelAlignmentTag{"pixelalignment_"};
};


#endif
