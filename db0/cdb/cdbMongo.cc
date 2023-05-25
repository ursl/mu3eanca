#include "cdbMongo.hh"

#include <fstream>
#include <iostream>
#include <sstream>

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


// ----------------------------------------------------------------------
// -- populate mongo DB:
// ----------------------------------------------------------------------

/*
  // -- global tags collection
  mongosh

  use mu3e
  db.globaltags.drop()
  db.createCollection("globaltags")
  db.globaltags.insert({gt: "dt23intrun", tags: ["pixelir", "fibresstart", "tilesNada"]})
  db.globaltags.insert({gt: "dt23prompt", tags: ["pixelv0", "fibresv11", "tilesA"]})

  db.globaltags.insert({gt: "mcideal", tags: ["pixelmcideal", "fibresmcideal", "tilesmcideal"]})
  db.globaltags.insert({gt: "mc23intrun", tags: ["pixelmc23intrun", "fibresmc23intrun", "tilesmc23ideal"]})

  db.globaltags.find()


  // -- iovs collection
  mongosh

  use mu3e
  db.iovs.drop()
  db.createCollection("iovs")
  db.iovs.insert({tag: "pixelir", iovs: [1,10,20,30,100,200]})
  db.iovs.insert({tag: "fibresstart", iovs: [1,2,3,4,15,45,90,150]})
  db.iovs.insert({tag: "tilesNada", iovs: [1]})
  db.iovs.insert({tag: "pixelv0", iovs: [202,210,900]})
  db.iovs.insert({tag: "fibresv11", iovs: [202,400,800]})
  db.iovs.insert({tag: "tilesA", iovs: [202,300,700]})
  db.iovs.insert({tag: "pixelmcideal", iovs: [1]})
  db.iovs.insert({tag: "fibresmcideal", iovs: [1]})
  db.iovs.insert({tag: "tilesmcideal", iovs: [1]})
  db.iovs.insert({tag: "pixelmc23intrun", iovs: [200]})
  db.iovs.insert({tag: "fibresmc23intrun", iovs: [150]})
  db.iovs.insert({tag: "tilesmc23ideal", iovs: [1]})
  db.iovs.find()

  
*/



mongocxx::instance instance{};

// ----------------------------------------------------------------------
cdbMongo::cdbMongo(string name, string uri) : cdb(name, uri) {
  init();
}


// ----------------------------------------------------------------------
cdbMongo::~cdbMongo() { }


// ----------------------------------------------------------------------
void cdbMongo::init() {
  mongocxx::uri uri(fURI);
  fConn = mongocxx::client(uri);
  fDB = fConn["mu3e"];

  // -- list all collections
  if (0) {
    auto cursor1 = fDB.list_collections();
    for (const bsoncxx::document::view& doc :cursor1) {
      bsoncxx::document::element ele = doc["name"];
      string name = ele.get_utf8().value.to_string();
      cout << " " << name << endl;
    }
  }
}

// ----------------------------------------------------------------------
vector<string> cdbMongo::getGlobalTags() {

  if (!fValidGlobalTags) {
    mongocxx::cursor cursor = fDB["globaltags"].find({});
    for (auto doc : cursor) {
      assert(doc["_id"].type() == bsoncxx::type::k_oid);
      string tname = string(doc["gt"].get_string().value).c_str();
      fGlobalTags.push_back(tname); 
    }
    fValidGlobalTags = true;
    return fGlobalTags;
  } else {
    return std::vector<std::string>();
  }

}

// ----------------------------------------------------------------------
vector<string> cdbMongo::getTags(string gt) {
  std::vector<std::string> tags; 

  auto cursor_filtered =  fDB["globaltags"].find(make_document(kvp("gt", gt)));
  for (auto doc : cursor_filtered) {
    // -- print it 
    // cout << bsoncxx::to_json(doc, bsoncxx::ExtendedJsonMode::k_relaxed) << endl;
    assert(doc["_id"].type() == bsoncxx::type::k_oid);
    bsoncxx::array::view subarr{doc["tags"].get_array()};
    for (bsoncxx::array::element ele : subarr) {
      string tname = string(ele.get_string().value).c_str();
      tags.push_back(tname); 
    }
  }

  return tags;
}


// ----------------------------------------------------------------------
vector<int> cdbMongo::getIovs(std::string tag) {
  std::vector<int> iovs; 

  auto cursor_filtered =  fDB["iovs"].find(make_document(kvp("tag", tag)));
  for (auto doc : cursor_filtered) {
    // -- print it 
    // cout << bsoncxx::to_json(doc, bsoncxx::ExtendedJsonMode::k_relaxed) << endl;
    assert(doc["_id"].type() == bsoncxx::type::k_oid);
    bsoncxx::array::view subarr{doc["iovs"].get_array()};
    for (bsoncxx::array::element ele : subarr) {
      int iov = ele.get_int32().value;
      iovs.push_back(iov); 
    }
  }

  return iovs;
}
