#include "calTileTimeCalibration.hh"

#include "cdbUtil.hh"

#include <iostream>
#include <sstream>
#include <cctype>
#include <algorithm>

#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::ordered_json;

// ----------------------------------------------------------------------
calTileTimeCalibration::calTileTimeCalibration(cdbAbs *db) : calAbs(db) {
}


// ----------------------------------------------------------------------
bool calTileTimeCalibration::getNextID(uint32_t &ID) {
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
calTileTimeCalibration::calTileTimeCalibration(cdbAbs *db, string tag) : calAbs(db, tag) {
  if (0) cout << "calTileTimeCalibration created and registered with tag ->" << fTag << "<-"
  << endl;
}


// ----------------------------------------------------------------------
calTileTimeCalibration::~calTileTimeCalibration() {
  fMapConstants.clear();
  if (fVerbose > 0) cout << "this is the end of calTileTimeCalibration with tag ->" << fTag << "<-" << endl;
}


// ----------------------------------------------------------------------
void calTileTimeCalibration::calculate(string hash) {
  if (fVerbose > 0) cout << "calTileTimeCalibration::calculate() with "  << "fHash ->" << hash << "<-";
  fMapConstants.clear();
  string spl = fTagIOVPayloadMap[hash].fBLOB;
  
  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();
  
  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  if (fVerbose > 0) cout << " header: " << hex << header << dec;
  fRunNumber = blob2Int(getData(ibuffer));
  fTimestamp = blob2Int(getData(ibuffer));
  fNBINS = blob2Int(getData(ibuffer));
  for (int i = 0; i < fNBINS; i++) {
    fBinBoundaries.push_back(blob2Double(getData(ibuffer)));
  }
  while (ibuffer != buffer.end()) {
    constants ct;
    ct.id = blob2UnsignedInt(getData(ibuffer));
    for (int i = 0; i < 32; i++) {
      ct.dnl_corrected_time_fraction.push_back(blob2Double(getData(ibuffer)));
    }
    ct.timeAlignment_offset_ns = blob2Double(getData(ibuffer));
    for (int i = 0; i < 32; i++) {
      ct.timeWalk_correction_ns.push_back(blob2Double(getData(ibuffer)));
      ct.timeWalk_correction_energy.push_back(blob2Double(getData(ibuffer)));
    }
    fMapConstants.insert(make_pair(ct.id, ct));
  }
}


// ----------------------------------------------------------------------
string calTileTimeCalibration::makeBLOB() {
  stringstream s;
  unsigned int header(0xdeadface);
  s << dumpArray(uint2Blob(header));
  
  // -- format of fMapConstants
  // tileID => [dnl_corrected_time_fraction, timeAlignment_offset_ns, timeWalk_correction_ns, timeWalk_correction_energy]
  s << dumpArray(int2Blob(fRunNumber));
  s << dumpArray(int2Blob(fTimestamp));
  s << dumpArray(int2Blob(fNBINS));
  for (int i = 0; i < fNBINS; i++) {
    s << dumpArray(double2Blob(fBinBoundaries[i]));
  }
  for (auto it: fMapConstants) {
    s << dumpArray(uint2Blob(it.first));
    for (int i = 0; i < 32; i++) {
      s << dumpArray(double2Blob(it.second.dnl_corrected_time_fraction[i]));
    }
    s << dumpArray(double2Blob(it.second.timeAlignment_offset_ns));
    for (int i = 0; i < fNBINS; i++) {
      s << dumpArray(double2Blob(it.second.timeWalk_correction_ns[i]));
      s << dumpArray(double2Blob(it.second.timeWalk_correction_energy[i]));
    }
  }
  if (fVerbose > 0) cout << "calTileQuality::makeBLOB> made BLOB with " << fMapConstants.size() << " tiles" << endl;
  return s.str();
}


// ----------------------------------------------------------------------
string calTileTimeCalibration::makeBLOB(const std::map<unsigned int, std::vector<double>>&) {
  return makeBLOB();
}


// ----------------------------------------------------------------------
void calTileTimeCalibration::printBLOB(std::string blob, int verbosity) {
  cout << printBLOBString(blob, verbosity) << endl;
}

// ----------------------------------------------------------------------
string calTileTimeCalibration::printBLOBString(std::string blob, int verbosity) {
  stringstream ss;
  
  std::vector<char> buffer(blob.begin(), blob.end());
  std::vector<char>::iterator ibuffer = buffer.begin();
  
  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  ss << "calTileTimeCalibration::printBLOB(string)" << endl;
  ss << "   header: " << hex << header << dec << endl;
  int runNumber = blob2Int(getData(ibuffer));
  ss << "   run number: " << runNumber << endl;
  int timestamp = blob2Int(getData(ibuffer));
  ss << "   timestamp: " << timestamp << endl;
  int nbins = blob2Int(getData(ibuffer));
  ss << "   nbins: " << nbins << endl;
  for (int i = 0; i < nbins; i++) {
    double boundary = blob2Double(getData(ibuffer));
    ss << "   boundary: " << boundary << endl;
  }
  int cnt(0);
  while (ibuffer != buffer.end()) {
    uint32_t id = blob2UnsignedInt(getData(ibuffer));
    ss << "   id = " << id << endl;
    ss << "   dnl_corrected_time_fraction: ";
    for (int i = 0; i < 32; i++) {
      double dnl_corrected_time_fraction = blob2Double(getData(ibuffer));
      ss << dnl_corrected_time_fraction;
    }
    ss << endl;
    double timeAlignment_offset_ns = blob2Double(getData(ibuffer));
    ss << "   timeAlignment_offset_ns: " << timeAlignment_offset_ns << endl;
    ss << "   timeWalk_correction_ns: ";
    for (int i = 0; i < fNBINS; i++) {
      double timeWalk_correction_ns = blob2Double(getData(ibuffer));
      ss << timeWalk_correction_ns;
    }
    ss << endl;
    ss << "   timeWalk_correction_energy: ";
    for (int i = 0; i < fNBINS; i++) {
      double timeWalk_correction_energy = blob2Double(getData(ibuffer));
      ss << timeWalk_correction_energy;
    }
    ss << endl;
  }
}


// ----------------------------------------------------------------------
void calTileTimeCalibration::readJSON(string filename) {
  json j;
  ifstream INS(filename);
  j = json::parse(INS);
  
  string strRunNumber = j["Run_number"];
  replaceAll(strRunNumber, "run", "");
  fRunNumber = stoi(strRunNumber);
  fTimestamp = j["Timestamp"];
  
  fMapConstants.clear();
  
  for (auto& [key, value] : j.items()) {
    constants a;
    // -- check if value is an object (new format) or a number (old format)
    if (value.is_object()) {
      fMapConstants.insert(make_pair(a.id, a));
    }
    cout << "calTileTimeCalibration::readJSON> read " << fMapConstants.size() << " tiles" << endl;
    // -- set iterator over all constants to the start of the map
    fMapConstantsIt = fMapConstants.begin();
  }
}
  