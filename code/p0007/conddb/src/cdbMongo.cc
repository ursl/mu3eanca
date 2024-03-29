#include "cdbMongo.hh"

#include <fstream>
#include <iostream>
#include <sstream>
#include <cassert>

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
vector<string> cdbMongo::readGlobalTags(string gt) {
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
  auto cursor =  fDB["iovs"].find({});
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
payload cdbMongo::getPayload(string hash) {
  
  // -- initialize with default
  std::stringstream sspl;
  sspl << "(cdbMongo> hash = " << hash 
       << " not found)";
  payload pl;
  pl.fComment = sspl.str();

  auto cursor_filtered =  fDB["payloads"].find(make_document(kvp("hash", hash)));
  for (auto doc : cursor_filtered) {
    // -- print it 
    // cout << bsoncxx::to_json(doc, bsoncxx::ExtendedJsonMode::k_relaxed) << endl;
    assert(doc["_id"].type() == bsoncxx::type::k_oid);
    pl.fComment = string(doc["comment"].get_string().value).c_str();
    pl.fHash    = string(doc["hash"].get_string().value).c_str();
    pl.fBLOB    = string(doc["BLOB"].get_string().value).c_str();
  }

  return pl;
}
