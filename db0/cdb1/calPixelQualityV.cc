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
  fMapConstants.clear();
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
  
  int npix(0);
  while (ibuffer != buffer.end()) {
    constants a; 
    a.id = blob2UnsignedInt(getData(ibuffer));
    // -- get number of pixel entries
    npix = blob2Int(getData(ibuffer));
    a.vpixel.reserve(npix);
    
    for (unsigned int i = 0; i < npix; ++i) {
      pixel px;
      px.icol            = blob2Int(getData(ibuffer));
      px.irow            = blob2Int(getData(ibuffer));
      unsigned int iqual = blob2UnsignedInt(getData(ibuffer));
      px.iqual           = static_cast<char>(iqual);
      a.vpixel.push_back(px);
    }
    // cout << "inserting " << a.id << " with size = " << sizeof(a) << endl;
    fMapConstants.insert(make_pair(a.id, a));
  }

  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();
}

// ----------------------------------------------------------------------
int calPixelQualityV::getStatus(unsigned int chipid, int icol, int irow) {
  constants a = fMapConstants[chipid];
  for (auto it: a.vpixel) {
    if (it.icol == icol && it.irow == irow) return static_cast<int>(it.iqual);
  }
  return -1;
}
