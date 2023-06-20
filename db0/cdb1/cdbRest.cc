#include "cdbRest.hh"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <dirent.h>    /// for directory reading
#include <curl/curl.h> /// for libcurl

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
static size_t cdbRestWriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  ((std::string*)userp)->append((char*)contents, size * nmemb);
  return size * nmemb;
}



// ----------------------------------------------------------------------
cdbRest::cdbRest(string gt, string uri) : cdbAbs(gt, uri) {
  fVerbose = 10;
  init();
}


// ----------------------------------------------------------------------
cdbRest::~cdbRest() { }


// ----------------------------------------------------------------------
void cdbRest::init() {
  fName = "REST"; 
  
  ifstream INS("api-key.private");
  getline(INS, fApiKey);
  INS.close();
  fApiKey = "api-key: " + fApiKey;

  CURL *curl;
  CURLcode res;
  std::string readBuffer;
  
  curl = curl_easy_init();

  struct curl_slist *headers=NULL;
  struct curl_slist *temp=NULL;
    
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, fURI.c_str());

    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "Access-Control-Request-Headers: *");
    headers = curl_slist_append(headers, fApiKey.c_str());

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{\"collection\":\"payloads\", \"database\":\"mu3e\", \"dataSource\":\"cdb0\", \"filter\": {\"hash\": \"tag_pixelalignment_dt23intrun_iov_200\"}}");
    
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cdbRestWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    std::cout << readBuffer << std::endl;
  }
  return;

  cdbAbs::init();
}


// ----------------------------------------------------------------------
void cdbRest::readGlobalTags() {
  fGlobalTags.clear();
  cout << "cdbRest::readGlobalTags()" << endl;
  if (!fValidGlobalTags) {
    // -- read global tags from fURI
    string gtdir = fURI + "/globaltags";
    cout << "gtdir = " << gtdir << endl;
    return;
  }
}


// ----------------------------------------------------------------------
void cdbRest::readTags() {
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
    cout << "cdbRest::readTags> for GT = " << fGT << endl;
    print(fTags);
  }
  return;
}


// ----------------------------------------------------------------------
void cdbRest::readIOVs() {
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

    cout << " DBX it = " << it << endl;

    
    std::stringstream buffer;
    buffer << INS.rdbuf();
    INS.close();
    
    bsoncxx::document::value doc = bsoncxx::from_json(buffer.str());
    bsoncxx::array::view subarr{doc["iovs"].get_array()};
    vector<int> viov; 
    for (bsoncxx::array::element ele : subarr) {
      int iov = ele.get_int32().value;
      cout << "   DBX iov = " << iov << endl;
      viov.push_back(iov);
    }
    fIOVs.insert(make_pair(it, viov)); 
  }
  
  return;
}


// ----------------------------------------------------------------------
payload cdbRest::getPayload(int irun, string tag) {
  string hash = getHash(irun, tag); 
  return getPayload(hash);
}


// ----------------------------------------------------------------------
payload cdbRest::getPayload(string hash) {
  // -- initialize with default
  std::stringstream sspl;
  sspl << "(cdbRest>  hash = " << hash 
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
  
  cout << "cdbRest::getPayload() Read " << filename << " hash ->" << hash << "<-" << endl;
  bsoncxx::document::value doc = bsoncxx::from_json(buffer.str());
  pl.fComment = string(doc["comment"].get_string().value).c_str();
  pl.fHash    = string(doc["hash"].get_string().value).c_str();
  pl.fBLOB    = string(doc["BLOB"].get_string().value).c_str();

  return pl;
}


