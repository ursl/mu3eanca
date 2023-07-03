#include "calMppcAlignment.hh"

#include "cdbUtil.hh"

#include <iostream>
#include <sstream>


using namespace std;

// ----------------------------------------------------------------------
calMppcAlignment::calMppcAlignment(cdbAbs *db) : calAbs(db) {
}


// ----------------------------------------------------------------------
bool calMppcAlignment::getNextID(uint32_t &ID) {
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
calMppcAlignment::calMppcAlignment(cdbAbs *db, string tag) : calAbs(db, tag) {
	cout << "calMppcAlignment created and registered with tag ->" << fTag << "<-" 
			 << endl;
}


// ----------------------------------------------------------------------
calMppcAlignment::~calMppcAlignment() {
  cout << "this is the end of calMppcAlignment with tag ->" << fTag << "<-" << endl;
}


// ----------------------------------------------------------------------
void calMppcAlignment::calculate(string hash) {
  cout << "calMppcAlignment::calculate() with "
       << "fHash ->" << hash << "<-"
       << endl;
  fMapConstants.clear();
  string spl = fTagIOVPayloadMap[hash].fBLOB;

  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();
  
  long unsigned int header = blob2UnsignedInt(getData(ibuffer)); 
  cout << "calMppcAlignment header: " << hex << header << dec << endl;

  while (ibuffer != buffer.end()) {
    constants a; 
    a.mppc = blob2UnsignedInt(getData(ibuffer));
    a.vx = blob2Double(getData(ibuffer));
    a.vy = blob2Double(getData(ibuffer));
    a.vz = blob2Double(getData(ibuffer));
    a.colx = blob2Double(getData(ibuffer));
    a.coly = blob2Double(getData(ibuffer));
    a.colz = blob2Double(getData(ibuffer));
    a.ncol = blob2Int(getData(ibuffer));

    cout << "mppc = " << a.mppc
         << " v = " << a.vx << "/" << a.vy << "/" << a.vz
         << " col = " << a.colx << "/" << a.coly << "/" << a.colz
         << endl;
    
    fMapConstants.insert(make_pair(a.mppc, a));
  }

  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();
}

