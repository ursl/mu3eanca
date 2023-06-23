#ifndef MU3CONDITIONS_h
#define MU3CONDITIONS_h

#include <string>
#include <map>

class cdbAbs;
class calAbs;
class cdb;

// ----------------------------------------------------------------------
// Entry point to access/insert/manage all Mu3e conditions data 
// ----------------------------------------------------------------------

class Mu3eConditions {
public:
  static Mu3eConditions* instance(cdbAbs *db = 0);
  calAbs* createClass(std::string name);
  calAbs* createClass(std::string name, std::string tag);
  calAbs* createClassWithDB(std::string name, std::string tag, cdbAbs *db);

  cdbAbs* getDB()             {return fDB;}
  void    setDB(cdbAbs *db)   {fDB = db;}
  void    setVerbosity(int v) {fVerbose = v;}

	void setRunNumber(int);
	int  getRunNumber() {return fRunNumber;}
 
  void    registerCalibration(std::string tag, calAbs *c);
	void    printCalibrations();
  calAbs* getCalibration(std::string name);
  
protected:
  Mu3eConditions(cdbAbs *);
  ~Mu3eConditions();

private:
  static Mu3eConditions* fInstance; 
  cdbAbs *fDB;
  int fVerbose{0};

  // -- should be replaced by IOV class
  int         fRunNumber{-1};

  // -- map of tag, calibration classes to be notified of update requirements
  std::map<std::string, calAbs*> fCalibrations;

};

#endif

