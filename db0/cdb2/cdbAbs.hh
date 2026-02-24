#ifndef CDBABS_h
#define CDBABS_h

#include "payload.hh"
#include "runRecord.hh"
#include "cfgPayload.hh"

#include <string>
#include <vector>
#include <map>

class calAbs;

// ----------------------------------------------------------------------
// abstract base class for DB access and tag/IOV management
// ----------------------------------------------------------------------

class cdbAbs {
public:
  cdbAbs() = default;
  cdbAbs(std::string uri, int verbose);
  void init();
  virtual ~cdbAbs();


  // -- access to runRecords
  virtual runRecord                getRunRecord(int irun) = 0;
  // -- all that are there (more from rest/mongodb than from JSON, usually)
  virtual std::vector<std::string> getAllRunNumbers() = 0;
  // -- all that match "class" selection and detector (csv list vtx,pix,fib,til)
  virtual std::vector<std::string> getAllRunNumbers(std::string selection, std::string det = "") = 0;

  // -- access to configs
  virtual cfgPayload      getConfig(std::string hash) = 0;

  // -- access to payloads and IOVs
  //  virtual payload         getPayload(int irun, std::string t);
  virtual payload         getPayload(std::string hash) = 0;

  void setVerbosity(int v) {fVerbose = v;}

  void        setName(std::string n) {fName = n;}
  std::string getName() {return fName;}

  // -- utility functions
  void print(std::vector<int>, int istart = 0);
  void print(std::vector<std::string>, int istart = 0);
  void print(std::map<std::string, std::vector<std::string>>);
  void print(std::map<std::string, std::vector<int>>);

  virtual std::vector<std::string>                readGlobalTags() = 0;
  virtual std::vector<std::string>                readTags(std::string gt) = 0;
  virtual std::map<std::string, std::vector<int>> readIOVs(std::vector<std::string> tags) = 0;

protected:

  std::string fURI{"URI unset"};
  int fVerbose{0};

  // -- know what you are
  std::string fName{"cdbAbs base"};

};

#endif
