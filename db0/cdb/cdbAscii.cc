#include "cdbAscii.hh"

#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

// ----------------------------------------------------------------------
// cdbAscii::cdbAscii() : cdb() {
// }

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
      return fGlobalTags;
    }
    string sline;
    while (getline(INS, sline)) {
      vector<string> tokens = split(sline, ',');
      if (tokens.size() > 0) fGlobalTags.push_back(tokens[0]);
    }
    INS.close();
    fValidGlobalTags = true;
    return fGlobalTags; 
  } else {
    return std::vector<std::string>();
  }

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
