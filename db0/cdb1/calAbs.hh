#ifndef CALABS_h
#define CALABS_h

#include "cdbAbs.hh"
#include "payload.hh"

#include <iostream>
#include <string>
#include <vector>
#include <map>

// ----------------------------------------------------------------------
// abstract base class for calibration classes
// ----------------------------------------------------------------------

class calAbs {
public:
  calAbs() = default;
  calAbs(cdbAbs *db);
  calAbs(cdbAbs *db, std::string tag);
  ~calAbs();

	void                setVerbosity(int v) {fVerbose = v;}
  virtual std::string getName() {return std::string("blurp");}
  virtual void        calculate() {std::cout << "calAbs::calculate() wrong function" << std::endl;}

  std::string         getHash() {return fHash;}
  void                update();
  
protected: 
	cdbAbs                            *fDB;
  std::string                        fTag;
  std::string                        fHash{"base_not_initialized"};
  int                                fVerbose{0};
  std::map<std::string, payload>     fTagIOVPayloadMap;  // cache
};


#endif
