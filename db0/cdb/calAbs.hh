#ifndef CALABS_h
#define CALABS_h

#include "cdb.hh"

#include <string>
#include <vector>
#include <map>

// ----------------------------------------------------------------------
// abstract base class for calibration classes
// ----------------------------------------------------------------------

class calAbs {
public:
  calAbs() = default;
  calAbs(cdb *db);
  calAbs(cdb *db, std::string tag);
  ~calAbs();

  virtual std::string getName() {return std::string("blurp");}
  virtual void        calculate() {}

  std::string         getHash() {return fHash;}
  void                update();
  
protected: 
	cdb                                *fDB;
  std::string                        fTag;
  std::string                        fHash{"base"};
  std::map<std::string, std::string> fTagIOVPayloadMap;  // cache
};


#endif
