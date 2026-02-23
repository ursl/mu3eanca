#include "cdbRest.hh"

#include "base64.hh"
#include "cdbUtil.hh"

#include <curl/curl.h>

#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <dirent.h>    /// for directory reading

using namespace std;


// ----------------------------------------------------------------------
static size_t cdbRestWriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  static_cast<std::string*>(userp)->append(static_cast<const char*>(contents), size * nmemb);
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
    //cout << "fCurlReadBuffer ->" << fCurlReadBuffer << "<-" << endl;
    vector<string> vgt = jsonGetValueVector(fCurlReadBuffer, "gt");
    for (auto it: vgt) {
      if (it != gt) {
        continue;
      }
      string stags = jsonGetVector(fCurlReadBuffer, string("\"") + it + string("\""));
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
    //cout << "it ->" << it << "<- fCurlReadBuffer = " << fCurlReadBuffer << endl;
    string sarr = jsonGetVector(fCurlReadBuffer, "iovs");
    //cout << "sarr = " << sarr << endl;
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
vector<string> cdbRest::getAllRunNumbers() {
  doCurl("runNumbers", "nada", "findAll");
  
  if (0) cout << "fCurlReadBuffer ->" << fCurlReadBuffer << "<-" << endl;
  replaceAll(fCurlReadBuffer, "[", "");
  replaceAll(fCurlReadBuffer, "]", "");
  vector<string> v = split(fCurlReadBuffer, ',');
    
  return v;
}


// ----------------------------------------------------------------------
vector<string> cdbRest::getAllRunNumbers(string selection, string det) {
  doCurl("runNumbers", "nada", "findAll");
  
  if (0) cout << "fCurlReadBuffer ->" << fCurlReadBuffer << "<-" << endl;
  replaceAll(fCurlReadBuffer, "[", "");
  replaceAll(fCurlReadBuffer, "]", "");
  // FIXME add filtering!
  vector<string> v = split(fCurlReadBuffer, ',');
    
  return v;
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
  //cout << "irun = " << irun << " fCurlReadBuffer ->" << fCurlReadBuffer << "<-" << endl;
  if (fCurlReadBuffer == "Not found") {
    return rr;
  }
  if (fCurlReadBuffer == "") {
    sspl.clear();
    sspl << "cdbRest::getRunRecord> runRecord for run = " << irun
         << " ERROR: empty response";
    rr.fEORComments = sspl.str();
    return rr;
  }
  rr.fillFromJson(fCurlReadBuffer);
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
  cfg.fCfgString = jsonGetCfgStringEsc(fCurlReadBuffer, "cfgString");
  replaceAll(cfg.fCfgString, "\\\n", "\n");
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
  
  pl.fHash    = jsonGetString(fCurlReadBuffer, "hash");
  pl.fComment = jsonGetString(fCurlReadBuffer, "comment");
  pl.fSchema  = jsonGetString(fCurlReadBuffer, "schema");
  pl.fDate    = jsonGetString(fCurlReadBuffer, "date");
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
  if (curlRes != CURLE_OK && fVerbose > 0) {
    cout << "cdbRest::doCurl> curl_easy_perform failed: " << curl_easy_strerror(curlRes) << endl;
  }

  if (0) cout << "==:cdbRest::doCurl(\"" << collection << "\"): "
                << " sapi ->" << sapi << "<- result: "
                << fCurlReadBuffer
                << endl;
  
  curl_easy_cleanup(curl);
}


// ----------------------------------------------------------------------
void cdbRest::stripOverhead() {
  //old version  replaceAll(fCurlReadBuffer, "{\"documents\":", "");
  //fCurlReadBuffer.pop_back();
}
