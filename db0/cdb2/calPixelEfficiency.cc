#include "calPixelEfficiency.hh"

#include "cdbUtil.hh"

#include <iostream>
#include <sstream>
#include <iomanip>


using namespace std;

// ----------------------------------------------------------------------
calPixelEfficiency::calPixelEfficiency(cdbAbs *db) : calAbs(db) {
}

// ----------------------------------------------------------------------
calPixelEfficiency::calPixelEfficiency(cdbAbs *db, string tag) : calAbs(db, tag) {
  if (fVerbose) cout << "calPixelEfficiency created and registered with tag ->"
                     << fTag << "<-"
                     << endl;
}


// ----------------------------------------------------------------------
calPixelEfficiency::~calPixelEfficiency() {
  cout << "this is the end of calPixelEfficiency with tag ->" << fTag << "<-" << endl;
}

// ----------------------------------------------------------------------
void calPixelEfficiency::calculate(string hash) {
  cout << "calPixelEfficiency::calculate> calculate with hash ->" << hash << "<-";
  fMapConstants.clear();
  string spl = fTagIOVPayloadMap[hash].fBLOB;
  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();
  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  cout << " header: " << hex << header << dec;
  uint32_t chip(0);
  int n(0), meanEfficiency(0.);
  while (ibuffer != buffer.end()) {
    chip = blob2UnsignedInt(getData(ibuffer));
    n = blob2Int(getData(ibuffer));
    constants a;
    a.id = chip;
    a.vefficiency.resize(n);
    for (int i = 0; i < n; i++) {
      a.vefficiency[i] = blob2Double(getData(ibuffer));
    }
    fMapConstants.insert(make_pair(a.id, a));
  }
  cout << " inserted " << fMapConstants.size() << " constants" << endl;
}

// ----------------------------------------------------------------------
string calPixelEfficiency::makeBLOB() {
  cout << "calPixelEfficiency::makeBLOB> make BLOB" << endl;
  stringstream s;
  unsigned int header(0xdeadface);
  s << dumpArray(uint2Blob(header));
  for (auto it: fMapConstants) {
    constants a = it.second;
    s << dumpArray(uint2Blob(a.id));
    s << dumpArray(int2Blob(a.vefficiency.size()));
    for (int i = 0; i < a.vefficiency.size(); i++) {
      s << dumpArray(double2Blob(a.vefficiency[i]));
    }
  }
  return s.str();
}


// ----------------------------------------------------------------------
void calPixelEfficiency::printBLOB(string s, int verbosity) {
  cout << printBLOBString(s, verbosity) << endl;
}

// ----------------------------------------------------------------------
string calPixelEfficiency::printBLOBString(string s, int verbosity) {
  stringstream ss;
  ss << "calPixelEfficiency::printBLOB> print BLOB" << endl;
  std::vector<char> buffer(s.begin(), s.end());
  std::vector<char>::iterator ibuffer = buffer.begin();
  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  ss << "header: " << hex << header << dec << endl;
  uint32_t chipID(0);
  int n(0);
  vector<double> vefficiency;
  while (ibuffer != buffer.end()) {
    chipID = blob2UnsignedInt(getData(ibuffer));
    n = blob2Int(getData(ibuffer));
    vefficiency.resize(n);
    for (int i = 0; i < n; i++) {
      vefficiency[i] = blob2Double(getData(ibuffer));
    }
    ss << "chipID: " << chipID << " n: " << n << ", efficiency: ";
    for (int i = 0; i < n; i++) {
      ss << setprecision(7) << vefficiency[i];
      if (i < n - 1) ss << ",";
      else ss << endl;
    }
  }
  return ss.str();
}

// ----------------------------------------------------------------------
void calPixelEfficiency::readCsv(string filename) {
  string spl("");
  ifstream INS(filename);
  if (!INS.is_open()) {
    cout << "calPixelEfficiency::readCsv> Error, file " + filename + " not found" << endl;
    return;
  }

  string sline;
  while (getline(INS, sline)) {
    if (string::npos != sline.find("#")) continue;
    vector<string> tokens = split(sline, ',');
    constants a;
    a.id = ::stoi(tokens[0]);
    int n = ::stoi(tokens[1]);
    a.vefficiency.resize(n);
    for (int i = 0; i < n; i++) {
      a.vefficiency[i] = ::stod(tokens[2 + i]);
    }
    fMapConstants.insert(make_pair(a.id, a));
  }
  INS.close();

  // -- check whether these are percent efficiencies
  double meanEfficiency(0.0);
  int n(0);
  for (auto &c : fMapConstants) {
    for (int i = 0; i < c.second.vefficiency.size(); i++) {
      if (c.second.vefficiency[i] > 0.0) {
        meanEfficiency += c.second.vefficiency[i];
        n++;
      }
    }
  }
  meanEfficiency /= n;
  if (meanEfficiency > 1.0) {
    cout << "calPixelEfficiency::readCsv> Note: mean efficiency " << meanEfficiency << " is greater than 1.0, assuming this is in percent and converting to fraction" << endl;
    for (auto &c : fMapConstants) {
      for (int i = 0; i < c.second.vefficiency.size(); i++) {
        c.second.vefficiency[i] /= 100.0;
      }
    }
  }
  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();
}

// ----------------------------------------------------------------------
void calPixelEfficiency::writeCsv(string filename) {
  ofstream ONS(filename);
  if (!ONS.is_open()) {
    cout << "calPixelEfficiency::writeCsv> Error, file " + filename + " not opened" << endl;
    return;
  }
  ONS << "#" << fSchema << endl;
  for (auto &c : fMapConstants) {
    ONS << c.first << "," << c.second.vefficiency.size() << ",";
    for (int i = 0; i < c.second.vefficiency.size(); i++) {
      ONS << fixed << setprecision(7) << c.second.vefficiency[i];
      if (i < c.second.vefficiency.size() - 1) ONS << ",";
      else ONS << endl;
    }
  }
  ONS.close();
}