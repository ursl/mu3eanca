#include "calPixelQualityV.hh"

#include "cdbUtil.hh"

#include <iostream>
#include <sstream>


using namespace std;

// ----------------------------------------------------------------------
calPixelQualityV::calPixelQualityV(cdbAbs *db) : calAbs(db) {
}


// ----------------------------------------------------------------------
bool calPixelQualityV::getNextID(uint32_t &ID) {
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
calPixelQualityV::calPixelQualityV(cdbAbs *db, string tag) : calAbs(db, tag) {
	cout << "calPixelQualityV created and registered with tag ->" << fTag << "<-" 
			 << endl;
}


// ----------------------------------------------------------------------
calPixelQualityV::~calPixelQualityV() {
  cout << "this is the end of calPixelQualityV with tag ->" << fTag << "<-" << endl;
}


// ----------------------------------------------------------------------
void calPixelQualityV::calculate(string hash) {
  cout << "calPixelQualityV::calculate() with "
       << "fHash ->" << hash << "<-"
       << endl;
  fMapConstants.clear();
  string spl = fTagIOVPayloadMap[hash].fBLOB;

  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();
  
  long unsigned int header = blob2UnsignedInt(getData(ibuffer)); 
  cout << "calPixelQualityV header: " << hex << header << dec << endl;

  while (ibuffer != buffer.end()) {
    constants a; 
    a.id = blob2UnsignedInt(getData(ibuffer));
    
    fMapConstants.insert(make_pair(a.id, a));
  }

  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();
}

