#include "calPixelEfficiency.hh"

#include "cdbUtil.hh"

#include <iostream>
#include <sstream>


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
  cout << "calPixelEfficiency::calculate> calculate pixel efficiency with hash ->" << hash << "<-" << endl;
  fMapConstants.clear();
  string spl = fTagIOVPayloadMap[hash].fBLOB;
  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();
  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  cout << "header: " << hex << header << dec << endl;
  uint32_t pixel(0);
  double efficiency(0);
  while (ibuffer != buffer.end()) {
    pixel = blob2UnsignedInt(getData(ibuffer));
    efficiency = blob2Double(getData(ibuffer));
    constants a;
    a.id = pixel;
    a.efficiency = efficiency;
    fMapConstants.insert(make_pair(a.id, a));
  }
  cout << " inserted " << fMapConstants.size() << " constants" << endl;

  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();
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
    s << dumpArray(double2Blob(a.efficiency));
  }
  return s.str();
}


// ----------------------------------------------------------------------
void calPixelEfficiency::printBLOB(string s, int verbosity) {
  cout << "calPixelEfficiency::printBLOB> print BLOB" << endl;
  std::vector<char> buffer(s.begin(), s.end());
  std::vector<char>::iterator ibuffer = buffer.begin();
  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  cout << "header: " << hex << header << dec << endl;
  uint32_t chipID(0);
  double efficiency(0);
  while (ibuffer != buffer.end()) {
    chipID = blob2UnsignedInt(getData(ibuffer));
    efficiency = blob2Double(getData(ibuffer));
    cout << "chipID: " << chipID << " efficiency: " << efficiency << endl;
  }
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
    spl += sline;
    spl += ",";
  }
  INS.close();

  spl.pop_back();
  vector<string> tokens = split(spl, ',');

  for (unsigned int it = 0; it < tokens.size(); it += 2) {
    constants a;
    int idx = it;
    a.id = ::stoi(tokens[idx++]);
    a.efficiency = ::stod(tokens[idx++]);

    fMapConstants.insert(make_pair(a.id, a));
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
  for (auto &c : fMapConstants) {
    ONS << c.first << "," << c.second.efficiency << endl;
  }
  ONS.close();
}