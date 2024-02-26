#ifndef CDBABS_h
#define CDBABS_h

#include <string>
#include <vector>
#include <map>

#include "payload.hh"
#include "runRecord.hh"
#include "cfgPayload.hh"

class calAbs;

// ----------------------------------------------------------------------
// abstract base class for DB access and tag/IOV management
// ----------------------------------------------------------------------

class cdbAbs {
public:
  cdbAbs() = default;
  cdbAbs(std::string gt, std::string uri, int verbose);
	void init();
  ~cdbAbs();


  // -- access to runRecords
  virtual runRecord       getRunRecord(int irun) {return runRecord();}

  // -- access to configs
  virtual cfgPayload      getConfig(std::string hash) {return cfgPayload();}

  // -- access to payloads and IOVs
  //  virtual payload         getPayload(int irun, std::string t);
  virtual payload         getPayload(std::string hash) {return payload();}

	void setVerbosity(int v) {fVerbose = v;}

  void        setName(std::string n) {fName = n;}
  std::string getName() {return fName;}

	// -- utility functions
	void print(std::vector<int>, int istart = 0);
	void print(std::vector<std::string>, int istart = 0);
	void print(std::map<std::string, std::vector<std::string>>);
	void print(std::map<std::string, std::vector<int>>);

	virtual std::vector<std::string>                readGlobalTags() {return std::vector<std::string>();}
	virtual std::vector<std::string>                readTags(std::string gt) {return std::vector<std::string>();}
	virtual std::map<std::string, std::vector<int>> readIOVs(std::vector<std::string> tags) {return std::map<std::string, std::vector<int>>();}

protected:

  std::string fURI{"URI unset"};
  int fVerbose{0};

  // -- know what you are
  std::string fName{"cdbAbs base"};

};

#endif
