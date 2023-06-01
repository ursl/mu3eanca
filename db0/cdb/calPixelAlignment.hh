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
  calPixelAlignment(cdb *db, std::string tag);
  ~calPixelAlignment();

  std::string getName() override {return fPixelAlignmentTag;}
  void        calculate() override;

private:
  std::string fPixelAlignmentTag{"pixelalignment_"};
};


#endif
