#include "cdbRest.hh"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <dirent.h>    /// for directory reading

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
cdbRest::cdbRest(string gt, string uri, int verbose) : cdbAbs(gt, uri, verbose) {
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

  fURIfindOne = fURI + "findOne"; 
  fURIfind    = fURI + "find"; 
  
  fCurl = curl_easy_init();

  struct curl_slist *headers=NULL;
  if (fCurl) {
    curl_easy_setopt(fCurl, CURLOPT_URL, fURIfindOne.c_str());

    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "Access-Control-Request-Headers: *");
    headers = curl_slist_append(headers, fApiKey.c_str());

    curl_easy_setopt(fCurl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(fCurl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(fCurl, CURLOPT_WRITEFUNCTION, cdbRestWriteCallback);
    curl_easy_setopt(fCurl, CURLOPT_WRITEDATA, &fCurlReadBuffer);

    curl_easy_setopt(fCurl, CURLOPT_POSTFIELDS, "{\"collection\":\"payloads\", \"database\":\"mu3e\", \"dataSource\":\"cdb0\", \"filter\": {\"hash\": \"tag_pixelalignment_dt23intrun_iov_200\"}}");

    //    fCurlRes = curl_easy_perform(fCurl);
    //    curl_easy_cleanup(fCurl);
    //    std::cout << fCurlReadBuffer << std::endl;
  }
  cdbAbs::init();
}


// ----------------------------------------------------------------------
void cdbRest::readGlobalTags() {
  fGlobalTags.clear();
  cout << "cdbRest::readGlobalTags()" << endl;
  if (!fValidGlobalTags) {
    curl_easy_setopt(fCurl, CURLOPT_URL, fURIfind.c_str());
    curl_easy_setopt(fCurl, CURLOPT_POSTFIELDS, "{\"collection\":\"globaltags\", \"database\":\"mu3e\", \"dataSource\":\"cdb0\"}");
    fCurlRes = curl_easy_perform(fCurl);
    curl_easy_cleanup(fCurl);

    bsoncxx::document::value doc0 = bsoncxx::from_json(fCurlReadBuffer);
    //cout << bsoncxx::to_json(doc0, bsoncxx::ExtendedJsonMode::k_relaxed) << endl;

    for (auto idoc : doc0) {
      bsoncxx::array::view subarr{idoc.get_array()};
      for (auto ele : subarr) {
        //cout << "ele.type() = " <<  bsoncxx::to_string(ele.type()) << endl;
        bsoncxx::document::view doc = ele.get_document();
        //cout << bsoncxx::to_json(doc) << endl;
        string tname = string(doc["gt"].get_string().value).c_str();
        fGlobalTags.push_back(tname); 
      }

    }
    if (fVerbose > 0) {
      cout << "cdbMongo::readGlobalTags()> fGlobalTags = ";
      print(fGlobalTags);
    }

    return;
  }
}


// ----------------------------------------------------------------------
void cdbRest::readTags() {
  fTags.clear();
  curl_easy_setopt(fCurl, CURLOPT_URL, fURIfind.c_str());
  curl_easy_setopt(fCurl, CURLOPT_POSTFIELDS, "{\"collection\":\"globaltags\", \"database\":\"mu3e\", \"dataSource\":\"cdb0\"}");
  fCurlRes = curl_easy_perform(fCurl);
  curl_easy_cleanup(fCurl);
  
  bsoncxx::document::value doc0 = bsoncxx::from_json(fCurlReadBuffer);
  //cout << bsoncxx::to_json(doc0, bsoncxx::ExtendedJsonMode::k_relaxed) << endl;


  exit(0);
  
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


