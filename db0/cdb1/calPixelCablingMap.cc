#include "calPixelCablingMap.hh"

#include "cdbUtil.hh"

#include <iostream>
#include <sstream>

#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <bsoncxx/stdx/string_view.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>
#include <mongocxx/stdx.hpp>


using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::basic::sub_array;
using bsoncxx::builder::basic::sub_document;
using bsoncxx::builder::basic::make_document;
using bsoncxx::document::element;

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
  
  long unsigned int header = blob2UnsignedInt(getData(ibuffer)); 
  cout << "calPixelCablingMap header: " << hex << header << dec << endl;

  unsigned int sensor(0), online(0);
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
  if (0xdeadface != header) {
    cout << "XXXXX ERRROR in calPixelCablingMap::printBLOB> header is wrong. Something is really messed up!" << endl;
  }

  string summary("calPixelCablingMap "); 
  int nchips(0);
  while (ibuffer != buffer.end()) {
    ++nchips;
    // -- offline chipID
    unsigned int chipID = blob2UnsignedInt(getData(ibuffer));
    // -- online 
    unsigned int online = blob2UnsignedInt(getData(ibuffer));
    if (verbosity > 0) cout << "      chipID offline/online = "
                            << chipID << "/" <<  online
                            << endl;
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

  unsigned int sensor(0), online(0);
  while (ibuffer != buffer.end()) {
    sensor = blob2UnsignedInt(getData(ibuffer));
    online = blob2UnsignedInt(getData(ibuffer));
    vmap.insert(make_pair(sensor, online));
  }

  return vmap;
}


// ----------------------------------------------------------------------
string calPixelCablingMap::makeBLOB() {
  stringstream s;
  long unsigned int header(0xdeadface);
  s << dumpArray(uint2Blob(header));
  
  for (auto it: fMapConstants) {
    s << dumpArray(uint2Blob(it.first));
    s << dumpArray(uint2Blob(it.second));
  }
  return s.str();

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

  string sline;
  while (getline(INS, sline)) {
    spl += sline; 
  }
  INS.close();

  // -- iterate over the elements in a bson document
  bsoncxx::document::value doc = bsoncxx::from_json(spl.c_str());
  for (element ele : doc.view()) {
    std::string_view index{ele.key().to_string()};
    unsigned int online, sensor;
    cout << "index = " << index
         << " online = " << ele["online"].get_string().value.to_string() //.value
         << " sensor = " << ele["sensor"].get_string().value.to_string() //.value 
         << endl;
    online = ::stoi(ele["online"].get_string().value.to_string());
    sensor = ::stoi(ele["sensor"].get_string().value.to_string());
    cout << " unsigned int version: " << online << " -> " << sensor << endl;
    fMapConstants.insert(make_pair(sensor, online));
  }
  return spl;
}
