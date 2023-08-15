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


// ----------------------------------------------------------------------
void calPixelAlignment::printBLOB(std::string sblob) {

  std::vector<char> buffer(sblob.begin(), sblob.end());
  std::vector<char>::iterator ibuffer = buffer.begin();
  
  long unsigned int header = blob2UnsignedInt(getData(ibuffer)); 
  cout << "calPixelAlignment::printBLOB(string)" << endl;
  cout << "   header: " << hex << header << dec << endl;
  
  while (ibuffer != buffer.end()) {
    // -- chipID
    unsigned int chipID = blob2UnsignedInt(getData(ibuffer));

    cout << "v = "
         << blob2Double(getData(ibuffer)) << "/" 
         << blob2Double(getData(ibuffer)) << "/" 
         << blob2Double(getData(ibuffer)) << " "
         << "row = " 
         << blob2Double(getData(ibuffer)) << "/"
         << blob2Double(getData(ibuffer)) << "/"
         << blob2Double(getData(ibuffer)) << " "
         << "col = " 
         << blob2Double(getData(ibuffer)) << "/"
         << blob2Double(getData(ibuffer)) << "/"
         << blob2Double(getData(ibuffer)) << "/"
         << endl
         << "n = "
         << blob2Int(getData(ibuffer)) << "/" 
         << blob2Int(getData(ibuffer)) << " " 
         << "rest = "
         << blob2Double(getData(ibuffer)) << "/" 
         << blob2Double(getData(ibuffer)) << "/" 
         << blob2Double(getData(ibuffer))
         << endl;
  }
}


// ----------------------------------------------------------------------
map<unsigned int, vector<double> > calPixelAlignment::decodeBLOB(string spl) {
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
    constants a;
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
    vdet.push_back(a.vx);        vdet.push_back(a.vy);   vdet.push_back(a.vz);
    vdet.push_back(a.rowx);      vdet.push_back(a.rowy); vdet.push_back(a.rowz);
    vdet.push_back(a.colx);      vdet.push_back(a.coly); vdet.push_back(a.colz);
    vdet.push_back(a.nrow);      vdet.push_back(a.ncol);
    vdet.push_back(a.width);     vdet.push_back(a.length);
    vdet.push_back(a.thickness); vdet.push_back(a.pixelSize);
    vmap.insert(make_pair(chipID, vdet));
  }

  return vmap;
}


// ----------------------------------------------------------------------
string calPixelAlignment::makeBLOB(map<unsigned int, vector<double> > m) {
  stringstream s;
  long unsigned int header(0xdeadface);
  s << dumpArray(uint2Blob(header));

  // -- format of m
  // chipID => vx,vy,vz,rowx,rowy,rowz,colx,coly,colz,nrow,ncol,width,length,thickness,pixelSize
  for (auto it: m) {
    s << dumpArray(uint2Blob(it.first));
    int npix = it.second.size()/3;
    // -- vx,vy,vz
    s << dumpArray(double2Blob(it.second[0]));
    s << dumpArray(double2Blob(it.second[1]));
    s << dumpArray(double2Blob(it.second[2]));
    // -- rowx,rowy,rowz
    s << dumpArray(double2Blob(it.second[3]));
    s << dumpArray(double2Blob(it.second[4]));
    s << dumpArray(double2Blob(it.second[5]));
    // -- colx,coly,colz
    s << dumpArray(double2Blob(it.second[6]));
    s << dumpArray(double2Blob(it.second[7]));
    s << dumpArray(double2Blob(it.second[8]));
    // -- nrow,ncol
    s << dumpArray(int2Blob(static_cast<int>(it.second[9])));
    s << dumpArray(int2Blob(static_cast<int>(it.second[10])));
    // -- width,length,thickness,pixelSize
    s << dumpArray(double2Blob(it.second[11]));
    s << dumpArray(double2Blob(it.second[12]));
    s << dumpArray(double2Blob(it.second[13]));
    s << dumpArray(double2Blob(it.second[14]));
  }
  return s.str();
}
