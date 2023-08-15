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
  fMapConstants.clear();
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
    // -- chipID
    a.id = blob2UnsignedInt(getData(ibuffer));
    // -- get number of pixel entries
    npix = blob2Int(getData(ibuffer));
    // -- initialize matrix with zero before filling specified pixels
    for (int ix = 0; ix < 256; ++ix) {
      for (int iy = 0; iy < 250; ++iy) {
        a.matrix[ix][iy] = static_cast<char>(0);
      }
    }
    for (int i = 0; i < npix; ++i) {
      int icol           = blob2Int(getData(ibuffer));
      int irow           = blob2Int(getData(ibuffer));
      unsigned int iqual = blob2UnsignedInt(getData(ibuffer));
      a.matrix[icol][irow] = static_cast<char>(iqual);
    }
    // cout << "inserting " << a.id << " with size = " << sizeof(a) << endl;
    fMapConstants.insert(make_pair(a.id, a));
  }

  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();
}


// ----------------------------------------------------------------------
int calPixelQuality::getStatus(unsigned int chipid, int icol, int irow) {
  return static_cast<int>(fMapConstants[chipid].matrix[icol][irow]);
}


// ----------------------------------------------------------------------
void calPixelQuality::printPixelQuality(unsigned int chipid, int minimumStatus) {
  for (int ix = 0; ix < 256; ++ix) {
    for (int iy = 0; iy < 250; ++iy) {
      if (fMapConstants[chipid].matrix[ix][iy] > minimumStatus) {
        cout << "chipID = " << chipid
             << " x/y = " << ix << "/" << iy
             << " status = " << static_cast<unsigned int>(fMapConstants[chipid].matrix[ix][iy])
             << endl;
      }
    }
  }
}


// ----------------------------------------------------------------------
void calPixelQuality::printBLOB(std::string sblob, int verbosity) {

  std::vector<char> buffer(sblob.begin(), sblob.end());
  std::vector<char>::iterator ibuffer = buffer.begin();
  
  long unsigned int header = blob2UnsignedInt(getData(ibuffer)); 
  cout << "calPixelQuality::printBLOB(string)" << endl;
  cout << "   header: " << hex << header << dec << endl;

  string summary("calPixelQuality chips "); 
  int nnp(0);
  while (ibuffer != buffer.end()) {
    // -- chipID
    unsigned int chipID = blob2UnsignedInt(getData(ibuffer));
    // -- get number of pixel entries
    int npix = blob2Int(getData(ibuffer));
    if (0 == verbosity) {
      summary += chipID + " ";
      nnp += npix;
    } else {
      cout << "   chipID: " << chipID << " npix: " << npix << endl;
      for (int i = 0; i < npix; ++i) {
        int icol           = blob2Int(getData(ibuffer));
        int irow           = blob2Int(getData(ibuffer));
        unsigned int iqual = blob2UnsignedInt(getData(ibuffer));
        cout << "      icol/irow = " << icol << "/" << irow << " iqual = " << iqual << endl;
      }
    }
  }
  if (0 == verbosity) {
    cout << summary << " with " << nnp << " noisy pixels" << endl;
  }
}


// ----------------------------------------------------------------------
map<unsigned int, vector<double> > calPixelQuality::decodeBLOB(string spl) {
  map<unsigned int, vector<double> > vmap;
  
  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();
  
  long unsigned int header = blob2UnsignedInt(getData(ibuffer)); 
  if (0xdeadface != header) {
    cout << "XXXXX ERRROR in calPixelQuality::decodeBLOB> header is wrong. Something is really messed up!" << endl;
  }
  while (ibuffer != buffer.end()) {
    // -- chipID
    unsigned int chipID = blob2UnsignedInt(getData(ibuffer));
    vector<double> vdet; 
    // -- get number of pixel entries
    int npix = blob2Int(getData(ibuffer));
    for (int i = 0; i < npix; ++i) {
      int icol           = blob2Int(getData(ibuffer));
      int irow           = blob2Int(getData(ibuffer));
      unsigned int iqual = blob2UnsignedInt(getData(ibuffer));
      vdet.push_back(static_cast<double>(icol)); 
      vdet.push_back(static_cast<double>(irow)); 
      vdet.push_back(static_cast<double>(iqual)); 
    }
    vmap.insert(make_pair(chipID, vdet));
  }

  return vmap;
}


// ----------------------------------------------------------------------
string calPixelQuality::makeBLOB(map<unsigned int, vector<double> > m) {
  stringstream s;
  long unsigned int header(0xdeadface);
  s << dumpArray(uint2Blob(header));

  // -- format of m
  // chipID => [npix, n*(col, row, iqual)]
  for (auto it: m) {
    s << dumpArray(uint2Blob(it.first));
    int npix = it.second.size()/3;
    s << dumpArray(int2Blob(npix));
    for (int ipix = 0; ipix < npix; ++ipix) {
      int idx   = ipix*3;
      int icol  = static_cast<int>(it.second[idx]);
      idx       = ipix*3 + 1;
      int irow  = static_cast<int>(it.second[idx]);
      idx       = ipix*3 + 2;
      int iqual = static_cast<int>(it.second[idx]);

      s << dumpArray(int2Blob(icol));
      s << dumpArray(int2Blob(irow));
      s << dumpArray(int2Blob(iqual));
    }
  }
  return s.str();
}
