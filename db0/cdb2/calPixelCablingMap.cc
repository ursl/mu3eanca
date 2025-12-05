#include "calPixelCablingMap.hh"

#include "cdbUtil.hh"

#include <iostream>
#include <sstream>

using namespace std;

// ----------------------------------------------------------------------
calPixelCablingMap::calPixelCablingMap(cdbAbs *db) : calAbs(db) {
}


// ----------------------------------------------------------------------
bool calPixelCablingMap::getNextID(unsigned int &ID) {
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
calPixelCablingMap::calPixelCablingMap(cdbAbs *db, string tag) : calAbs(db, tag) {
  cout << "calPixelCablingMap created and registered with tag ->" << fTag << "<-"
       << endl;
}


// ----------------------------------------------------------------------
calPixelCablingMap::~calPixelCablingMap() {
  fMapConstants.clear();
  cout << "this is the end of calPixelCablingMap with tag ->" << fTag << "<-" << endl;
}


// ----------------------------------------------------------------------
void calPixelCablingMap::calculate(string hash) {
  cout << "calPixelCablingMap::calculate() with "
       << "fHash ->" << hash << "<-"
       << endl;
  fMapConstants.clear();
  string spl = fTagIOVPayloadMap[hash].fBLOB;

  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  cout << "calPixelCablingMap header: " << hex << header << dec << endl;

  constants a;
  while (ibuffer != buffer.end()) {
    a.sensor = blob2UnsignedInt(getData(ibuffer));
    a.offsetA = blob2UnsignedInt(getData(ibuffer));
    a.offsetB = blob2UnsignedInt(getData(ibuffer));
    a.offsetC = blob2UnsignedInt(getData(ibuffer));
    a.offsetM = blob2UnsignedInt(getData(ibuffer));
    fMapConstants.insert(make_pair(a.sensor, a));
  }

  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();
}


// ----------------------------------------------------------------------
void calPixelCablingMap::printBLOB(string sblob, int verbosity) {
  cout << printBLOBString(sblob, verbosity) << endl;
}

// ----------------------------------------------------------------------
string calPixelCablingMap::printBLOBString(string sblob, int verbosity) {
  stringstream ss;

  vector<char> buffer(sblob.begin(), sblob.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  ss << "calPixelCablingMap::printBLOB(string," << verbosity << ")" << endl;
  ss << "   header: " << hex << header << dec << endl;
  if (0xdeadface != header) {
    ss << "XXXXX ERRROR in calPixelCablingMap::printBLOB> header is wrong. Something is really messed up!" << endl;
  }

  string summary("calPixelCablingMap ");
  int nsensors(0);
  while (ibuffer != buffer.end()) {
    ++nsensors;
    // -- offline chipID
    unsigned int sensor = blob2UnsignedInt(getData(ibuffer));
    // -- offsetA
    unsigned int offsetA = blob2UnsignedInt(getData(ibuffer));
    // -- offsetB
    unsigned int offsetB = blob2UnsignedInt(getData(ibuffer));
    // -- offsetC
    unsigned int offsetC = blob2UnsignedInt(getData(ibuffer));
    // -- offsetM
    unsigned int offsetM = blob2UnsignedInt(getData(ibuffer));
    if (verbosity > 0) ss << "   sensor offsetA/offsetB/offsetC/offsetM = "
                          << sensor << "/" <<  offsetA << "/" << offsetB << "/" << offsetC << "/" << offsetM
                          << endl;
  }
  if (0 == verbosity) {
    ss << summary << " with " << nsensors << " sensors" << endl;
  }
  return ss.str();
}


// ----------------------------------------------------------------------
string calPixelCablingMap::makeBLOB() {
  stringstream s;
  unsigned int header(0xdeadface);
  s << dumpArray(uint2Blob(header));

  for (auto it: fMapConstants) {
    constants a = it.second;
    s << dumpArray(uint2Blob(a.sensor));
    s << dumpArray(uint2Blob(a.offsetA));
    s << dumpArray(uint2Blob(a.offsetB));
    s << dumpArray(uint2Blob(a.offsetC));
    s << dumpArray(uint2Blob(a.offsetM));
  }
  return s.str();

}

// ----------------------------------------------------------------------
string calPixelCablingMap::readJson(string filename) {
  string spl("");
  ifstream INS(filename);
  if (!INS.is_open()) {
    return "calPixelCablingMap::readJson> Error, file " + filename + " not found";
  }

  string sline;
  while (getline(INS, sline)) {
    spl += sline;
  }
  INS.close();

  // FIXME migrate to NON-BSONCXX code
  // -- iterate over the elements in a bson document
  // bsoncxx::document::value doc = bsoncxx::from_json(spl.c_str());
  // for (element ele : doc.view()) {
  //   std::string_view index{ele.key().to_string()};
  //   unsigned int online, sensor;
  //   // cout << "index = " << index
  //   //      << " online = " << ele["online"].get_string().value.to_string() //.value
  //   //      << " sensor = " << ele["sensor"].get_string().value.to_string() //.value
  //   //      << endl;
  //   online = ::stoi(ele["online"].get_string().value.to_string());
  //   sensor = ::stoi(ele["sensor"].get_string().value.to_string());
  //   fMapConstants.insert(make_pair(sensor, online));
  // }
  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();

  return spl;
}
