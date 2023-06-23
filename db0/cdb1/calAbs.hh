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

  void                setIOVs(std::vector<int> v) {fIOVs = v;}
	void                setVerbosity(int v) {fVerbose = v;}
  virtual std::string getName() {return std::string("blurp");}
  virtual std::string getHash() {return fHash;}
  virtual void        calculate(std::string hash) {std::cout << "calAbs::calculate() ?" << std::endl;}

  void                update(std::string hash);
  
protected: 
	cdbAbs                            *fDB;
  std::string                        fTag{"unset"}, fHash{"unset"};
  int                                fVerbose{0};
  std::vector<int>                   fIOVs;  // set it up initially
  std::map<std::string, payload>     fTagIOVPayloadMap;  // cache
};


#endif
