#include "calFibresAlignment.hh"

#include "cdbUtil.hh"

#include <iostream>
#include <sstream>


using namespace std;

// ----------------------------------------------------------------------
calFibresAlignment::calFibresAlignment(cdbAbs *db) : calAbs(db) {
}


// ----------------------------------------------------------------------
bool calFibresAlignment::getNextID(uint32_t &ID) {
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
calFibresAlignment::calFibresAlignment(cdbAbs *db, string tag) : calAbs(db, tag) {
	cout << "calFibresAlignment created and registered with tag ->" << fTag << "<-" 
			 << endl;
}


// ----------------------------------------------------------------------
calFibresAlignment::~calFibresAlignment() {
  cout << "this is the end of calFibresAlignment with tag ->" << fTag << "<-" << endl;
}


// ----------------------------------------------------------------------
void calFibresAlignment::calculate(string hash) {
  cout << "calFibresAlignment::calculate() with "
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
    a.cx = blob2Double(getData(ibuffer));
    a.cy = blob2Double(getData(ibuffer));
    a.cz = blob2Double(getData(ibuffer));
    a.fx = blob2Double(getData(ibuffer));
    a.fy = blob2Double(getData(ibuffer));
    a.fz = blob2Double(getData(ibuffer));
    a.round = blob2Bool(getData(ibuffer));
    a.square = blob2Bool(getData(ibuffer));
    a.diameter = blob2Double(getData(ibuffer));

    fMapConstants.insert(make_pair(a.id, a));
  }

  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();
}

