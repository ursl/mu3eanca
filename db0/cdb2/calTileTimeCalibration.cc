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
  // -- read metadata
  fRunNumber = blob2Int(getData(ibuffer));
  fTimestamp = blob2Int(getData(ibuffer));
  // -- read constants
  while (ibuffer != buffer.end()) {
    constants ct;
    // -- read id
    ct.id = blob2UnsignedInt(getData(ibuffer));
    for (int i = 0; i < 32; i++) {
      ct.dnl_corrected_time_fraction.push_back(blob2Double(getData(ibuffer)));
    }
    // -- read time alignment offset
    ct.timeAlignment_offset_ns = blob2Double(getData(ibuffer));
    // -- read timewalk correction NBINS
    int nbins = blob2Int(getData(ibuffer));
    for (int i = 0; i < nbins; i++) {
      ct.timeWalk_correction_ns.push_back(blob2Double(getData(ibuffer)));
    }
    for (int i = 0; i < nbins; i++) {
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
  for (auto it: fMapConstants) {
    // -- dump id
    s << dumpArray(uint2Blob(it.first));
    // -- dump dnl_corrected_time_fraction
    for (int i = 0; i < 32; i++) {
      s << dumpArray(double2Blob(it.second.dnl_corrected_time_fraction[i]));
    }
    // -- dump time alignment offset  
    s << dumpArray(double2Blob(it.second.timeAlignment_offset_ns));
    // -- dump timewalk correction NBINS
    int nbins = it.second.timeWalk_correction_ns.size();
    s << dumpArray(int2Blob(nbins));
    for (int i = 0; i < nbins; i++) {
      s << dumpArray(double2Blob(it.second.timeWalk_correction_ns[i]));
    }
    for (int i = 0; i < nbins; i++) {
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
    int nbins = blob2Int(getData(ibuffer));
    for (int i = 0; i < nbins; i++) {
      double timeWalk_correction_ns = blob2Double(getData(ibuffer));
      ss << timeWalk_correction_ns;
    }
    ss << endl;
    ss << "   timeWalk_correction_energy: ";
    for (int i = 0; i < nbins; i++) {
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

  if (j.contains("run_number")) {
    string strRunNumber = j["run_number"];
    replaceAll(strRunNumber, "run", "");
    fRunNumber = stoi(strRunNumber);
  }
  if (j.contains("timestamp")) {
    fTimestamp = j["timestamp"];
  }

  fMapConstants.clear();

  if (!j.contains("modules") || !j["modules"].is_object()) {
    cerr << "calTileTimeCalibration::readJSON> missing 'modules' object\n";
    return;
  }
  const json& modules = j["modules"];
  if (!modules.contains("dnl_correction") || !modules["dnl_correction"].is_object()) {
    cerr << "calTileTimeCalibration::readJSON> missing 'modules.dnl_correction'\n";
    return;
  }

  // -- read DNL correction
  const json& dnl = modules["dnl_correction"];
  if (!dnl.contains("channels") || !dnl["channels"].is_object()) {
    cerr << "calTileTimeCalibration::readJSON> missing 'modules.dnl_correction.channels'\n";
    return;
  }

  for (auto& [chanKey, chanVal] : dnl["channels"].items()) {
    if (!chanVal.contains("corrected_time_fraction")
        || !chanVal["corrected_time_fraction"].is_array()) {
      cerr << "calTileTimeCalibration::readJSON> channel " << chanKey
           << ": missing corrected_time_fraction array\n";
      continue;
    }
    constants a;
    a.id = stoul(chanKey);
    a.dnl_corrected_time_fraction =
      chanVal["corrected_time_fraction"].get<std::vector<double>>();
    fMapConstants.insert(make_pair(a.id, a));
  }

  // -- read time alignment (parallel to dnl_correction under modules)
  if (!modules.contains("time_alignment") || !modules["time_alignment"].is_object()) {
    cerr << "calTileTimeCalibration::readJSON> missing 'modules.time_alignment', skipping\n";
  } else {
    const json& time_alignment = modules["time_alignment"];
    if (!time_alignment.contains("channels") || !time_alignment["channels"].is_object()) {
      cerr << "calTileTimeCalibration::readJSON> missing 'modules.time_alignment.channels', skipping\n";
    } else {
      for (auto& [chanKey, chanVal] : time_alignment["channels"].items()) {
        if (!chanVal.contains("offset_ns") || !chanVal["offset_ns"].is_number()) {
          cerr << "calTileTimeCalibration::readJSON> channel " << chanKey
               << ": missing or invalid offset_ns\n";
          continue;
        }
        uint32_t id = stoul(chanKey);
        bool isValid = chanVal.contains("is_valid") && chanVal["is_valid"].get<bool>();
        double offset_ns = chanVal["offset_ns"].get<double>();

        constants& a = fMapConstants[id];
        a.id = id;
        a.timeAlignment_offset_ns = isValid ? offset_ns : -999.;
      }
    }
  }

  // -- read timewalk correction (parallel to dnl_correction / time_alignment under modules)
  if (!modules.contains("timewalk_correction") || !modules["timewalk_correction"].is_object()) {
    cerr << "calTileTimeCalibration::readJSON> missing 'modules.timewalk_correction', skipping\n";
  } else {
    const json& timewalk = modules["timewalk_correction"];
    if (!timewalk.contains("channels") || !timewalk["channels"].is_object()) {
      cerr << "calTileTimeCalibration::readJSON> missing 'modules.timewalk_correction.channels', skipping\n";
    } else {
      for (auto& [chanKey, chanVal] : timewalk["channels"].items()) {
        if (!chanVal.contains("correction_ns") || !chanVal["correction_ns"].is_array()) {
          cerr << "calTileTimeCalibration::readJSON> channel " << chanKey
               << ": missing correction_ns array\n";
          continue;
        }
        if (!chanVal.contains("energy") || !chanVal["energy"].is_array()) {
          cerr << "calTileTimeCalibration::readJSON> channel " << chanKey
               << ": missing energy array\n";
          continue;
        }
        std::vector<double> correction_ns =
          chanVal["correction_ns"].get<std::vector<double>>();
        std::vector<double> energy =
          chanVal["energy"].get<std::vector<double>>();
        if (correction_ns.size() != energy.size()) {
          cerr << "calTileTimeCalibration::readJSON> channel " << chanKey
               << ": correction_ns and energy size mismatch ("
               << correction_ns.size() << " vs " << energy.size() << ")\n";
          continue;
        }
        uint32_t id = stoul(chanKey);
        constants& a = fMapConstants[id];
        a.id = id;
        a.timeWalk_correction_ns = std::move(correction_ns);
        a.timeWalk_correction_energy = std::move(energy);
      }
    }
  }

  cout << "calTileTimeCalibration::readJSON> read " << fMapConstants.size()
       << " channels" << endl;

  // -- print stuff
  cout << "calTileTimeCalibration::readJSON> run number: " << fRunNumber << endl;
  cout << "calTileTimeCalibration::readJSON> timestamp: " << fTimestamp << endl;
  for (auto it: fMapConstants) {
    cout << "calTileTimeCalibration::readJSON> id: " << it.first << endl;
    for (unsigned int i = 0; i < it.second.dnl_corrected_time_fraction.size(); i++) {
      cout << "calTileTimeCalibration::readJSON> dnl_corrected_time_fraction[" << i << "]: " << it.second.dnl_corrected_time_fraction[i] << endl;
    }
    cout << "calTileTimeCalibration::readJSON> timeAlignment_offset_ns: " << it.second.timeAlignment_offset_ns << endl;
    for (unsigned int i = 0; i < it.second.timeWalk_correction_ns.size(); i++) {
      cout << "calTileTimeCalibration::readJSON> timeWalk[" << i << "] energy="
           << it.second.timeWalk_correction_energy[i]
           << " correction_ns=" << it.second.timeWalk_correction_ns[i] << endl;
    }
  }
  fMapConstantsIt = fMapConstants.begin();
}
  