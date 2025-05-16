#ifndef CDBJSON_h
#define CDBJSON_h

#include "cdbAbs.hh"

// ----------------------------------------------------------------------
// implementation class for a JSON file-based DB
// ----------------------------------------------------------------------

class cdbJSON: public cdbAbs {
public:
  cdbJSON() = default;
  cdbJSON(std::string gt, std::string uri, int verbose);
  ~cdbJSON();

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
  std::vector<std::string> allFiles(std::string dirName);


private:

};


#endif
