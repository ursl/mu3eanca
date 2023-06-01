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
  std::string              getPayload(int irun, std::string t) override;
  std::string              getPayload(std::string hash) override;

protected:
	void readTags() override;
  void readGlobalTags() override;
  void readIOVs() override;


private: 
  mongocxx::client   fConn;
  mongocxx::database fDB;
};


#endif
