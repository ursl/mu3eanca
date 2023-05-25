#ifndef CDBASCII_h
#define CDBASCII_h

#include "cdb.hh"

#include <map>

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
  std::vector<std::string> getTags(std::string gt);
  std::vector<int>         getIovs(std::string tag);

  std::vector<std::string> split(const std::string &s, char delim);
  void                     cleanupString(std::string &);
  // -- helper functions for above
  void                     replaceAll(std::string &s,
                                      const std::string &from, const std::string &to);
  void                     split(const std::string &s, char delim,
                                 std::vector<std::string> &elems);

private: 

  std::map<std::string, std::vector<std::string>> fTagMap;
  std::map<std::string, std::vector<int>> fIovMap;
  
};


#endif
