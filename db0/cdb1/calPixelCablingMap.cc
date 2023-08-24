#include "calPixelCablingMap.hh"

#include "cdbUtil.hh"

#include <iostream>
#include <sstream>


using namespace std;

// ----------------------------------------------------------------------
calPixelCablingMap::calPixelCablingMap(cdbAbs *db) : calAbs(db) {
}


// ----------------------------------------------------------------------
bool calPixelCablingMap::getNextID(uint32_t &ID) {
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
  
  long unsigned int header = blob2UnsignedInt(getData(ibuffer)); 
  cout << "calPixelCablingMap header: " << hex << header << dec << endl;

  uint32_t sensor(0), online(0);
  while (ibuffer != buffer.end()) {
    sensor = blob2UnsignedInt(getData(ibuffer));
    online = blob2UnsignedInt(getData(ibuffer));
    fMapConstants.insert(make_pair(sensor, online));
  }

  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();
}


// ----------------------------------------------------------------------
void calPixelCablingMap::printBLOB(std::string sblob, int verbosity) {

  std::vector<char> buffer(sblob.begin(), sblob.end());
  std::vector<char>::iterator ibuffer = buffer.begin();
  
  long unsigned int header = blob2UnsignedInt(getData(ibuffer)); 
  cout << "calPixelCablingMap::printBLOB(string," << verbosity << ")" << endl;
  cout << "   header: " << hex << header << dec << endl;

  string summary("calPixelCablingMap "); 
  int nchips(0);
  while (ibuffer != buffer.end()) {
    ++nchips;
    // -- offline chipID
    unsigned int chipID = blob2UnsignedInt(getData(ibuffer));
    // -- online 
    unsigned int online = blob2UnsignedInt(getData(ibuffer));
    if (verbosity > 0) cout << "      chipID offline/online = " << chipID << "/" <<  online << endl;
  }
  if (0 == verbosity) {
    cout << summary << " with " << nchips << " chips" << endl;
  }
}


// ----------------------------------------------------------------------
map<unsigned int, vector<double> > calPixelCablingMap::decodeBLOB(string spl) {
  map<unsigned int, vector<double> > vmap;
  
  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();
  
  long unsigned int header = blob2UnsignedInt(getData(ibuffer)); 
  if (0xdeadface != header) {
    cout << "XXXXX ERRROR in calPixelCablingMap::decodeBLOB> header is wrong. Something is really messed up!" << endl;
  }

  uint32_t sensor(0), online(0);
  while (ibuffer != buffer.end()) {
    sensor = blob2UnsignedInt(getData(ibuffer));
    online = blob2UnsignedInt(getData(ibuffer));
    vmap.insert(make_pair(sensor, online));
  }

  return vmap;
}


// ----------------------------------------------------------------------
string calPixelCablingMap::makeBLOB(map<unsigned int, vector<double> > m) {
  stringstream s;
  long unsigned int header(0xdeadface);
  s << dumpArray(uint2Blob(header));

  // -- format of m
  // chipID => [online]
  for (auto it: m) {
    s << dumpArray(uint2Blob(it.first));
    s << dumpArray(uint2Blob(it.second[0]));
  }
  return s.str();
}


// ----------------------------------------------------------------------
string calPixelCablingMap::readJson(string filename) {
  string spl("");
  ifstream INS(filename); 
  if (!INS.is_open()) {
    return string("calPixelCablingMap::readJson> Error, file " + filename + " not found");  
  }

  string bigLine(""), sline;
  while (getline(INS, sline)) {
    bigLine += sline; 
  }
  INS.close();
  replaceAll(bigLine, "\n", "");
  cleanupString(bigLine);

  cout << "bigLine: " << bigLine << endl;
  
  return spl;
}
