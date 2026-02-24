#include "cdbJSON.hh"

#include "base64.hh"
#include "cdbUtil.hh"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <dirent.h>  /// for directory reading
#include <iomanip>

using namespace std;

// ----------------------------------------------------------------------
cdbJSON::cdbJSON(string uri, int verbose) : cdbAbs(uri, verbose) {
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
  rr.fEORComments = sspl.str();
  
  // -- read runRecord: try block subdir first, then flat fallback
  ifstream INS;
  string subpath = runRecordSubPathFromRun(irun);
  string filename = fURI + "/runrecords/" + subpath;
  INS.open(filename);
  if (INS.fail()) {
    std::ostringstream oss;
    oss << "runRecord_" << irun << ".json";
    filename = fURI + "/runrecords/" + oss.str();
    INS.clear();
    INS.open(filename);
  }
  if (INS.fail()) {
    cout << "Error failed to open ->" << filename << "<-" << endl;
    return rr;
  }
  
  std::stringstream buffer;
  buffer << INS.rdbuf();
  INS.close();
  
  cout << "cdbJSON::getRunRecord() Read " << filename << endl;
  string jstring = buffer.str();
  rr.fillFromJson(jstring);

  return rr;
}


// ----------------------------------------------------------------------
vector<string> cdbJSON::getAllRunNumbers() {
  vector<string> v;
  string dir = fURI + "/runrecords";
  vector<string> vfiles = allRunRecordPaths(dir);
  for (auto it: vfiles) {
    string::size_type pos = it.rfind("/");
    string file = it.substr(pos+1);
    replaceAll(file, "runRecord_", "");
    replaceAll(file, ".json", "");
    v.push_back(file);
  }
  return v;
}

// ----------------------------------------------------------------------
vector<string> cdbJSON::getAllRunNumbers(string selection, string det) {
  vector<string> v;
  string dir = fURI + "/runrecords";
  vector<string> vfiles = allRunRecordPaths(dir);
  for (auto it: vfiles) {
    string::size_type pos = it.rfind("/");
    string file = it.substr(pos+1);
    replaceAll(file, "runRecord_", "");
    replaceAll(file, ".json", "");
    // FIXME add filtering
    (void)selection;
    (void)det;
    v.push_back(file);
  }
  return v;
}

// ----------------------------------------------------------------------
cfgPayload cdbJSON::getConfig(string hash) {
  // -- initialize with default
  std::stringstream sspl;
  sspl << "(cdbJSON>  config for hash = " << hash
       << " not found)";
  cfgPayload cfg;
  
  // -- read config
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
  cfg.fHash      = jsonGetString(jstring, "cfgHash");
  cfg.fDate      = jsonGetString(jstring, "cfgDate");
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
  
  // -- read payload: try tag/block subdir first, then flat fallback
  ifstream INS;
  string subpath = payloadSubPathFromHash(hash);
  string filename = fURI + "/payloads/" + subpath;
  INS.open(filename);
  if (INS.fail() && subpath != hash) {
    filename = fURI + "/payloads/" + hash;
    INS.clear();
    INS.open(filename);
  }
  if (INS.fail()) {
    cout << "Error failed to open ->" << filename << "<-" << endl;
    return pl;
  }
  
  std::stringstream buffer;
  buffer << INS.rdbuf();
  INS.close();
  
  if (fVerbose > 0) cout << "cdbJSON::getPayload() Read " << filename << endl;
  
  string jstring = buffer.str();
  pl.fHash       = jsonGetString(jstring, "hash");
  pl.fComment    = jsonGetString(jstring, "comment");
  pl.fSchema     = jsonGetString(jstring, "schema");
  pl.fDate       = jsonGetString(jstring, "date");
  pl.fBLOB       = base64_decode(jsonGetString(jstring, "BLOB"));
  
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
