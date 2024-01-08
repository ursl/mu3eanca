#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
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
// cd cdb1/json 
// [requires]
//    globaltags/*
//    tags/*
//    payloads/*
//
// Usage:
// ../bin/syncCloud --dir globaltags
// ../bin/syncCloud --dir tags
// ../bin/syncCloud --dir payloads
// ../bin/syncCloud --dir runrecords
// 
// ----------------------------------------------------------------------

bool gDBX(false);

mongocxx::instance gInstance{}; // This should be done only once.
mongocxx::uri gUri("mongodb://pc11740:27017");
mongocxx::client gClient{gUri};

// ----------------------------------------------------------------------
void clearCollection(string scollection, string field) {
  vector<string> idCollections;

  auto db = gClient["mu3e"];
  auto collection = db[scollection];
  
  auto cursor_all = collection.find({});
  cout << "collection " << collection.name()
       << " contains these documents:" << endl;
  // for (auto doc : cursor_all) {
  //   cout << bsoncxx::to_json(doc, bsoncxx::ExtendedJsonMode::k_relaxed) << endl;
  // }
  for (auto doc : cursor_all) {
    cout << bsoncxx::to_json(doc, bsoncxx::ExtendedJsonMode::k_relaxed) << endl;
    auto delete_one_result = collection.delete_one(doc);
    cout << "delete_one_result = " << delete_one_result->deleted_count() << endl;
  }

}


// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {

  // -- command line arguments
  string dirName("fixme");
  bool onlyDelete(false); // ONLY delete, do not write new records
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i], "-d"))  {gDBX = true;}
    if (!strcmp(argv[i], "--dir"))  {dirName = string(argv[++i]);}
    if (!strcmp(argv[i], "-dir"))  {dirName = string(argv[++i]);}
    if (!strcmp(argv[i], "--del"))  {onlyDelete = true;}
    if (!strcmp(argv[i], "-del"))  {onlyDelete = true;}
  }

  map<string, string> tagDel = {{"globaltags", "gt"},
                                {"tags", "tags"},
                                {"payloads", "hash"}
  };

  vector<string> vfiles;
  DIR *folder;
  struct dirent *entry;
  
  folder = opendir(dirName.c_str());
  if (folder == NULL) {
    cout << "Unable to read directory ->" << dirName << "<-" << endl;
    if (!onlyDelete) return 0;
  } else {
    
    while ((entry=readdir(folder))) {
      if (8 == entry->d_type) {
        vfiles.push_back(dirName + "/" + entry->d_name);
      }
    }
    closedir(folder);
    sort(vfiles.begin(), vfiles.end());    
  }

  cout << "clearCollection(" << dirName << ", " << tagDel[dirName] << ");" << endl;
  clearCollection(dirName, tagDel[dirName]);

  if (onlyDelete) return 0;

  auto db = gClient["mu3e"];
  auto collection = db[dirName];
  
  string collectionContents;
  ifstream INS;
  for (auto it: vfiles) {
    INS.open(it);
    getline(INS, collectionContents);
    INS.close();


    auto insert_one_result = collection.insert_one(bsoncxx::from_json(collectionContents));
  }
  
	return 0;
}
