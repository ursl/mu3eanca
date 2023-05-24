#ifndef CDB_h
#define CDB_h

#include <string>
#include <vector>

// ----------------------------------------------------------------------
// abstract base class for DB access
// ----------------------------------------------------------------------

class cdb {
public:
  cdb() = default;
  cdb(std::string name, std::string uri);
  ~cdb();

  std::string                      getName() {return fName;}
  virtual std::vector<std::string> getGlobalTags() {return std::vector<std::string>();}
  
protected: 
  std::string fName{"default"};
  std::string fURI{"unset"};

  bool fValidGlobalTags{false}; 
  std::vector<std::string> fGlobalTags;
};


#endif
