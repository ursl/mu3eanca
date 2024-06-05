#include "cdbMongo.hh"

#include <fstream>
#include <iostream>
#include <sstream>
#include <cassert>

#include "base64.hh"
#include "cdbUtil.hh"

#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>

using namespace std;

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::sub_array;
using bsoncxx::builder::basic::sub_document;
using bsoncxx::builder::basic::make_document;

// -- required for mongo
mongocxx::instance instance{};


// ----------------------------------------------------------------------
cdbMongo::cdbMongo(string name, string uri, int verbose) : cdbAbs(name, uri, verbose) {
  init();
}


// ----------------------------------------------------------------------
cdbMongo::~cdbMongo() { }


// ----------------------------------------------------------------------
void cdbMongo::init() {
  fName = "mongodb";
  mongocxx::uri uri(fURI);
  fConn = mongocxx::client(uri);
  fDB = fConn["mu3e"];
  
  // -- list all collections
  if (fVerbose > 4) {
    cout << "list all collections " << endl;
    auto cursor1 = fDB.list_collections();
    for (const bsoncxx::document::view& doc :cursor1) {
      bsoncxx::document::element ele = doc["name"];
      string name = ele.get_utf8().value.to_string();
      cout << " " << name << endl;
    }
  }
  
  cdbAbs::init();
}

// ----------------------------------------------------------------------
vector<string> cdbMongo::readGlobalTags() {
  vector<string> v;
  mongocxx::cursor cursor = fDB["globaltags"].find({});
  for (auto doc : cursor) {
    if (fVerbose > 1) {
      cout << "cdbMongo::readGlobalTags()> "
           << bsoncxx::to_json(doc, bsoncxx::ExtendedJsonMode::k_relaxed)
           << endl;
    }
    assert(doc["_id"].type() == bsoncxx::type::k_oid);
    string tname = string(doc["gt"].get_string().value).c_str();
    v.push_back(tname);
  }
  if (fVerbose > 0) {
    cout << "cdbMongo::readGlobalTags()> readGlobalTags = ";
    print(v);
  }
  return v;
}


// ----------------------------------------------------------------------
vector<string> cdbMongo::readTags(string gt) {
  vector<string> v;
  auto cursor_filtered =  fDB["globaltags"].find(make_document(kvp("gt", gt)));
  for (auto doc : cursor_filtered) {
    // -- print it
    // cout << bsoncxx::to_json(doc, bsoncxx::ExtendedJsonMode::k_relaxed) << endl;
    assert(doc["_id"].type() == bsoncxx::type::k_oid);
    bsoncxx::array::view subarr{doc["tags"].get_array()};
    for (bsoncxx::array::element ele : subarr) {
      string tname = string(ele.get_string().value).c_str();
      v.push_back(tname);
    }
  }
  if (fVerbose > 0) {
    cout << "cdbMongo::readTags()> for GT = " << gt << ": ";
    print(v);
  }
  return v;
}


// ----------------------------------------------------------------------
map<string, vector<int>> cdbMongo::readIOVs(vector<string> tags) {
  map<string, vector<int>> m;
  auto cursor =  fDB["tags"].find({});
  for (auto doc : cursor) {
    // -- print it
    //    cout << bsoncxx::to_json(doc, bsoncxx::ExtendedJsonMode::k_relaxed) << endl;
    assert(doc["_id"].type() == bsoncxx::type::k_oid);
    string tname = string(doc["tag"].get_string().value).c_str();
    // -- look only at tags in fGT
    if (tags.end() == find(tags.begin(), tags.end(), tname)) continue;
    bsoncxx::array::view subarr{doc["iovs"].get_array()};
    vector<int> viov;
    for (bsoncxx::array::element ele : subarr) {
      int iov = ele.get_int32().value;
      viov.push_back(iov);
    }
    m.insert(make_pair(tname, viov));
  }
  
  if (fVerbose > 1) {
    cout << "cdbMongo::readIOVs>" << endl;
    print(m);
  }
  
  return m;
}


// ----------------------------------------------------------------------
runRecord cdbMongo::getRunRecord(int irun) {
  // -- initialize with default
  std::stringstream sspl;
  sspl << "(cdbMongo>  runRecord for run = " << to_string(irun)
       << " not found)";
  runRecord rr;
  rr.fEORComments = sspl.str();
  
  auto cursor_filtered =  fDB["runrecords"].find(make_document(kvp("BOR.Run number", irun)));
  for (auto doc : cursor_filtered) {
    assert(doc["_id"].type() == bsoncxx::type::k_oid);
    
    auto bor = doc["BOR"];
    rr.fBORRunNumber     = bor["Run number"].get_int32().value;
    rr.fBORStartTime     = bor["Start time"].get_string().value.to_string();
    rr.fBORSubsystems    = bor["Subsystems"].get_int32().value;
    if (bor["Beam"].type() == bsoncxx::type::k_double) {
      rr.fBORBeam          = bor["Beam"].get_double().value;
    } else if (bor["Beam"].type() == bsoncxx::type::k_int32) {
      rr.fBORBeam          = static_cast<double>(bor["Beam"].get_int32().value);
    }
    rr.fBORShiftCrew     = bor["Shift crew"].get_string().value.to_string();
    
    auto eor = doc["EOR"];
    rr.fEORStopTime      = eor["Stop time"].get_string().value.to_string();
    if (eor["Events"].type() == bsoncxx::type::k_double) {
      rr.fEOREvents        = eor["Events"].get_int64().value;
    } else if (eor["Events"].type() == bsoncxx::type::k_int64) {
      rr.fEOREvents        = eor["Events"].get_int64().value;
    }
    rr.fEORFileSize      = eor["File size"].get_double().value;
    rr.fEORDataSize      = eor["Uncompressed data size"].get_double().value;
    rr.fEORComments      = eor["Comments"].get_string().value.to_string();
  }
  
  return rr;
}


// ----------------------------------------------------------------------
payload cdbMongo::getPayload(string hash) {

  // -- initialize with default
  std::stringstream sspl;
  sspl << "(cdbMongo> hash = " << hash
       << " not found)";
  payload pl;
  pl.fComment = sspl.str();
  
  auto cursor_filtered =  fDB["payloads"].find(make_document(kvp("hash", hash)));
  for (auto doc : cursor_filtered) {
    assert(doc["_id"].type() == bsoncxx::type::k_oid);
    pl.fComment = doc["comment"].get_string().value.to_string();
    pl.fHash    = doc["hash"].get_string().value.to_string();
    pl.fBLOB    = base64_decode(doc["BLOB"].get_string().value.to_string());
  }
  
  return pl;
}


// ----------------------------------------------------------------------
cfgPayload cdbMongo::getConfig(string hash) {

  cfgPayload cfg;
  
  // -- initialize with default
  std::stringstream sspl;
  sspl << "(cdbMongo>  hash = " << hash
       << " not found)";
  cfg.fCfgString = sspl.str();
  
  auto cursor_filtered =  fDB["configs"].find(make_document(kvp("cfgHash", hash)));
  for (auto doc : cursor_filtered) {
    assert(doc["_id"].type() == bsoncxx::type::k_oid);
    cfg.fDate      = doc["cfgDate"].get_string().value.to_string();
    cfg.fHash      = doc["cfgHash"].get_string().value.to_string();
    cfg.fCfgString = doc["cfgString"].get_string().value.to_string();
  }
  return cfg;
}
