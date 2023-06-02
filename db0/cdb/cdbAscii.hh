#ifndef CDBASCII_h
#define CDBASCII_h

#include "cdb.hh"

// ----------------------------------------------------------------------
// implementation class for an ASCII DB
// ----------------------------------------------------------------------

class cdbAscii: public cdb {
public:
  cdbAscii() = default;
  cdbAscii(std::string gt, std::string uri);
  ~cdbAscii();

  void                     init();
  std::string              getPayload(int irun, std::string t) override;
  std::string              getPayload(std::string hash) override;

protected:
	void readTags() override;
	void readGlobalTags() override;
	void readIOVs() override;

private: 
  
};


#endif
