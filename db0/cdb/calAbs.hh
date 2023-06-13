#ifndef CALABS_h
#define CALABS_h

#include "cdb.hh"
#include "payload.hh"

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

	void                setVerbosity(int v) {fVerbose = v;}
  virtual std::string getName() {return std::string("blurp");}
  virtual void        calculate() {}

  std::string         getHash() {return fHash;}
  void                update();
  
protected: 
	cdb                                *fDB;
  std::string                        fTag;
  std::string                        fHash{"base_not_initialized"};
  int                                fVerbose{0};
  std::map<std::string, payload>     fTagIOVPayloadMap;  // cache
};


#endif
