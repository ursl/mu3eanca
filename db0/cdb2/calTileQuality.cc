#include "calTileQuality.hh"

#include "cdbUtil.hh"

#include <iostream>
#include <sstream>
#include <cctype>
#include <algorithm>

#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::ordered_json;

// ----------------------------------------------------------------------
calTileQuality::calTileQuality(cdbAbs *db) : calAbs(db) {
}


// ----------------------------------------------------------------------
bool calTileQuality::getNextID(uint32_t &ID) {
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
calTileQuality::calTileQuality(cdbAbs *db, string tag) : calAbs(db, tag) {
  if (0) cout << "calTileQuality created and registered with tag ->" << fTag << "<-"
       << endl;
}


// ----------------------------------------------------------------------
calTileQuality::~calTileQuality() {
  fMapConstants.clear();
  cout << "this is the end of calTileQuality with tag ->" << fTag << "<-" << endl;
}


// ----------------------------------------------------------------------
calTileQuality::Status calTileQuality::getChannelQuality(uint32_t id) {
  if (fMapConstants.find(id) == fMapConstants.end()) {
    return ChannelNotFound;
  }
  return static_cast<Status>(fMapConstants[id].quality);
}


// ----------------------------------------------------------------------
void calTileQuality::calculate(string hash) {
  cout << "calTileQuality::calculate() with "  << "fHash ->" << hash << "<-";
  fMapConstants.clear();
  string spl = fTagIOVPayloadMap[hash].fBLOB;

  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  cout << " header: " << hex << header << dec;
  int npix(0);
  while (ibuffer != buffer.end()) {
    constants cq;
    cq.id = blob2UnsignedInt(getData(ibuffer));
    cq.quality = blob2Int(getData(ibuffer));
    fMapConstants.insert(make_pair(cq.id, cq));
  }
  cout << " inserted " << fMapConstants.size() << " constants" << endl;

  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();
}


// ----------------------------------------------------------------------
string calTileQuality::makeBLOB() {
  stringstream s;
  unsigned int header(0xdeadface);
  s << dumpArray(uint2Blob(header));

  // -- format of fMapConstants
  // chipID => [quality]
  for (auto it: fMapConstants) {
    s << dumpArray(uint2Blob(it.first));
    s << dumpArray(int2Blob(it.second.quality));
  }
  cout << "calTileQuality::makeBLOB> made BLOB with " << fMapConstants.size() << " tiles" << endl;
  return s.str();
}

// ----------------------------------------------------------------------
void calTileQuality::printBLOB(std::string blob, int verbosity) {

  std::vector<char> buffer(blob.begin(), blob.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  cout << "calTileQuality::printBLOB(string)" << endl;
  cout << "   header: " << hex << header << dec << endl;

  int cnt(0);
  while (ibuffer != buffer.end()) {
    uint32_t id = blob2UnsignedInt(getData(ibuffer));
    Status status = static_cast<Status>(blob2Int(getData(ibuffer)));
    if (verbosity > 0) {
      if (status == Good) {
        ++cnt;
        cout << "   id = " << id
             << " quality = " << status
             << endl;
      }
    } else if (verbosity < 0) {
      if (status != Good) {
        ++cnt;
        cout << "   id = " << id
             << " quality = " << status
             << endl;
      }
    } else {
      ++cnt;
      cout << "   id = " << id
           << " quality = " << status
           << endl;
    }
  }
  string tileType("all");
  if (verbosity < 0) {
    tileType = "not good";
  } else if (verbosity > 0) {
    tileType = "good";
  }
  cout << "calTileQuality::printBLOB(...) printed status for " << cnt  << " tiles " << "(" << tileType << ")" << endl;
}


// ----------------------------------------------------------------------
void calTileQuality::writeJSON(string filename) {
  string spl("");
  ofstream OUT(filename);
  json j;
  j["Run_number"] = fRunNumber;
  for (auto it: fMapConstants) {
    uint32_t tileID = it.first;
    int quality = it.second.quality;
    j[to_string(tileID)] = {
      {"tileID", tileID},
      {"Good", (quality == calTileQuality::Good) ? 1 : 0},
      {"Dead", (quality == calTileQuality::Dead) ? 1 : 0},
      {"Noisy", (quality == calTileQuality::Noisy) ? 1 : 0}
    };
  }
  OUT << j.dump(2);
  OUT.close();
}

// ----------------------------------------------------------------------
void calTileQuality::readJSON(string filename) {
  json j;
  ifstream INS(filename);
  j = json::parse(INS);
  fMapConstants.clear();
  for (auto& [key, value] : j.items()) {
    constants a;
    // -- check if value is an object (new format) or a number (old format)
    if (value.is_object()) {
      // -- new format: {"tileID": 1, "Good": 0, "Dead": 0, "Noisy": 0}
      a.id = value["tileID"];
      int iGood = value["Good"];
      int iDead = value["Dead"];
      int iNoisy = value["Noisy"];
      if (1 == iGood) {
        a.quality = Status::Good;
      } else if (1 == iNoisy) {
        a.quality = Status::Noisy;
      } else if (1 == iDead) {
        a.quality = Status::Dead;
      } else {
        a.quality = Status::Unset;
      }
      int cntFlags(0); 
      if (0 != iGood) ++cntFlags;  
      if (0 != iNoisy) ++cntFlags;  
      if (0 != iDead) ++cntFlags;  
      if (cntFlags > 1) {
        a.quality = Status::DeclaredBad;
      }
    } else if (value.is_number()) {
      if (key == "Run_number") {
        fRunNumber = value.get<int>();
        continue;
      }
      // -- old format: just a number
      // -- key is already a string from json.items()
      if (key.empty()) {
        continue;  // Skip empty keys silently
      }
      // -- Check if key is numeric before converting (skip metadata keys like "Run_number")
      bool isNumeric = !key.empty() && all_of(key.begin(), key.end(), [](char c) { return isdigit(c); });
      if (!isNumeric) {
        continue;  // Skip non-numeric keys silently (they're likely metadata)
      }
      try {
        a.id = stoi(key);
      } catch (const std::invalid_argument& e) {
        continue;  // Skip invalid keys silently
      } catch (const std::out_of_range& e) {
        cerr << "calTileQuality::readJSON> Key out of range: " << key << endl;
        continue;
      }
      a.quality = static_cast<Status>(value.get<int>());
    } else {
      // -- unexpected format
      cerr << "calTileQuality::readJSON> Unexpected format for key: " << key << endl;
      continue;
    }
    fMapConstants.insert(make_pair(a.id, a));
  }
  cout << "calTileQuality::readJSON> read " << fMapConstants.size() << " tiles" << endl;
  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();
}
