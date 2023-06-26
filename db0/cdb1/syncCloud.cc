#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <dirent.h>  /// for directory reading

#include <curl/curl.h> 

using namespace std;

// ----------------------------------------------------------------------
// syncCloud
// ---------
//
//
// requires json/*/*
//
// Usage:
// bin/syncCloud
// 
// ----------------------------------------------------------------------

string gApiKey, gCurlReadBuffer; 

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
 
  CURLcode curlRes = curl_easy_perform(curl);

  if (0) cout << "==:cdbRest::doCurl(\"" << collection << "\"): "
              << gCurlReadBuffer
              << endl;
}



// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {

  // -- command line arguments
  bool json(0);
  string password("fixme");
  for (int i = 0; i < argc; i++){
  }

  // -- check whether directories for JSONs already exist
  DIR *folder = opendir("json/payloads");
  if (folder == NULL) {
    cout << "error: directory json/payloads not found. Before sync'ing to cloud, create file-based payloads" << endl;
    closedir(folder);
    exit(1);
  }

  ifstream INS("api-key.private");
  getline(INS, gApiKey);
  INS.close();
  gApiKey = "api-key: " + gApiKey;

  string collectionName("json/globaltags/mc23intrun"), collectionContents;
  INS.open(collectionName);
  getline(INS, collectionContents);
  INS.close();
  // cout << "collectionName ->" << collectionName << "<-" << endl;
  // cout << "collectionContents ->" << collectionContents << "<-" << endl;

  writeCurl("globaltags", collectionContents);
  
  
  
	return 0;
}
