#include "cdbRest.hh"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <dirent.h>    /// for directory reading

#include <curl/curl.h> 

#include "base64.hh"
#include "cdbUtil.hh"

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

  fURIfindOne = fURI + "/findOne"; 
  fURIfind    = fURI + "/findAll"; 
  
  cdbAbs::init();
}


// ----------------------------------------------------------------------
vector<string> cdbRest::readGlobalTags() {
  vector<string> v;
  doCurl("globaltags", "nada", "findAll");
  
  if (1) {
    vector<string> vgt = jsonGetValueVector(fCurlReadBuffer, "gt");
    for (auto it : vgt) {
      v.push_back(it); 
    }
  }
  
  if (fVerbose > 0) {
    cout << "cdbRest::readGlobalTags()> ";
    print(v);
  }
  
  return v;
}


// ----------------------------------------------------------------------
vector<string> cdbRest::readTags(string gt) {
  vector<string> v;
  doCurl("globaltags", "nada", "findAll");
  
  if (1) {
    vector<string> vgt = jsonGetValueVector(fCurlReadBuffer, "gt");
    for (auto it: vgt) {
      if (it != gt) continue;
      string stags = jsonGetVector(fCurlReadBuffer, it);
      v = split(stags, ',');
    }

  }

  if (fVerbose > 0) {
    cout << "**cdbRest::readGlobalTags()> tags = ";
    print(v);
  }
  
  return v;
}


// ----------------------------------------------------------------------
map<string, vector<int>> cdbRest::readIOVs(vector<string> tags) {
  map<string, vector<int>> m;

  for (auto it: tags) {
    fCurlReadBuffer.clear();
    doCurl("tags", it, "findOne");

    vector<int> viov;
    string sarr = jsonGetVector(fCurlReadBuffer, "iovs");

    vector<string> subarr = split(sarr, ',');
    if (subarr.size() > 0) {
      for (auto it: subarr) {
        viov.push_back(stoi(it));
      }
    } else {
      viov.push_back(stoi(sarr));
    }
    m.insert(make_pair(it, viov)); 
  }

  if (fVerbose > 0) {
    cout << "**cdbRest::readIOVs>" << endl;
    print(m);
  }
  return m;
}  


// ----------------------------------------------------------------------
runRecord cdbRest::getRunRecord(int irun) {
  // -- initialize with default
  std::stringstream sspl;
  sspl << "(cdbRest>  runRecord for run = " << to_string(irun)
       << " not found)";
  runRecord rr;
  rr.fEORComments = sspl.str();
  
  fCurlReadBuffer.clear();
  doCurl("runrecords", to_string(irun), "findOne");
  stripOverhead();
  cout << "fCurlReadBuffer ->" << fCurlReadBuffer << "<-" << endl;
  cout << "hallo" << endl;
  rr.fBORRunNumber     = stoi(jsonGetValue(fCurlReadBuffer, "Run number"));
  cout << "hallo 2" << endl;
  rr.fBORStartTime     = jsonGetString(fCurlReadBuffer, "Start time");
  rr.fBORSubsystems    = stoi(jsonGetValue(fCurlReadBuffer, "Subsystems"));
  rr.fBORBeam          = stof(jsonGetValue(fCurlReadBuffer, "Beam"));
  rr.fBORShiftCrew     = jsonGetString(fCurlReadBuffer, "Shift crew");
  
  rr.fEORStopTime      = jsonGetString(fCurlReadBuffer, "Stop time");
  rr.fEOREvents        = stoi(jsonGetValue(fCurlReadBuffer, "Events"));
  rr.fEORFileSize      = stoi(jsonGetValue(fCurlReadBuffer, "File size"));
  rr.fEORDataSize      = stoi(jsonGetValue(fCurlReadBuffer, "Data size"));
  rr.fEORComments      = jsonGetString(fCurlReadBuffer, "Comments");
  
  rr.fConfigurationKey = jsonGetString(fCurlReadBuffer, "Configuration key");

  return rr;
}


// ----------------------------------------------------------------------
cfgPayload cdbRest::getConfig(string hash) {

  fCurlReadBuffer.clear();
  doCurl("configs", hash, "findOne");
  stripOverhead();

  
  
  cfgPayload cfg;

  // -- initialize with default
  std::stringstream sspl;
  sspl << "(cdbRest>  hash = " << hash 
       << " not found)";
  payload pl;
  cfg.fCfgString = sspl.str();
  
  cfg.fHash      = jsonGetValue(fCurlReadBuffer, "cfgHash");
  cfg.fDate      = jsonGetValue(fCurlReadBuffer, "cfgDate");
  cfg.fCfgString = base64_decode(jsonGetString(fCurlReadBuffer, "cfgString"));
  return cfg;
}


// ----------------------------------------------------------------------
payload cdbRest::getPayload(string hash) {
 
  fCurlReadBuffer.clear();
  doCurl("payloads", hash, "findOne");
  stripOverhead();

  // -- initialize with default
  std::stringstream sspl;
  sspl << "(cdbRest>  hash = " << hash 
       << " not found)";
  payload pl;
  pl.fComment = sspl.str();

  pl.fComment = jsonGetString(fCurlReadBuffer, "comment");
  pl.fHash    = jsonGetString(fCurlReadBuffer, "hash");
  pl.fBLOB    = base64_decode(jsonGetString(fCurlReadBuffer, "BLOB"));
  
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
    sapi = fURIfindOne + "/" + collection + "/" + filter;
  } else if (string::npos != api.find("findAll")) {
    sapi = fURIfind + "/" + collection;
  } else {
    sapi = fURIfind;
  }

  curl_easy_setopt(curl, CURLOPT_URL, sapi.c_str());

  struct curl_slist *headers = NULL;
  //  headers = curl_slist_append(headers, "Content-Type: application/json");
  //  headers = curl_slist_append(headers, "Access-Control-Request-Headers: *");
  headers = curl_slist_append(headers, fApiKey.c_str());
  
  //  curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cdbRestWriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fCurlReadBuffer);

  CURLcode curlRes = curl_easy_perform(curl);

  if (0) cout << "==:cdbRest::doCurl(\"" << collection << "\"): "
              << fCurlReadBuffer
              << endl;
}


// ----------------------------------------------------------------------
void cdbRest::stripOverhead() {
  //old version  replaceAll(fCurlReadBuffer, "{\"documents\":", "");
  //fCurlReadBuffer.pop_back();
}
