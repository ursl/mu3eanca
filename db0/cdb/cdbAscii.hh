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
  std::vector<std::string> getGlobalTags() override;
  std::vector<std::string> split(const std::string &s, char delim);
  // -- helper function for above
  void                     split(const std::string &s, char delim,
                                 std::vector<std::string> &elems);

private: 

};


#endif
