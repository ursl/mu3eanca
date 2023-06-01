#include "cdbAscii.hh"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>

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
}


// ----------------------------------------------------------------------
string cdbAscii::getPayload(int irun, string tag) {
  int iov = whichIOV(irun, tag);

  // -- hash is a misnomer here
  std::stringstream ssHash;
  ssHash << "tag_" << tag << "_iov_" << iov;
  string hash = ssHash.str();

  std::stringstream sspl;
  sspl << "(cdbAscii> run = " << irun << " tag = " << tag 
       << " hash = " << hash 
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


// ----------------------------------------------------------------------
vector<string> cdbAscii::split(const string &s, char delim) {
  vector<string> elems;
  split(s, delim, elems);
  return elems;
}

// ----------------------------------------------------------------------
void cdbAscii::split(const string &s, char delim, vector<string> &elems) {
  stringstream ss(s);
  string item;
  while (getline(ss, item, delim)) {
    elems.push_back(item);
  }
  //  return elems;
}


// ----------------------------------------------------------------------
void cdbAscii::replaceAll(string &str, const string &from, const string &to) {
  if (from.empty()) return;
  size_t start_pos = 0;
  while((start_pos = str.find(from, start_pos)) != string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
  }
}


// ----------------------------------------------------------------------
void cdbAscii::cleanupString(string &s) {
  replaceAll(s, "\t", " ");
  string::size_type s1 = s.find("#");
  if (string::npos != s1) s.erase(s1);
  if (0 == s.length()) return;
  string::iterator new_end = unique(s.begin(), s.end(), bothAreSpaces);
  s.erase(new_end, s.end());
  if (s.substr(0, 1) == string(" ")) s.erase(0, 1);
  if (s.substr(s.length()-1, 1) == string(" ")) s.erase(s.length()-1, 1);
}


