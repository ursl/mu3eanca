#ifndef CDB_h
#define CDB_h

#include <string>
#include <vector>
#include <map>

class calAbs;

// ----------------------------------------------------------------------
// abstract base class for DB access and tag/IOV management
// ----------------------------------------------------------------------

class cdb {
public:
  cdb() = default;
  cdb(std::string gt, std::string uri);
	void init();
  ~cdb();

	// -- access to metadata (in DB)
  std::string                                  getGlobalTag() {return fGT;}
  virtual std::vector<std::string>             getGlobalTags() {return fGlobalTags;}
  virtual std::vector<std::string>             getTags(std::string gt) {return fTags;}
  virtual std::vector<int>                     getIOVs(std::string t) {return fIOVs[t];}

  // -- access to payloads and IOVs
  virtual std::string                          getHash(int irun, std::string t);
  virtual std::string                          getPayload(int irun, std::string t) {return std::string();}
  virtual std::string                          getPayload(std::string hash) {return std::string();}

	void setGlobalTag(std::string gt) {fGT = gt;}
	void setVerbosity(int v) {fVerbose = v;}
  
	void setRunNumber(int);
	int  getRunNumber() {return fRunNumber;}

  void        setName(std::string n) {fName = n;}
  std::string getName() {return fName;}
  
  void registerCalibration(std::string tag, calAbs *c);

	// -- utility functions
	void print(std::vector<int>, int istart = 0);
	void print(std::vector<std::string>, int istart = 0);
	void print(std::map<std::string, std::vector<std::string>>);
	void print(std::map<std::string, std::vector<int>>);

protected: 
	virtual void readGlobalTags() {};
	virtual void readTags() {};
	virtual void readIOVs() {};

	int whichIOV(int run, std::string tag);
	
  std::string fGT{"GT unset"};
  std::string fURI{"URI unset"};

	// -- should be replaced by IOV class
	int         fRunNumber{-1};

  int fVerbose{0};
  
	// -- flag to indicate that all GTs have been read
  bool fValidGlobalTags{false}; 
	// -- all global tags in the DB
  std::vector<std::string> fGlobalTags;                    
	// -- for the set global tag
  std::vector<std::string> fTags;
  // -- list of IOVs for each tag for the set GT
  std::map<std::string, std::vector<int>> fIOVs;           

  // -- map of tag, calibration classes to be notified of update requirements
  std::map<std::string, calAbs*> fCalibrations;

  // -- know what you are
  std::string fName{"CDB base"};

};

#endif
