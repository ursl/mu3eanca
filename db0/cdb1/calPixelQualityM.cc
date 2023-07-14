#include "calPixelQualityM.hh"

#include "cdbUtil.hh"

#include <iostream>
#include <sstream>


using namespace std;

// ----------------------------------------------------------------------
calPixelQualityM::calPixelQualityM(cdbAbs *db) : calAbs(db) {
}


// ----------------------------------------------------------------------
bool calPixelQualityM::getNextID(uint32_t &ID) {
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
calPixelQualityM::calPixelQualityM(cdbAbs *db, string tag) : calAbs(db, tag) {
	cout << "calPixelQualityM created and registered with tag ->" << fTag << "<-" 
			 << endl;
}


// ----------------------------------------------------------------------
calPixelQualityM::~calPixelQualityM() {
  for (auto it: fMapConstants) it.second.mpixel.clear();    
  fMapConstants.clear();
  cout << "this is the end of calPixelQualityM with tag ->" << fTag << "<-" << endl;
}


// ----------------------------------------------------------------------
void calPixelQualityM::calculate(string hash) {
  cout << "calPixelQualityM::calculate() with "
       << "fHash ->" << hash << "<-"
       << endl;
  fMapConstants.clear();
  string spl = fTagIOVPayloadMap[hash].fBLOB;

  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();
  
  long unsigned int header = blob2UnsignedInt(getData(ibuffer)); 
  cout << "calPixelQualityM header: " << hex << header << dec << endl;
  
  int npix(0);
  while (ibuffer != buffer.end()) {
    constants a; 
    a.id = blob2UnsignedInt(getData(ibuffer));
    // -- get number of pixel entries
    npix = blob2Int(getData(ibuffer));
    
    for (unsigned int i = 0; i < npix; ++i) {
      int icol            = blob2Int(getData(ibuffer));
      int irow            = blob2Int(getData(ibuffer));
      unsigned int iqual = blob2UnsignedInt(getData(ibuffer));
      int idx = icol*250 + irow;
      a.mpixel.insert(make_pair(idx, static_cast<char>(iqual)));
    }
    // cout << "inserting " << a.id << " with size = " << sizeof(a) << endl;
    fMapConstants.insert(make_pair(a.id, a));
  }

  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();
}


// ----------------------------------------------------------------------
int calPixelQualityM::getStatus(unsigned int chipid, int icol, int irow) {
  return static_cast<int>(fMapConstants[chipid].mpixel[icol*250+256]);
}
