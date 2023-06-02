#include "cdbAscii.hh"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>

#include "util/util.hh"

using namespace std;


// ----------------------------------------------------------------------
bool bothAreSpaces(char lhs, char rhs) {
  return (lhs == rhs) && (lhs == ' ');
}


// ----------------------------------------------------------------------
cdbAscii::cdbAscii(string gt, string uri) : cdb(gt, uri) {
  init();
}


// ----------------------------------------------------------------------
cdbAscii::~cdbAscii() { }


// ----------------------------------------------------------------------
void cdbAscii::init() {
  fName = "ascii"; 
  cdb::init();
}


// ----------------------------------------------------------------------
void cdbAscii::readGlobalTags() {
  fGlobalTags.clear();
  cout << "cdbAscii::readGlobalTags()" << endl;
  if (!fValidGlobalTags) {
    // -- read global tags from fURI
    ifstream INS;
    string gtname = fURI + "/globaltags.txt";
    INS.open(gtname);
    if (INS.fail()) {
      cout << "Error failed to open ->" << gtname << "<-" << endl;
      return;
    }
    string sline;
    while (getline(INS, sline)) {
      vector<string> tokens = split(sline, ',');
      if (tokens.size() > 0) {
        fGlobalTags.push_back(tokens[0]);
      }     
    }
    INS.close();
    fValidGlobalTags = true;
  } 
  return;
}


// ----------------------------------------------------------------------
void cdbAscii::readTags() {
  fTags.clear();
  // -- read global tags from fURI
  ifstream INS;
  string gtname = fURI + "/globaltags.txt";
  INS.open(gtname);
  if (INS.fail()) {
    cout << "Error failed to open ->" << gtname << "<-" << endl;
    return;
  }
  string sline;
  while (getline(INS, sline)) {
    vector<string> tokens = split(sline, ',');
    if (string::npos == tokens[0].find(fGT)) continue;
    if (tokens.size() > 0) {
      for (unsigned int iv = 1; iv < tokens.size(); ++iv) {
        fTags.push_back(tokens[iv]);
      }     
    }
  }
  INS.close();
  if (fVerbose > 0) {
    cout << "cdbAscii::readTags> for GT = " << fGT << endl;
    print(fTags);
  }
  return;
}


// ----------------------------------------------------------------------
void cdbAscii::readIOVs() {
  // -- read iovs from fURI
  ifstream INS;
  string gtname = fURI + "/iovs.txt";
  INS.open(gtname);
  if (INS.fail()) {
    cout << "Error failed to open ->" << gtname << "<-" << endl;
    return;
  }
  string sline;
  while (getline(INS, sline)) {
    vector<string> tokens = split(sline, ',');
    if (tokens.size() > 0) {
      // -- insert only those tags that are in the global tag
      if (fTags.end() == find(fTags.begin(), fTags.end(), tokens[0])) continue;
      vector<int> vtokens;
      for (unsigned int i = 1; i < tokens.size(); ++i) {
        vtokens.push_back(stoi(tokens[i]));
      }
      fIOVs.insert(make_pair(tokens[0], vtokens));
    }     
  }
  INS.close();

  if (fVerbose > 1) {
    cout << "cdbAscii::readIOVs>" << endl;
    print(fIOVs);
  }
  return;
}


// ----------------------------------------------------------------------
string cdbAscii::getPayload(int irun, string tag) {
  string hash = getHash(irun, tag); 
  return getPayload(hash);
}


// ----------------------------------------------------------------------
string cdbAscii::getPayload(string hash) {
  // -- initialize with default
  std::stringstream sspl;
  sspl << "(cdbAscii>  hash = " << hash 
       << " not found)";
  string payload = sspl.str();

  // -- read payloads from fURI
  ifstream INS;
  string gtname = fURI + "/payloads.txt";
  INS.open(gtname);
  if (INS.fail()) {
    cout << "Error failed to open ->" << gtname << "<-" << endl;
    return payload;
  }
  string sline;
  while (getline(INS, sline)) {
    vector<string> tokens = split(sline, ',');
    if (tokens.size() > 0) {
      if (string::npos != tokens[0].find(hash)) {
        payload = tokens[1];
        break;
      }
    }     
  }
  INS.close();

  return payload;
}

