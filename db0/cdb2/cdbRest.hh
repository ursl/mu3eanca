#ifndef CDBREST_h
#define CDBREST_h

#include "cdbAbs.hh"

// ----------------------------------------------------------------------
// implementation class for a RESTful access of payloads
// ----------------------------------------------------------------------

class cdbRest: public cdbAbs {
public:
  cdbRest() = default;
  cdbRest(std::string uri, int verbose);
  ~cdbRest();

  void                 init();
  payload              getPayload(std::string hash) override;
  cfgPayload           getConfig(std::string hash) override;

  std::vector<std::string>                 readGlobalTags() override;
  std::vector<std::string>                 readTags(std::string gt) override;
  std::map<std::string, std::vector<int> > readIOVs(std::vector<std::string> tags) override;

  // -- access to runRecords
  runRecord            getRunRecord(int irun) override;
  // -- all that are there (more from rest/mongodb than from JSON, usually)
  std::vector<std::string>                 getAllRunNumbers() override;
  // -- all that match "class" selection and detector (csv list vtx,pix,fib,til)
  std::vector<std::string>                 getAllRunNumbers(std::string selection, std::string det = "") override;

protected:
  void doCurl(std::string collection, std::string filter = "nada", std::string api = "find");
  void stripOverhead();

private:
  std::string        fApiKey, fURIfindOne, fURIfind, fCurlReadBuffer;

};


#endif
