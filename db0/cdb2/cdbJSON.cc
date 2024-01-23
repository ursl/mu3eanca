#include "cdbJSON.hh"

#include "base64.hh"
#include "cdbUtil.hh"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <dirent.h>  /// for directory reading

using namespace std;

// ----------------------------------------------------------------------
cdbJSON::cdbJSON(string gt, string uri, int verbose) : cdbAbs(gt, uri, verbose) {
  init();
}


// ----------------------------------------------------------------------
cdbJSON::~cdbJSON() { }


// ----------------------------------------------------------------------
void cdbJSON::init() {
  fName = "JSON"; 
  cdbAbs::init();
}


// ----------------------------------------------------------------------
vector<string> cdbJSON::readGlobalTags() {
  vector<string> v;
  // -- read global tags from fURI
  string gtdir = fURI + "/globaltags";
  cout << "cdbJSON::readGlobalTags() from  gtdir = " << gtdir << endl;
  vector<string> gtFiles = allFiles(gtdir);
  
  ifstream INS;
  for (auto it: gtFiles) {
    // -- remove everything up to and including the last /
    string::size_type pos = it.rfind("/");
    string file = it.substr(pos+1);
    v.push_back(file);
    if (fVerbose > 0) cout << "JSON read GT " << file << endl;
  }
  return v;
}


// ----------------------------------------------------------------------
vector<string> cdbJSON::readTags(string gt) {
  vector<string> v;
  // -- read global tags from fURI
  string gtdir = fURI + "/globaltags/";

  ifstream INS;
  string gtfile = gtdir + gt;
  INS.open(gtfile);
  if (INS.fail()) {
    cout << "Error failed to open ->" << gtfile << "<-" << endl;
    return v;
  }

  std::stringstream buffer;
  buffer << INS.rdbuf();
  INS.close();

  string lBuffer = buffer.str();
  
  vector<string> subarr = split(jsonGetVector(lBuffer, "tags"), ',');
  for (auto it: subarr) {
    v.push_back(it);
  }

  if (fVerbose > 0) {
    cout << "cdbJSON::readTags> for GT = " << gt << endl;
    print(v);
  }
  return v;
}


// ----------------------------------------------------------------------
map<string, vector<int>> cdbJSON::readIOVs(vector<string> tags) {
  map<string, vector<int>> m;

  // -- read iovs from fURI
  ifstream INS;
  string dir = fURI + "/tags/";
  
  for (auto it: tags) {
    string file = dir + it;
    INS.open(file);
    if (INS.fail()) {
      cout << "Error failed to open ->" << file << "<-" << endl;
      return m;
    }

    std::stringstream buffer;
    buffer << INS.rdbuf();
    INS.close();
    
    string lBuffer = buffer.str();
    
    vector<int> viov;
    string sarr = jsonGetVector(lBuffer, "iovs");
    
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
  
  return m;
}


// ----------------------------------------------------------------------
runRecord cdbJSON::getRunRecord(int irun) {
  // -- initialize with default
  std::stringstream sspl;
  sspl << "(cdbJSON>  runRecord for run = " << to_string(irun)
       << " not found)";
  runRecord rr;
  rr.fRunDescription = sspl.str();
  
  // -- read runRecord for run irun 
  ifstream INS;
  string filename = fURI + "/runrecords/" + to_string(irun);
  INS.open(filename);
  if (INS.fail()) {
    cout << "Error failed to open ->" << filename << "<-" << endl;
    return rr;
  }

  std::stringstream buffer;
  buffer << INS.rdbuf();
  INS.close();
  
  cout << "cdbJSON::getRunRecord() Read " << filename << endl;

  string jstring = buffer.str();
  rr.fRun              = stoi(jsonGetValue(jstring, "run"));
  rr.fRunStart         = jsonGetValue(jstring, "runStart");
  rr.fRunEnd           = jsonGetValue(jstring, "runEnd");
  rr.fRunDescription   = jsonGetValue(jstring, "runDescription");
  rr.fRunOperators     = jsonGetValue(jstring, "runOperators");
  rr.fNFrames          = stoi(jsonGetValue(jstring, "nFrames"));
  rr.fBeamMode         = stoi(jsonGetValue(jstring, "beamMode"));
  rr.fBeamCurrent      = stof(jsonGetValue(jstring, "beamCurrent"));
  rr.fMagnetCurrent    = stof(jsonGetValue(jstring, "magnetCurrent"));
  rr.fConfigurationKey = jsonGetValue(jstring, "configurationKey");
  
  return rr;
}


// ----------------------------------------------------------------------
cfgPayload cdbJSON::getConfig(string hash) {
  // -- initialize with default
  std::stringstream sspl;
  sspl << "(cdbJSON>  config for hash = " << hash
       << " not found)";
  cfgPayload cfg;

  // -- read runRecord for run irun 
  ifstream INS;
  string filename = fURI + "/configs/" + hash;
  INS.open(filename);
  if (INS.fail()) {
    cout << "Error failed to open ->" << filename << "<-" << endl;
    return cfg;
  }

  std::stringstream buffer;
  buffer << INS.rdbuf();
  INS.close();

  string jstring = buffer.str();
  cfg.fHash      = jsonGetValue(jstring, "cfgHash");
  cfg.fDate      = jsonGetValue(jstring, "cfgDate");
  cfg.fCfgString = jsonGetCfgStringEsc(jstring, "cfgString");

  
  return cfg;
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
  
  cout << "cdbJSON::getPayload() Read " << filename << endl;

  string jstring = buffer.str();
  pl.fHash       = jsonGetValue(jstring, "hash");
  pl.fComment    = jsonGetValue(jstring, "comment");
  pl.fBLOB       = base64_decode(jsonGetValue(jstring, "BLOB"));

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
