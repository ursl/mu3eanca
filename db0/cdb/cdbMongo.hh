#ifndef CDBMONGO_h
#define CDBMONGO_h

#include "cdb.hh"

#include <mongocxx/instance.hpp>
#include <mongocxx/client.hpp>

// ----------------------------------------------------------------------
// implementation class for a Mongo DB
// ----------------------------------------------------------------------

class cdbMongo: public cdb {
public:
  cdbMongo() = default;
  cdbMongo(std::string name, std::string uri);
  ~cdbMongo();

  void                     init();
  std::vector<std::string> getGlobalTags() override;
  std::vector<std::string> getTags(std::string gt);

private: 
  mongocxx::client   fConn;
  mongocxx::database fDB;
};


#endif
