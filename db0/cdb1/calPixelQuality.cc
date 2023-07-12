#include "calPixelQuality.hh"

#include "cdbUtil.hh"

#include <iostream>
#include <sstream>


using namespace std;

// ----------------------------------------------------------------------
calPixelQuality::calPixelQuality(cdbAbs *db) : calAbs(db) {
}


// ----------------------------------------------------------------------
bool calPixelQuality::getNextID(uint32_t &ID) {
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
calPixelQuality::calPixelQuality(cdbAbs *db, string tag) : calAbs(db, tag) {
	cout << "calPixelQuality created and registered with tag ->" << fTag << "<-" 
			 << endl;
}


// ----------------------------------------------------------------------
calPixelQuality::~calPixelQuality() {
  cout << "this is the end of calPixelQuality with tag ->" << fTag << "<-" << endl;
}


// ----------------------------------------------------------------------
void calPixelQuality::calculate(string hash) {
  cout << "calPixelQuality::calculate() with "
       << "fHash ->" << hash << "<-"
       << endl;
  fMapConstants.clear();
  string spl = fTagIOVPayloadMap[hash].fBLOB;

  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();
  
  long unsigned int header = blob2UnsignedInt(getData(ibuffer)); 
  cout << "calPixelQuality header: " << hex << header << dec << endl;

  int npix(0);
  while (ibuffer != buffer.end()) {
    constants a; 
    a.id = blob2UnsignedInt(getData(ibuffer));
    // -- get number of pixel entries
    npix = blob2Int(getData(ibuffer));
    // -- fill matrix with zero
    for (unsigned int ix = 0; ix < 256; ++ix) {
      for (unsigned int iy = 0; iy < 250; ++iy) {
        a.matrix[ix][iy] = static_cast<char>(0);
      }
    }
    for (unsigned int i = 0; i < npix; ++i) {
      int icol           = blob2Int(getData(ibuffer));
      int irow           = blob2Int(getData(ibuffer));
      unsigned int iqual = blob2UnsignedInt(getData(ibuffer));
      a.matrix[icol][irow] = static_cast<char>(iqual);
    }
    cout << "inserting " << a.id << " with size = " << sizeof(a) << endl;
    fMapConstants.insert(make_pair(a.id, a));
  }

  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();
}


// ----------------------------------------------------------------------
char calPixelQuality::getStatus(unsigned int chipid, int icol, int irow) {
  return fMapConstants[chipid].matrix[icol][irow];
}


// ----------------------------------------------------------------------
void calPixelQuality::printPixelQuality(unsigned int chipid, int minimumStatus) {
  for (unsigned int ix = 0; ix < 256; ++ix) {
    for (unsigned int iy = 0; iy < 250; ++iy) {
      if (fMapConstants[chipid].matrix[ix][iy] > minimumStatus) {
        cout << "chipID = " << chipid
             << " x/y = " << ix << "/" << iy
             << " status = " << static_cast<unsigned int>(fMapConstants[chipid].matrix[ix][iy])
             << endl;
      }
    }
  }
}
