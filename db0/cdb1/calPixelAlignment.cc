#include "calPixelAlignment.hh"

#include "cdbUtil.hh"

#include <iostream>
#include <sstream>


using namespace std;

// ----------------------------------------------------------------------
calPixelAlignment::calPixelAlignment(cdbAbs *db) : calAbs(db) {
}


// ----------------------------------------------------------------------
bool calPixelAlignment::getNextID(uint32_t &ID) {
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
calPixelAlignment::calPixelAlignment(cdbAbs *db, string tag) : calAbs(db, tag) {
	cout << "calPixelAlignment created and registered with tag ->" << fTag << "<-" 
			 << endl;
}


// ----------------------------------------------------------------------
calPixelAlignment::~calPixelAlignment() {
  cout << "this is the end of calPixelAlignment with tag ->" << fTag << "<-" << endl;
}


// ----------------------------------------------------------------------
void calPixelAlignment::calculate(string hash) {
  cout << "calPixelAlignment::calculate() with "
       << "fHash ->" << hash << "<-"
       << endl;
  fMapConstants.clear();
  string spl = fTagIOVPayloadMap[hash].fBLOB;

  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();
  
  long unsigned int header = blob2UnsignedInt(getData(ibuffer)); 
  cout << "header: " << hex << header << dec << endl;

  while (ibuffer != buffer.end()) {
    constants a; 
    a.id = blob2UnsignedInt(getData(ibuffer));
    a.vx = blob2Double(getData(ibuffer));
    a.vy = blob2Double(getData(ibuffer));
    a.vz = blob2Double(getData(ibuffer));
    a.rowx = blob2Double(getData(ibuffer));
    a.rowy = blob2Double(getData(ibuffer));
    a.rowz = blob2Double(getData(ibuffer));
    a.colx = blob2Double(getData(ibuffer));
    a.coly = blob2Double(getData(ibuffer));
    a.colz = blob2Double(getData(ibuffer));
    a.nrow = blob2Int(getData(ibuffer));
    a.ncol = blob2Int(getData(ibuffer));
    a.width = blob2Double(getData(ibuffer));
    a.length = blob2Double(getData(ibuffer));
    a.thickness = blob2Double(getData(ibuffer));
    a.pixelSize = blob2Double(getData(ibuffer));

    fMapConstants.insert(make_pair(a.id, a));
  }

  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();
}

