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
cdbAscii::cdbAscii(string name, string uri) : cdb(name, uri) {
  init();
}


// ----------------------------------------------------------------------
cdbAscii::~cdbAscii() { }


// ----------------------------------------------------------------------
void cdbAscii::init() {
  
}


// ----------------------------------------------------------------------
vector<string> cdbAscii::getGlobalTags() {
  if (!fValidGlobalTags) {
    // -- read global tags from fURI
    ifstream INS;
    string gtname = fURI + "/globaltags.txt";
    INS.open(gtname);
    if (INS.fail()) {
      cout << "Error failed to open ->" << gtname << "<-" << endl;
      return vector<string>();
    }
    string sline;
    while (getline(INS, sline)) {
      vector<string> tmptokens = split(sline, ',');
      vector<string> tokens;
      for (auto it : tmptokens) {
        cleanupString(it);
        tokens.push_back(it);
      }
      if (tokens.size() > 0) {
        fGlobalTags.push_back(tokens[0]);
        vector<string> vtokens = tokens;
        vtokens.erase(vtokens.begin());
        fTagMap.insert(make_pair(tokens[0], vtokens));
      }     
    }
    INS.close();
    fValidGlobalTags = true;
    return fGlobalTags; 
  } else {
    return fGlobalTags;
  }

}


// ----------------------------------------------------------------------
vector<string> cdbAscii::getTags(string gt) {
  return fTagMap[gt];
}


// ----------------------------------------------------------------------
vector<int> cdbAscii::getIovs(std::string tag) {
  if (fIovMap.find(tag) == fIovMap.end()) {
    // -- read iovs from fURI
    ifstream INS;
    string gtname = fURI + "/iovs.txt";
    INS.open(gtname);
    if (INS.fail()) {
      cout << "Error failed to open ->" << gtname << "<-" << endl;
      return vector<int>();
    }
    string sline;
    while (getline(INS, sline)) {
      vector<string> tokens = split(sline, ',');
      if (tokens.size() > 0) {
        vector<int> vtokens;
        for (unsigned int i = 1; i < tokens.size(); ++i) {
          vtokens.push_back(stoi(tokens[i]));
        }
        fIovMap.insert(make_pair(tokens[0], vtokens));
      }     
    }
    INS.close();
    return fIovMap[tag];
  } else {
    return fIovMap[tag];
  }
 
}


// ----------------------------------------------------------------------
string cdbAscii::getPayload(int irun, string t) {
  int iov(-1);
  vector<int> iovs = getIovs(t);
  for (auto it : iovs) {
    if (irun >= it) {
      iov = it;
    }
  }
  
  std::stringstream ssHash;
  ssHash << "tag_" << t << "_iov_" << iov;
  string hash = ssHash.str();

  std::stringstream sspl;
  sspl << "(cdbAscii> run = " << irun << " tag = " << t 
       << " hash = " << hash 
       << " not found)";
  string payload = sspl.str();

  if (fTagIovPayloadMap.find(hash) == fTagIovPayloadMap.end()) {
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
        fTagIovPayloadMap.insert(make_pair(tokens[0], tokens[1]));
      }     
    }
    INS.close();
    return fTagIovPayloadMap[hash];
  } else {
    return fTagIovPayloadMap[hash];
  }

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


