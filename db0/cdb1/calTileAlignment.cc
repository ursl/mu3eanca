#include "calTileAlignment.hh"

#include "cdbUtil.hh"

#include <iostream>
#include <sstream>


using namespace std;

// ----------------------------------------------------------------------
calTileAlignment::calTileAlignment(cdbAbs *db) : calAbs(db) {
}


// ----------------------------------------------------------------------
bool calTileAlignment::getNextID(uint32_t &ID) {
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
calTileAlignment::calTileAlignment(cdbAbs *db, string tag) : calAbs(db, tag) {
	cout << "calTileAlignment created and registered with tag ->" << fTag << "<-" 
			 << endl;
}


// ----------------------------------------------------------------------
calTileAlignment::~calTileAlignment() {
  cout << "this is the end of calTileAlignment with tag ->" << fTag << "<-" << endl;
}


// ----------------------------------------------------------------------
void calTileAlignment::calculate(string hash) {
  cout << "calTileAlignment::calculate() with "
       << "fHash ->" << hash << "<-"
       << endl;
  fMapConstants.clear();
  string spl = fTagIOVPayloadMap[hash].fBLOB;

  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();
  
  long unsigned int header = blob2UnsignedInt(getData(ibuffer)); 
  cout << "calTileAlignment header: " << hex << header << dec << endl;

  while (ibuffer != buffer.end()) {
    constants a; 
    a.sensor = blob2Int(getData(ibuffer));
    a.id = blob2UnsignedInt(getData(ibuffer));
    a.posx = blob2Double(getData(ibuffer));
    a.posy = blob2Double(getData(ibuffer));
    a.posz = blob2Double(getData(ibuffer));
    a.dirx = blob2Double(getData(ibuffer));
    a.diry = blob2Double(getData(ibuffer));
    a.dirz = blob2Double(getData(ibuffer));

    fMapConstants.insert(make_pair(a.id, a));
  }

  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();
}

