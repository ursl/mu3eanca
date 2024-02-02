#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string.h>
#include <dirent.h>  /// for directory reading

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
// ../bin/syncMongo --dir runrecords
// ../bin/syncMongo --dir configs
// 
// ----------------------------------------------------------------------

bool gDBX(false);

//mongocxx::instance gInstance{}; // This should be done only once.
mongocxx::uri gUri("mongodb://pc11740:27017");
mongocxx::client gClient{gUri};

// ----------------------------------------------------------------------
void clearCollection(string scollection) {
  vector<string> idCollections;

  auto db = gClient["mu3e"];
  auto collection = db[scollection];
  
  auto cursor_all = collection.find({});
  cout << "collection " << collection.name()
       << " contains these documents:" << endl;
  for (auto doc : cursor_all) {
    cout << bsoncxx::to_json(doc, bsoncxx::ExtendedJsonMode::k_relaxed) << endl;
    auto delete_one_result = collection.delete_one(doc);
    cout << "delete_one_result = " << delete_one_result->deleted_count() << endl;
  }

}


// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {

  // -- command line arguments
  string dirName("fixme"), dirPath("fixme");
  bool onlyDelete(false); // ONLY delete, do not write new records
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i], "-d"))  {gDBX = true;}
    if (!strcmp(argv[i], "--dir"))  {dirPath = string(argv[++i]);}
    if (!strcmp(argv[i], "-dir"))  {dirPath = string(argv[++i]);}
    if (!strcmp(argv[i], "--del"))  {onlyDelete = true;}
    if (!strcmp(argv[i], "-del"))  {onlyDelete = true;}
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
  clearCollection(dirName);

  if (onlyDelete) return 0;

  auto db = gClient["mu3e"];
  auto collection = db[dirName];
  
  string collectionContents;
  ifstream INS;
  for (auto it: vfiles) {
    INS.open(it);

    std::stringstream buffer;
    buffer << INS.rdbuf();
    INS.close();

    collectionContents = buffer.str();
    
    if (gDBX) {
      cout << "insert: " << it << endl
           << collectionContents
           << endl;       
    } else {
      if (dirName == "configs") {
        size_t offset = string("cfgString").size() + 5; 
        replaceAll(collectionContents, "\n", "\\n", collectionContents.find("cfgString") + offset);
        //        replaceAll(collectionContents, "\"", "\\"", collectionContents.find("cfgString") + offset);
      }
      cout << "insert: " << it << endl
           << collectionContents
           << endl;
      auto insert_one_result = collection.insert_one(bsoncxx::from_json(collectionContents));
    }
  }
  
	return 0;
}
