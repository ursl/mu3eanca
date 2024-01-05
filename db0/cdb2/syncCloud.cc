#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <dirent.h>  /// for directory reading

#include <curl/curl.h> 


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
// syncCloud
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

string gApiKey(""), gCurlReadBuffer; 

string gURI("https://eu-central-1.aws.data.mongodb-api.com/app/data-pauzo/endpoint/data/v1/action/");

// ----------------------------------------------------------------------
static size_t cdbRestWriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  ((std::string*)userp)->append((char*)contents, size * nmemb);
  return size * nmemb;
}

// ----------------------------------------------------------------------
void writeCurl(string collection, string payload) {
  CURL *curl = curl_easy_init();
 
  if (!curl) {
    cout << "cdbRest::init()> ERROR failed to setup curl?!" << endl;
    exit(0);
  }

  gCurlReadBuffer.clear();
  string sapi = gURI + ("insertOne");

  curl_easy_setopt(curl, CURLOPT_URL, sapi.c_str());

  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "Content-Type: application/json");
  headers = curl_slist_append(headers, "Access-Control-Request-Headers: *");
  headers = curl_slist_append(headers, gApiKey.c_str());
  
  curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cdbRestWriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &gCurlReadBuffer);

  stringstream sstr;
  sstr << "{\"collection\":\"" << collection
       << "\", \"database\":\"mu3e\", \"dataSource\":\"cdb0\",";
  sstr << " \"document\": " << payload;
  sstr << "}";

  string theString = sstr.str(); 
  cout << "theString = " << theString << endl;
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, theString.c_str());
 
  if (!gDBX) CURLcode curlRes = curl_easy_perform(curl);

  if (0) cout << "==:cdbRest::doCurl(\"" << collection << "\"): "
              << gCurlReadBuffer
              << endl;
}


// ----------------------------------------------------------------------
void clearCollection(string collection, string field) {
  vector<string> idCollections;
  // -- read documents to delete
  if (1) {
    CURL *curl = curl_easy_init();
    
    if (!curl) {
      cout << "cdbRest::init()> ERROR failed to setup curl?!" << endl;
      exit(0);
    }
    
    gCurlReadBuffer.clear();
    string sapi = gURI + ("find");
    
    curl_easy_setopt(curl, CURLOPT_URL, sapi.c_str());
    
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "Access-Control-Request-Headers: *");
    headers = curl_slist_append(headers, gApiKey.c_str());
    
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cdbRestWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &gCurlReadBuffer);
    
    stringstream sstr;
    sstr << "{\"collection\":\"" << collection
         << "\", \"database\":\"mu3e\", \"dataSource\":\"cdb0\"}";
    
    string theString = sstr.str(); 
    cout << "theString = " << theString << endl;
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, theString.c_str());
    
    CURLcode curlRes = curl_easy_perform(curl);
    
    bsoncxx::document::value doc0 = bsoncxx::from_json(gCurlReadBuffer);
    cout << bsoncxx::to_json(doc0, bsoncxx::ExtendedJsonMode::k_relaxed) << endl;
    
    for (auto idoc : doc0) {
      bsoncxx::array::view subarr{idoc.get_array()};
      for (auto ele : subarr) {
        cout << "ele.type() = " <<  bsoncxx::to_string(ele.type()) << endl;
        bsoncxx::document::view doc = ele.get_document();
        cout << bsoncxx::to_json(doc) << endl;
        string tname = string(doc[field].get_string().value).c_str();
        idCollections.push_back(tname); 
      }
    }
    
    for (auto it: idCollections) {
      cout << "-> " << it << endl;
    }
    
    curl_easy_cleanup(curl);
  }

  // -- execute deletions
  if (1) {
    CURL *curl = curl_easy_init();
    
    if (!curl) {
      cout << "cdbRest::init()> ERROR failed to setup curl?!" << endl;
      exit(0);
    }
    
    gCurlReadBuffer.clear();
    string sapi = gURI + ("deleteOne");
    
    curl_easy_setopt(curl, CURLOPT_URL, sapi.c_str());
    
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "Access-Control-Request-Headers: *");
    headers = curl_slist_append(headers, gApiKey.c_str());
    
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cdbRestWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &gCurlReadBuffer);
    
    for (auto it: idCollections) {
      stringstream sstr;
      sstr << "{\"collection\":\"" << collection
           << "\", \"database\":\"mu3e\", \"dataSource\":\"cdb0\",";
      sstr << "\"filter\": { \"" << field << "\": \"" << it << "\" } }";
      string theString = sstr.str(); 

      cout << "deleting theString = " << theString << endl;
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, theString.c_str());
    
      if (!gDBX) CURLcode curlRes = curl_easy_perform(curl);
    }
    
    curl_easy_cleanup(curl);
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
    if (!strcmp(argv[i], "--key"))  {gApiKey += string(argv[++i]);}
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
    return 0;
  } 
  
  while ((entry=readdir(folder))) {
    if (8 == entry->d_type) {
      vfiles.push_back(dirName + "/" + entry->d_name);
    }
  }
  closedir(folder);
  
  sort(vfiles.begin(), vfiles.end());    

  gApiKey = "api-key: " + gApiKey;

  cout << "clearCollection(" << dirName << ", " << tagDel[dirName] << ");" << endl;
  cout << "gApiKey ->" << gApiKey << "<-" << endl;
  clearCollection(dirName, tagDel[dirName]);

  if (onlyDelete) return 0;
  
  string collectionContents;
  ifstream INS;
  for (auto it: vfiles) {
    INS.open(it);
    getline(INS, collectionContents);
    INS.close();
    cout << "writeCurl(" << dirName << ", " << collectionContents << ")" << endl;
    writeCurl(dirName, collectionContents);
  }
  
	return 0;
}
