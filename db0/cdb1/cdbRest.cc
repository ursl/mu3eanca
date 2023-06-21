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
  
  cdbAbs::init();
}


// ----------------------------------------------------------------------
void cdbRest::readGlobalTags() {
  fGlobalTags.clear();
  cout << "cdbRest::readGlobalTags() fGT = " << fGT << endl;
  if (!fValidGlobalTags) {
    doCurl("globaltags");
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

        if (string::npos != tname.find(fGT)) {
          bsoncxx::array::view subarr{doc["tags"].get_array()};
          for (bsoncxx::array::element ele : subarr) {
            string tname = string(ele.get_string().value).c_str();
            fTags.push_back(tname); 
          }
        }
      }

    }
    if (fVerbose > 0) {
      cout << "cdbRest::readGlobalTags()> fGlobalTags = ";
      print(fGlobalTags);
    }

    return;
  }
}


// ----------------------------------------------------------------------
void cdbRest::readTags() {
  // -- do nothing here, all tags were filled in readGlobalTags already
  if (fVerbose > 0) {
    cout << "cdbRest::readTags()> for GT = " << fGT << ": ";
    print(fTags);
  }
}


// ----------------------------------------------------------------------
void cdbRest::readIOVs() {
  fCurlReadBuffer.clear();
  doCurl("iovs");
  bsoncxx::document::value doc0 = bsoncxx::from_json(fCurlReadBuffer);

  for (auto idoc : doc0) {
    bsoncxx::array::view subarr{idoc.get_array()};
    for (auto ele : subarr) {
      bsoncxx::document::view doc = ele.get_document();
      string tname = string(doc["tag"].get_string().value).c_str();
      // -- look only at tags in fGT
      if (fTags.end() == find(fTags.begin(), fTags.end(), tname)) continue;
      bsoncxx::array::view subarr{doc["iovs"].get_array()};
      vector<int> viov; 
      for (bsoncxx::array::element ele : subarr) {
        int iov = ele.get_int32().value;
        viov.push_back(iov);
      }
      fIOVs.insert(make_pair(tname, viov)); 
    }
  }

  if (fVerbose > 1) {
    cout << "cdbRest::readIOVs>" << endl;
    print(fIOVs);
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

  fCurlReadBuffer.clear();
  stringstream sstr;
  sstr << "\"hash\": \"" << hash << "\"";
  string theFilter = sstr.str();
  doCurl("payloads", theFilter);
  bsoncxx::document::value doc0 = bsoncxx::from_json(fCurlReadBuffer);

  // -- initialize with default
  std::stringstream sspl;
  sspl << "(cdbRest>  hash = " << hash 
       << " not found)";
  payload pl;
  pl.fComment = sspl.str();



  return pl;
}


// ----------------------------------------------------------------------
void cdbRest::doCurl(string collection, string filter, string api) {
  CURL *curl = curl_easy_init();

  if (!curl) {
    cout << "cdbRest::init()> ERROR failed to setup curl?!" << endl;
    exit(0);
  }

  fCurlReadBuffer.clear();
  string sapi("");
  if (string::npos != api.find("findOne")) {
    sapi = fURIfindOne;
  } else if (string::npos != api.find("find")) {
    sapi = fURIfind;
  } else {
    sapi = fURIfind;
  }

  curl_easy_setopt(curl, CURLOPT_URL, sapi.c_str());

  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "Content-Type: application/json");
  headers = curl_slist_append(headers, "Access-Control-Request-Headers: *");
  headers = curl_slist_append(headers, fApiKey.c_str());
  
  //  curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cdbRestWriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fCurlReadBuffer);

  stringstream sstr;
  sstr << "{\"collection\":\"" << collection
       << "\", \"database\":\"mu3e\", \"dataSource\":\"cdb0\"";
  if (string::npos  == filter.find("nada")) {
    sstr << "\"filter\": " << filter << "}"; //    {\"hash\": \"tag_pixelalignment_dt23intrun_iov_200\"};
  } else {
    sstr << "}";
  }
  string theString = sstr.str(); 
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, theString.c_str());
 
  CURLcode curlRes = curl_easy_perform(curl);

  cout << "==:cdbRest::doCurl(\"" << collection << "\") got "
       << fCurlReadBuffer
       << endl;
}
