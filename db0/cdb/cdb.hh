#ifndef CDB_h
#define CDB_h

#include <string>
#include <vector>
#include <map>

// ----------------------------------------------------------------------
// abstract base class for DB access and tag/IOV management
// ----------------------------------------------------------------------

class cdb {
public:
  cdb() = default;
  cdb(std::string name, std::string uri);
  ~cdb();

  std::string                      getName() {return fName;}
  virtual std::vector<std::string> getGlobalTags() {return std::vector<std::string>();}
  virtual std::vector<std::string> getTags(std::string gt) {return std::vector<std::string>();}
  virtual std::vector<int>         getIovs(std::string t) {return std::vector<int>();}
  virtual std::string              getPayload(int irun, std::string t) {return std::string();}
  
protected: 
  std::string fName{"default"};
  std::string fURI{"unset"};

  bool fValidGlobalTags{false}; 
  std::vector<std::string> fGlobalTags;
  std::map<std::string, std::vector<int>> fTagIOVs;
};


#endif
