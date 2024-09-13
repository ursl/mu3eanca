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
// ../bin/syncMongo --dir ~/data/mu3e/json12/globaltags
// ../bin/syncMongo --dir /Users/ursl/data/mu3e/json12/tags
// ../bin/syncMongo --dir json/payloads [symlink: json -> /Users/ursl/data/mu3e/json12]
// ../bin/syncMongo --uri mongodb://127.0.0.1:27017 --dir runrecords
// ../bin/syncMongo --dir configs
//
// ----------------------------------------------------------------------

bool gDBX(false);

//mongocxx::uri gUri("mongodb://localhost:27017");
//mongocxx::client gClient{gUri};

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
    auto tagCursor     = collection.find(document{} << "tag" << pattern << finalize);
    for (auto doc : tagCursor) {
      cout << "*********** Tags *** " << endl;
      cout << bsoncxx::to_json(doc, bsoncxx::ExtendedJsonMode::k_relaxed) << endl;
      auto delete_one_result = collection.delete_one(doc);
      cout << "*** deleted" << endl;
    }
    
    auto hashCursor    = collection.find(document{} << "hash" << pattern << finalize);
    for (auto doc : hashCursor) {
      cout << "*********** Hash *** " << endl;
      cout << bsoncxx::to_json(doc, bsoncxx::ExtendedJsonMode::k_relaxed) << endl;
      auto delete_one_result = collection.delete_one(doc);
      cout << "*** deleted" << endl;
    }
    
    auto cfgHashCursor = collection.find(document{} << "cfgHash" << pattern << finalize);
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

  // -- command line arguments
  string dirName("fixme"), dirPath("fixme"), pattern("unset");
  bool onlyDelete(false); // ONLY delete, do not write new records
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-d"))    {gDBX = true;}
    if (!strcmp(argv[i], "--dir")) {dirPath = string(argv[++i]);}
    if (!strcmp(argv[i], "-dir"))  {dirPath = string(argv[++i]);}
    if (!strcmp(argv[i], "--del")) {onlyDelete = true;}
    if (!strcmp(argv[i], "-del"))  {onlyDelete = true;}
    if (!strcmp(argv[i], "--pat")) {pattern = string(argv[++i]);}
    if (!strcmp(argv[i], "-p"))    {pattern = string(argv[++i]);}
    if (!strcmp(argv[i], "--uri")) {gUri = bsoncxx::string::view_or_value(argv[++i]); gClient = gUri;}
    if (!strcmp(argv[i], "-u"))    {gUri = bsoncxx::string::view_or_value(argv[++i]); gClient = gUri;}
  }
  
  vector<string> vfiles;
  DIR *folder;
  struct dirent *entry;
  
  folder = opendir(dirPath.c_str());
  if (folder == NULL) {
    cout << "Unable to read directory ->" << dirPath << "<-" << endl;
    if (!onlyDelete) return 0;
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
  cout << "clearCollection(" << dirName << ");" << endl;
  clearCollection(dirName, pattern);
  
  
  if (onlyDelete) return 0;
  
  auto db = gClient["mu3e"];
  auto collection = db[dirName];
  
  string collectionContents, historyString;
  ifstream INS;
  for (auto it: vfiles) {
    INS.open(it);
    
    std::stringstream buffer;
    buffer << INS.rdbuf();
    INS.close();
    
    collectionContents = buffer.str();

    if (dirName == "runrecords") {
      collectionContents = collectionContents.substr(0, collectionContents.size() - 3);
      collectionContents += ",\n";
    
      historyString = "  \"History\": ";
      historyString += "[{\"date\": \"" + shortTimeStamp() + "\", \"comment\": \"Database entry inserted\"}]";
      collectionContents += historyString + "\n";
      collectionContents += "}";
    }
    
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
      cout << "insert: " << it << endl
           << collectionContents
           << endl;
      auto insert_one_result = collection.insert_one(bsoncxx::from_json(collectionContents));
    }
  }
  
  return 0;
}
