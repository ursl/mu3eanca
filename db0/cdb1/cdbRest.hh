#ifndef CDBREST_h
#define CDBREST_h

#include "cdbAbs.hh"

#include <curl/curl.h> 

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

  
  std::vector<std::string>                 readGlobalTags(std::string gt) override;
  std::vector<std::string>                 readTags(std::string gt) override;
	std::map<std::string, std::vector<int> > readIOVs(std::vector<std::string> tags) override;
  
protected:
  void doCurl(std::string collection, std::string filter = "nada", std::string api = "find"); 
  // -- remove from fCurlReadBuffer "{"_id":"649042ec330d198bb5be6e19", ... ]}
  void stripOverhead();
  
private: 
  std::string        fApiKey, fURIfindOne, fURIfind, fCurlReadBuffer;

};


#endif
