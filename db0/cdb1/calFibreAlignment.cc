#include "calFibreAlignment.hh"

#include "cdbUtil.hh"

#include <iostream>
#include <sstream>


using namespace std;

// ----------------------------------------------------------------------
calFibreAlignment::calFibreAlignment(cdbAbs *db) : calAbs(db) {
}


// ----------------------------------------------------------------------
bool calFibreAlignment::getNextID(uint32_t &ID) {
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
calFibreAlignment::calFibreAlignment(cdbAbs *db, string tag) : calAbs(db, tag) {
	cout << "calFibreAlignment created and registered with tag ->" << fTag << "<-" 
			 << endl;
}


// ----------------------------------------------------------------------
calFibreAlignment::~calFibreAlignment() {
  cout << "this is the end of calFibreAlignment with tag ->" << fTag << "<-" << endl;
}


// ----------------------------------------------------------------------
void calFibreAlignment::calculate(string hash) {
  cout << "calFibreAlignment::calculate() with "
       << "fHash ->" << hash << "<-"
       << endl;
  fMapConstants.clear();
  string spl = fTagIOVPayloadMap[hash].fBLOB;

  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();
  
  long unsigned int header = blob2UnsignedInt(getData(ibuffer)); 
  cout << "calFibreAlignment header: " << hex << header << dec << endl;

  while (ibuffer != buffer.end()) {
    constants a; 
    a.id = blob2UnsignedInt(getData(ibuffer));
    a.cx = blob2Double(getData(ibuffer));
    a.cy = blob2Double(getData(ibuffer));
    a.cz = blob2Double(getData(ibuffer));
    a.fx = blob2Double(getData(ibuffer));
    a.fy = blob2Double(getData(ibuffer));
    a.fz = blob2Double(getData(ibuffer));
    a.round = static_cast<bool>(blob2UnsignedInt(getData(ibuffer)));
    a.square = static_cast<bool>(blob2UnsignedInt(getData(ibuffer)));
    a.diameter = blob2Double(getData(ibuffer));

    fMapConstants.insert(make_pair(a.id, a));
  }

  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();
}

