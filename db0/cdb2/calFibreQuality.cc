#include "calFibreQuality.hh"

#include "cdbUtil.hh"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <map>
#include <vector>

#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::ordered_json;

// ----------------------------------------------------------------------
calFibreQuality::calFibreQuality(cdbAbs *db) : calAbs(db) {
}


// ----------------------------------------------------------------------
bool calFibreQuality::getNextID(uint32_t &ID) {
  if (fMapConstantsIt == fMapConstants.end()) {
    // -- reset
    ID = 999999;
    fMapConstantsIt = fMapConstants.begin();
    return false;
  } else {
    ID = fMapConstantsIt->first;
    fMapConstantsIt++;
  }
  return true;
}


// ----------------------------------------------------------------------
calFibreQuality::calFibreQuality(cdbAbs *db, string tag) : calAbs(db, tag) {
  if (fVerbose) cout << "calFibreQuality created and registered with tag ->" << fTag << "<-"
                     << endl;
}


// ----------------------------------------------------------------------
calFibreQuality::~calFibreQuality() {
  fMapConstants.clear();
  cout << "this is the end of calFibreQuality with tag ->" << fTag << "<-" << endl;
}


// ----------------------------------------------------------------------
calFibreQuality::Status calFibreQuality::getAsicStatus(uint32_t asicID) {
  if (fMapConstants.find(asicID) == fMapConstants.end()) {
    return ChannelNotFound;
  }
  if (getAsicLock(asicID) && getAsicHasData(asicID)) {
    return Good;
  } else if (!getAsicLock(asicID)) {
    return Noisy;
  } else if (!getAsicHasData(asicID)) {
    return Dead;
  }
  return Unset;
}

// ----------------------------------------------------------------------
bool calFibreQuality::getAsicLock(uint32_t asicID) {
  if (fMapConstants.find(asicID) == fMapConstants.end()) {
    return false;
  }
  return fMapConstants[asicID].lock;
}

// ----------------------------------------------------------------------
bool calFibreQuality::getAsicHasData(uint32_t asicID) {
  if (fMapConstants.find(asicID) == fMapConstants.end()) {
    return false;
  }
  return fMapConstants[asicID].hasData;
}

// ----------------------------------------------------------------------
int calFibreQuality::getAsicQuality(uint32_t asicID) {
  if (fMapConstants.find(asicID) == fMapConstants.end()) {
    return -1;
  }
  return fMapConstants[asicID].quality;
}

// ----------------------------------------------------------------------
double calFibreQuality::getAsicThreshold(uint32_t asicID) {
  if (fMapConstants.find(asicID) == fMapConstants.end()) {
    return -1.0;
  }
  return fMapConstants[asicID].threshold;
}

// ----------------------------------------------------------------------
double calFibreQuality::getAsicEfficiency(uint32_t asicID) {
  if (fMapConstants.find(asicID) == fMapConstants.end()) {
    return -1.0;
  }
  return fMapConstants[asicID].efficiency;
}

// ----------------------------------------------------------------------
void calFibreQuality::calculate(string hash) {
  cout << "calFibreQuality::calculate> calculate with hash ->" << hash << "<-";
  fMapConstants.clear();
  string spl = fTagIOVPayloadMap[hash].fBLOB;

  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  cout << " header: " << hex << header << dec;
  while (ibuffer != buffer.end()) {
    constants cq;
    cq.id = blob2UnsignedInt(getData(ibuffer));
    cq.quality = blob2Int(getData(ibuffer));
    cq.lock = blob2Int(getData(ibuffer));
    cq.hasData = blob2Int(getData(ibuffer));
    cq.threshold = blob2Double(getData(ibuffer));
    cq.efficiency = blob2Double(getData(ibuffer));
    fMapConstants.insert(make_pair(cq.id, cq));
  }
  cout << " inserted " << fMapConstants.size() << " constants" << endl;
  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();
}


// ----------------------------------------------------------------------
string calFibreQuality::makeBLOB() {
  stringstream s;
  unsigned int header(0xdeadface);
  s << dumpArray(uint2Blob(header));

  // -- format of fMapConstants
  // chipID => [lock, hasData]
  for (auto it: fMapConstants) {
    s << dumpArray(uint2Blob(it.first));
    s << dumpArray(int2Blob(it.second.quality));
    s << dumpArray(int2Blob(it.second.lock));
    s << dumpArray(int2Blob(it.second.hasData));
    s << dumpArray(double2Blob(it.second.threshold));
    s << dumpArray(double2Blob(it.second.efficiency));
  }
  return s.str();
}


// ----------------------------------------------------------------------
void calFibreQuality::printBLOB(string blob, int verbosity) {
  cout << printBLOBString(blob, verbosity) << endl;
}

// ----------------------------------------------------------------------
string calFibreQuality::printBLOBString(string blob, int verbosity) {
  stringstream s;

  vector<char> buffer(blob.begin(), blob.end());
  vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  s << "calFibreQuality::printBLOB(string)" << endl;
  s << "   header: " << hex << header << dec << endl;

  int cnt(0);
  while (ibuffer != buffer.end()) {
    uint32_t id = blob2UnsignedInt(getData(ibuffer));
    int quality = blob2Int(getData(ibuffer));
    int lock = blob2Int(getData(ibuffer));
    int hasData = blob2Int(getData(ibuffer));
    double threshold = blob2Double(getData(ibuffer));
    double efficiency = blob2Double(getData(ibuffer));
    s << "   id = " << setw(3) << id
         << " quality = " << quality
         << " lock = " << lock
         << " hasData = " << hasData
         << " threshold = " << threshold
         << " efficiency = " << efficiency
         << endl;
    ++cnt;
  }
  s << "calFibreQuality::printBLOB(...) printed status for " << cnt << " Asics" << endl;
  return s.str();
}


// ----------------------------------------------------------------------
void calFibreQuality::writeCSV(string filename) {
 
}

// ----------------------------------------------------------------------
void calFibreQuality::readCSV(string filename) {
  ifstream INS(filename);
  if (!INS.is_open()) {
    cout << "calFibreQuality::readCSV> Error, file " + filename + " not found" << endl;
    return;
  }
  
  fMapConstants.clear();
  
  string sline;
  while (getline(INS, sline)) {
    if (string::npos == sline.find("#")) {
      vector<string> tokens = split(sline, ',');
      constants a;
      uint32_t id = stoi(tokens[0]);
      a.lock = stoi(tokens[1]);
      a.hasData = stoi(tokens[2]);
      a.quality = stoi(tokens[3]);
      a.threshold = stod(tokens[4]);
      a.efficiency = stod(tokens[5]);
      fMapConstants.insert(make_pair(id, a));
    }
  }
  INS.close();

}
