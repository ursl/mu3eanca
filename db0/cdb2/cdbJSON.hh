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
  runRecord            getRunRecord(int irun) override;
  payload              getPayload(std::string hash) override;

  std::vector<std::string>                 readGlobalTags() override;
  std::vector<std::string>                 readTags(std::string gt) override;
	std::map<std::string, std::vector<int> > readIOVs(std::vector<std::string> tags) override;

protected:
  std::vector<std::string> allFiles(std::string dirName);

  
private: 
  
};


#endif
