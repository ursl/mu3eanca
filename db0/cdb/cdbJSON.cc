#include "cdbJSON.hh"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <dirent.h>  /// for directory reading

#include "util/util.hh"

#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>

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
cdbJSON::cdbJSON(string gt, string uri) : cdb(gt, uri) {
  fVerbose = 10;
  init();
}


// ----------------------------------------------------------------------
cdbJSON::~cdbJSON() { }


// ----------------------------------------------------------------------
void cdbJSON::init() {
  fName = "JSON"; 
  readGlobalTags();
  readTags();
  //  cdb::init();
}


// ----------------------------------------------------------------------
void cdbJSON::readGlobalTags() {
  fGlobalTags.clear();
  cout << "cdbJSON::readGlobalTags()" << endl;
  if (!fValidGlobalTags) {
    // -- read global tags from fURI
    string gtdir = fURI + "/globaltags";
    cout << "gtdir = " << gtdir << endl;
    vector<string> gtFiles = allFiles(gtdir);

    ifstream INS;
    for (auto it: gtFiles) {
      // -- remove everything up to and including the last /
      string::size_type pos = it.rfind("/");
      string file = it.substr(pos+1);
      fGlobalTags.push_back(file);
      if (fVerbose > 0) cout << "JSON read GT " << file << endl;
    }
  }
  return;
}


// ----------------------------------------------------------------------
void cdbJSON::readTags() {
  fTags.clear();
  // -- read global tags from fURI
  string gtdir = fURI + "/globaltags/";

  ifstream INS;
  string gtfile = gtdir + fGT;
  INS.open(gtfile);
  if (INS.fail()) {
    cout << "Error failed to open ->" << gtfile << "<-" << endl;
    return;
  }

  cout << "Read " << gtfile << endl;
  std::stringstream buffer;
  buffer << INS.rdbuf();
  INS.close();
  
  bsoncxx::document::value doc = bsoncxx::from_json(buffer.str());
  bsoncxx::array::view subarr{doc["tags"].get_array()};
  for (bsoncxx::array::element ele : subarr) {
    string tname = string(ele.get_string().value).c_str();
    fTags.push_back(tname); 
  }
  
  if (fVerbose > 0) {
    cout << "cdbJSON::readTags> for GT = " << fGT << endl;
    print(fTags);
  }
  return;
}


// ----------------------------------------------------------------------
void cdbJSON::readIOVs() {
  // -- read iovs from fURI
  ifstream INS;
  string dir = fURI + "/iovs/";
  
  for (auto it: fTags) {
    string file = dir + it;
    INS.open(file);
    if (INS.fail()) {
      cout << "Error failed to open ->" << file << "<-" << endl;
      return;
    }

    std::stringstream buffer;
    buffer << INS.rdbuf();
    INS.close();
    
    bsoncxx::document::value doc = bsoncxx::from_json(buffer.str());
    bsoncxx::array::view subarr{doc["tags"].get_array()};
    for (bsoncxx::array::element ele : subarr) {
      string tname = string(ele.get_string().value).c_str();
      fTags.push_back(tname); 
    }

  }
  
  return;
}


// ----------------------------------------------------------------------
payload cdbJSON::getPayload(int irun, string tag) {
  string hash = getHash(irun, tag); 
  return getPayload(hash);
}


// ----------------------------------------------------------------------
payload cdbJSON::getPayload(string hash) {
  // -- initialize with default
  std::stringstream sspl;
  sspl << "(cdbJSON>  hash = " << hash 
       << " not found)";
  payload pl;
  pl.fComment = sspl.str();
  
  // -- read payload for hash 
  ifstream INS;
  string filename = fURI + "/payloads/" + hash;
  INS.open(filename);
  if (INS.fail()) {
    cout << "Error failed to open ->" << filename << "<-" << endl;
    return pl;
  }

  std::stringstream buffer;
  buffer << INS.rdbuf();
  INS.close();

  bsoncxx::document::value doc = bsoncxx::from_json(buffer.str());
  bsoncxx::array::view subarr{doc["BLOB"].get_array()};
  for (bsoncxx::array::element ele : subarr) {
    string tname = string(ele.get_string().value).c_str();
    pl.fBLOB = tname; 
  }
  

  return pl;
}


// ----------------------------------------------------------------------
vector<string> cdbJSON::allFiles(string dirName) {
  vector<string> vfiles;
  DIR *folder;
  struct dirent *entry;
  
  folder = opendir(dirName.c_str());
  if (folder == NULL) {
    puts("Unable to read directory");
    return vfiles;
  } 
  
  while ((entry=readdir(folder))) {
    if (8 == entry->d_type) {
      vfiles.push_back(dirName + "/" + entry->d_name);
    }
  }
  closedir(folder);
  
  sort(vfiles.begin(), vfiles.end());    
  return vfiles;
}
