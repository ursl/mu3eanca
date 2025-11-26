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
  cout << "calFibreQuality created and registered with tag ->" << fTag << "<-"
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
void calFibreQuality::calculate(string hash) {
  if (fVerbose > 0) cout << "calFibreQuality::calculate() with "
       << "fHash ->" << hash << "<-"
       << endl;
  fMapConstants.clear();
  string spl = fTagIOVPayloadMap[hash].fBLOB;

  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));

  int npix(0);
  while (ibuffer != buffer.end()) {
    constants cq;
    cq.id = blob2UnsignedInt(getData(ibuffer));
    cq.lock = blob2Int(getData(ibuffer));
    cq.hasData = blob2Int(getData(ibuffer));
    fMapConstants.insert(make_pair(cq.id, cq));
  }

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
    s << dumpArray(int2Blob(it.second.lock));
    s << dumpArray(int2Blob(it.second.hasData));
  }
  return s.str();
}

// ----------------------------------------------------------------------
void calFibreQuality::printBLOB(std::string blob, int verbosity) {

  std::vector<char> buffer(blob.begin(), blob.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  cout << "calFibreQuality::printBLOB(string)" << endl;
  cout << "   header: " << hex << header << dec << endl;

  int cnt(0);
  while (ibuffer != buffer.end()) {
    uint32_t id = blob2UnsignedInt(getData(ibuffer));
    int lock = blob2Int(getData(ibuffer));
    int hasData = blob2Int(getData(ibuffer));
    cout << "   id = " << id
         << " lock = " << lock
         << " hasData = " << hasData
         << endl;
    ++cnt;
  }
  cout << "calFibreQuality::printBLOB(...) printed status for " << cnt << " Asics" << endl;
}


// ----------------------------------------------------------------------
void calFibreQuality::writeCSV(string filename) {
  ofstream OUT(filename);
  if (!OUT.is_open()) {
    cout << "calFibreQuality::writeCSV> Error, cannot open file " + filename + " for writing" << endl;
    return;
  }
  
  // -- Write header line
  OUT << "run_nr";
  for (int asicID = 0; asicID <= 95; ++asicID) {
    OUT << ", lock_asic" << asicID;
  }
  for (int asicID = 0; asicID <= 95; ++asicID) {
    OUT << ", has_data_asic" << asicID;
  }
  OUT << endl;
  
  // -- Write data line
  OUT << fRunNumber;
  for (int asicID = 0; asicID <= 95; ++asicID) {
    int lock = 0;
    if (fMapConstants.find(asicID) != fMapConstants.end()) {
      lock = fMapConstants[asicID].lock;
    }
    OUT << ", " << lock;
  }
  for (int asicID = 0; asicID <= 95; ++asicID) {
    int hasData = 0;
    if (fMapConstants.find(asicID) != fMapConstants.end()) {
      hasData = fMapConstants[asicID].hasData;
    }
    OUT << ", " << hasData;
  }
  OUT << endl;
  
  OUT.close();
}

// ----------------------------------------------------------------------
void calFibreQuality::readCSV(string filename) {
  ifstream INS(filename);
  if (!INS.is_open()) {
    cout << "calFibreQuality::readCSV> Error, file " + filename + " not found" << endl;
    return;
  }
  
  fMapConstants.clear();
  
  // -- Read header line
  string headerLine;
  if (!getline(INS, headerLine)) {
    cout << "calFibreQuality::readCSV> Error, cannot read header line" << endl;
    return;
  }
  
  // -- Parse header to find column indices
  stringstream headerStream(headerLine);
  vector<string> headers;
  string header;
  while (getline(headerStream, header, ',')) {
    // Remove leading/trailing whitespace
    header.erase(0, header.find_first_not_of(" \t"));
    header.erase(header.find_last_not_of(" \t") + 1);
    headers.push_back(header);
  }
  
  // -- Build map of column name -> index
  map<string, int> columnMap;
  for (size_t i = 0; i < headers.size(); ++i) {
    columnMap[headers[i]] = i;
  }
  
  // -- Read data line
  string dataLine;
  if (!getline(INS, dataLine)) {
    cout << "calFibreQuality::readCSV> Error, cannot read data line" << endl;
    return;
  }
  
  // -- Parse data values
  stringstream dataStream(dataLine);
  vector<string> values;
  string value;
  while (getline(dataStream, value, ',')) {
    // Remove leading/trailing whitespace
    value.erase(0, value.find_first_not_of(" \t"));
    value.erase(value.find_last_not_of(" \t") + 1);
    values.push_back(value);
  }
  
  // -- Extract run number (if present)
  if (columnMap.find("run_nr") != columnMap.end()) {
    int colIdx = columnMap["run_nr"];
    if (colIdx < static_cast<int>(values.size())) {
      fRunNumber = stoi(values[colIdx]);
    }
  }
  
  // -- Process ASICs 0-95
  for (int asicID = 0; asicID <= 95; ++asicID) {
    constants cq;
    cq.id = asicID;
    
    // -- Extract lock value
    string lockCol = "lock_asic" + to_string(asicID);
    if (columnMap.find(lockCol) != columnMap.end()) {
      int colIdx = columnMap[lockCol];
      if (colIdx < static_cast<int>(values.size())) {
        cq.lock = stoi(values[colIdx]);
      } else {
        cq.lock = 0;
      }
    } else {
      cq.lock = 0;
    }
    
    // -- Extract has_data value
    string hasDataCol = "has_data_asic" + to_string(asicID);
    if (columnMap.find(hasDataCol) != columnMap.end()) {
      int colIdx = columnMap[hasDataCol];
      if (colIdx < static_cast<int>(values.size())) {
        cq.hasData = stoi(values[colIdx]);
      } else {
        cq.hasData = 0;
      }
    } else {
      cq.hasData = 0;
    }
    cout << "calFibreQuality::readCSV> added asicID " << asicID << " with lock " << cq.lock << " and hasData " << cq.hasData << endl;
    fMapConstants.insert(make_pair(cq.id, cq));

  }
  
  // -- Set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();
  
  INS.close();
}
