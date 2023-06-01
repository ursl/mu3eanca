#ifndef CDBASCII_h
#define CDBASCII_h

#include "cdb.hh"

// ----------------------------------------------------------------------
// implementation class for an ASCII DB
// ----------------------------------------------------------------------

class cdbAscii: public cdb {
public:
  cdbAscii() = default;
  cdbAscii(std::string name, std::string uri);
  ~cdbAscii();

  void                     init();
  std::string              getPayload(int irun, std::string t) override;
  std::string              getPayload(std::string hash) override;

  std::vector<std::string> split(const std::string &s, char delim);
  void                     cleanupString(std::string &);
  // -- helper functions for above
  void                     replaceAll(std::string &s,
                                      const std::string &from,
                                      const std::string &to);
  void                     split(const std::string &s, char delim,
                                 std::vector<std::string> &elems);

protected:
	void readTags() override;
	void readGlobalTags() override;
	void readIOVs() override;

private: 
  
};


#endif
