#ifndef CDBJSON_h
#define CDBJSON_h

#include "cdb.hh"

// ----------------------------------------------------------------------
// implementation class for a JSON file-based DB
// ----------------------------------------------------------------------

class cdbJSON: public cdb {
public:
  cdbJSON() = default;
  cdbJSON(std::string gt, std::string uri);
  ~cdbJSON();

  void                     init();
  std::string              getPayload(int irun, std::string t) override;
  std::string              getPayload(std::string hash) override;

protected:
	void readTags() override;
	void readGlobalTags() override;
	void readIOVs() override;

  std::vector<std::string> allFiles(std::string dirName);

  
private: 
  
};


#endif
