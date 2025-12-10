#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string.h>
#include <dirent.h>  /// for directory reading
#include <sys/time.h>

#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>

#include "cdbUtil.hh"
#include "../../common/json.h"

using json = nlohmann::ordered_json;

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::basic::sub_array;
using bsoncxx::builder::basic::sub_document;
using bsoncxx::builder::basic::make_document;


using namespace std;

// ----------------------------------------------------------------------
// syncMongo
// ---------
//
//
// [requires]
//    globaltags/*
//    tags/*
//    payloads/*
//    configs/*
//
// Usage:
// ./bin/syncMongo --host 127.0.0.1 --dir ~/data/mu3e/json12/globaltags
// ./bin/syncMongo --host localhost --dir /Users/ursl/data/mu3e/json12/tags
// ./bin/syncMongo --host localhost --dir json/payloads [symlink: json -> /Users/ursl/data/mu3e/json12]
// ./bin/syncMongo --host localhost --dir runrecords
// ./bin/syncMongo --host pc11740 --dir configs
// ./bin/syncMongo --host pc11740 --dir ../../run2025/analysis/payloads --pat tag_pixelqualitylm_mcidealv6.1_iov_218
//
//
// ----------------------------------------------------------------------

bool gDBX(false);

mongocxx::uri gUri;
mongocxx::client gClient;

// ----------------------------------------------------------------------
string shortTimeStamp() {
  char buffer[11];
  time_t t;
  time(&t);
  tm r;
  strftime(buffer, sizeof(buffer), "%X", localtime_r(&t, &r));
  struct timeval tv;
  gettimeofday(&tv, 0);
 
  tm *ltm = localtime(&t);
  int year  = 1900 + ltm->tm_year;
  int month = 1 + ltm->tm_mon;
  int day   = ltm->tm_mday;
  int hour  = ltm->tm_hour;
  int min   = ltm->tm_min;
  int sec   = ltm->tm_sec;
  std::stringstream result;
  result << year << "/"
         << std::setfill('0') << std::setw(2) << month << "/"
         << std::setfill('0') << std::setw(2) << day << " ";
  result << std::setfill('0') << std::setw(2) << hour << ":"
         << std::setfill('0') << std::setw(2) << min << ":"
         << std::setfill('0') << std::setw(2) << sec ;
  return result.str();
}


// ----------------------------------------------------------------------
void clearCollection(string scollection, string pattern) {
  vector<string> idCollections;
  
  auto db = gClient["mu3e"];
  auto collection = db[scollection];
  
  if (pattern != "unset") {
    auto gtCursor     = collection.find(document{} << "gt" << open_document << "$regex" << pattern << "$options" << "i" << close_document << finalize);
    cout << "hello" << endl;
    for (auto doc : gtCursor) {
      cout << "*********** Global Tags *** " << endl;
      cout << bsoncxx::to_json(doc, bsoncxx::ExtendedJsonMode::k_relaxed) << endl;
      auto delete_one_result = collection.delete_one(doc);
      cout << "*** deleted" << endl;
    }

    auto tagCursor     = collection.find(document{} << "tag" << open_document << "$regex" << pattern << "$options" << "i" << close_document << finalize);
    cout << "hello" << endl;
    for (auto doc : tagCursor) {
      cout << "*********** Tags *** " << endl;
      cout << bsoncxx::to_json(doc, bsoncxx::ExtendedJsonMode::k_relaxed) << endl;
      auto delete_one_result = collection.delete_one(doc);
      cout << "*** deleted" << endl;
    }
    
    auto hashCursor    = collection.find(document{} << "hash" << open_document << "$regex" << pattern << "$options" << "i" << close_document << finalize);
    for (auto doc : hashCursor) {
      if (doc["hash"]) {
        cout << doc["hash"].get_utf8().value.to_string() << " ... deleted" << endl;
      }
      auto delete_one_result = collection.delete_one(doc);
    }
    
    auto cfgHashCursor = collection.find(document{} << "cfgHash" << open_document << "$regex" << pattern << "$options" << "i" << close_document << finalize);
    for (auto doc : cfgHashCursor) {
      cout << "*********** cfgHash *** " << endl;
      cout << bsoncxx::to_json(doc, bsoncxx::ExtendedJsonMode::k_relaxed) << endl;
      auto delete_one_result = collection.delete_one(doc);
      cout << "*** deleted" << endl;
    }
  } else {
    auto cursor_all = collection.find({});
    cout << "collection " << collection.name()
         << " contains these documents:" << endl;
    for (auto doc : cursor_all) {
      cout << bsoncxx::to_json(doc, bsoncxx::ExtendedJsonMode::k_relaxed) << endl;
      // string h = doc["cfgHash"].get_string().value.to_string();
      // cout << "** h = " << h << endl;
      auto delete_one_result = collection.delete_one(doc);
      cout << "delete_one_result = " << delete_one_result->deleted_count() << endl;
    }
  }
  
}


// // ----------------------------------------------------------------------
// void updateDoc(string hashName, string hashValue, string scollection, const string& docContents) {
//   vector<string> idCollections;

//   auto db = gClient["mu3e"];
//   auto collection = db[scollection];

//   //  bsoncxx::from_json(docContents)
//   cout << "before update" << endl;
//   auto update_one_result =
//     collection.update_one(make_document(kvp(hashName, hashValue)), bsoncxx::from_json(docContents));
//   cout << "after update" << endl;
//   assert(update_one_result);  // Acknowledged writes return results.
//   assert(update_one_result->modified_count() == 1);
// }


// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {
  mongocxx::instance instance;

  // -- command line arguments
  string dirName("fixme"), dirPath("fixme"), pattern("unset"), uriString("unset"), host("localhost");
  bool all(false);
  bool noDeletion(true); // by default nothing is deleted
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-d"))    {gDBX = true;}
    if (!strcmp(argv[i], "--all")) {all = true;}
    if (!strcmp(argv[i], "--dir")) {dirPath = string(argv[++i]);}
    if (!strcmp(argv[i], "--del")) {noDeletion = false;}
    if (!strcmp(argv[i], "--host")) {host = string(argv[++i]);}
    if (!strcmp(argv[i], "--pat")) {pattern = string(argv[++i]);}
  }

  uriString = "mongodb://" + host + ":27017";
  
  if (uriString == "unset") {
    cout << "uriString is unset" << endl;
    return 0;
  } else {
    cout << "uriString ->" << uriString << "<-" << endl;
    gUri = bsoncxx::string::view_or_value(uriString);
    gClient = gUri;
    cout << "gUri set" << endl;
  }

  // -- remove trailing slash (if present)
  if (!dirPath.empty() && dirPath.back() == '/') {
    dirPath.pop_back();
  }

  if (all) {
    string string1 = string(argv[0]) + " --dir " + dirPath + "/" + "globaltags" + " --host " + host;
    string string2 = string(argv[0]) + " --dir " + dirPath + "/" + "tags" + " --host " + host;
    string string3 = string(argv[0]) + " --dir " + dirPath + "/" + "payloads" + " --host " + host;
    cout << string1 << endl;
    system(string1.c_str()); 
    cout << string2 << endl;
    system(string2.c_str()); 
    cout << string3 << endl;
    system(string3.c_str()); 
    return 0;
  }

  
  vector<string> vfiles;
  DIR *folder;
  struct dirent *entry;
  
  folder = opendir(dirPath.c_str());
  if (folder == NULL) {
    cout << "Unable to read directory ->" << dirPath << "<-" << endl;
    return 0;
  } else {
      while ((entry=readdir(folder))) {
      if (8 == entry->d_type) {
        vfiles.push_back(dirPath + "/" + entry->d_name);
      }
    }
    closedir(folder);
    sort(vfiles.begin(), vfiles.end());
  }
  
  dirName = dirPath.substr(dirPath.rfind("/")+1);
  cout << "dirPath ->" << dirPath << "<-" << endl;
  cout << "dirName ->" << dirName << "<-" << endl;
  if (noDeletion) {
    // -- do nothing
  } else{
    if (dirName != "runrecords" && dirName != "configs") {
      cout << "clearCollection(" << dirName << ");" << endl;
      clearCollection(dirName, pattern);
    } else {
      cout << "NO! Do NOT clearCollection(" << dirName << ");" << endl;
    }
  }
    
  auto db = gClient["mu3e"];
  auto collection = db[dirName];
  
  string collectionContents, historyString;
  ifstream INS;
  for (auto it: vfiles) {
    if (pattern != "unset") {
      if (string::npos == it.find(pattern)) {
        cout << "pattern ->" << pattern << "<- not matched to ->" << it << "<- ... skipping" << endl;
        continue;
      }
    }

    INS.open(it);
    
    std::stringstream buffer;
    buffer << INS.rdbuf();
    INS.close();
    

    
    collectionContents = buffer.str();
    
    // -- parse buffer into nlohmann json
    json j = json::parse(collectionContents);
    
    // -- drop the "_id" field if it exists
    j.erase("_id");

    // -- add History to json object j
    j["History"] = json::array({json::object({{"date", shortTimeStamp()}, {"comment", "Database entry inserted"}})});
    
    if (dirName == "runrecords") {
      // -- check whether EOR is present using nlohmann json
      if (!j.contains("EOR")) {
        cout << "EOR not found in ->" << it << "<- ... skipping" << endl;
        continue;
      }
      // -- check if EOR appears multiple times (as array with multiple elements)
      if (j["EOR"].is_array() && j["EOR"].size() > 1) {
        cout << "multiple EORs found in ->" << it << "<- ... skipping" << endl;
        continue;
      }
    }
    // -- convert json object j back to string
    collectionContents = j.dump();
    
    if (gDBX) {
      cout << "insert: " << it << endl
           << collectionContents
           << endl;
    } else {
    
      if (pattern != "unset") {
        if (string::npos == it.find(pattern)) {
          cout << "pattern ->" << pattern << "<- not matched to ->" << it << "<- ... skipping" << endl;
          continue;
        }
      }
      
      
      if (dirName == "configs") {
        size_t offset = string("cfgString").size() + 5;
        replaceAll(collectionContents, "\n", "\\n", collectionContents.find("cfgString") + offset);
      }
      cout << "start inserting " << it << endl;
      auto insert_one_result = collection.insert_one(bsoncxx::from_json(collectionContents));
      bsoncxx::oid oid = insert_one_result->inserted_id().get_oid().value;
      std::string oidString = oid.to_string();

      string sfilename = "./inserted/" + it.substr(it.rfind("/")+1);
      ofstream ONS(sfilename);
      ONS << collectionContents << endl;
      ONS.close();
    }
  }
  
  return 0;
}
