#ifndef CDBREST_h
#define CDBREST_h

#include "cdb.hh"

// ----------------------------------------------------------------------
// implementation class for a RESTful access of payloads
// ----------------------------------------------------------------------

class cdbRest: public cdb {
public:
  cdbRest() = default;
  cdbRest(std::string gt, std::string uri);
  ~cdbRest();

  void                 init();
  payload              getPayload(int irun, std::string t) override;
  payload              getPayload(std::string hash) override;

protected:
	void readTags() override;
	void readGlobalTags() override;
	void readIOVs() override;

  
private: 
  std::string fApiKey;
  
};


#endif
