#ifndef MU3ECONDITIONS_h
#define MU3ECONDITIONS_h

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
  static Mu3eConditions* instance(std::string gt = "", cdbAbs *db = 0);

	// -- access to metadata (from DB)
  std::string                                  getGlobalTag() {return fGT;}
  virtual std::vector<std::string>             getGlobalTags() {return fGlobalTags;}
  virtual std::vector<std::string>             getTags() {return fTags;}
  virtual std::vector<int>                     getIOVs(std::string t) {return fIOVs[t];}

  calAbs* createClass(std::string name);
  calAbs* createClass(std::string name, std::string tag);
  calAbs* createClassWithDB(std::string name, std::string tag, cdbAbs *db);

  cdbAbs* getDB()               {return fDB;}
  void    setDB(cdbAbs *db)     {fDB = db;}
  void    setVerbosity(int v)   {fVerbose = v;}
  void    setPrintTiming(int v) {fPrintTiming = v;}

	void setRunNumber(int);
	int  getRunNumber() {return fRunNumber;}
 
  void    registerCalibration(std::string tag, calAbs *c);
	void    printCalibrations();
  calAbs* getCalibration(std::string name);
  std::map<std::string, calAbs*> getCalibrations() {return fCalibrations;}
  
  std::string  getHash(int irun, std::string tag);
  int          whichIOV(int run, std::string tag);
  
protected:
  Mu3eConditions(std::string gt, cdbAbs *);
  ~Mu3eConditions();
  
private:
  static Mu3eConditions* fInstance; 
  cdbAbs *fDB;
  std::string fGT{"GT unset"};
  int fVerbose{0}, fPrintTiming{0};

  // -- should be replaced by IOV class
  int         fRunNumber{-1};

	// -- all global tags in the DB
  std::vector<std::string> fGlobalTags;                    
	// -- for the set global tag
  std::vector<std::string> fTags;
  // -- list of IOVs for each tag for the set GT
  std::map<std::string, std::vector<int>> fIOVs;           

  // -- map of tag, calibration classes to be notified of update requirements
  std::map<std::string, calAbs*> fCalibrations;
};

#endif

