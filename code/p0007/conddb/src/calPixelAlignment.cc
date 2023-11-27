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
  if (0) cout << "calPixelAlignment created and registered with tag ->" << fTag << "<-" 
              << endl;
}


// ----------------------------------------------------------------------
calPixelAlignment::~calPixelAlignment() {
  cout << "this is the end of calPixelAlignment with tag ->" << fTag << "<-" << endl;
}


// ----------------------------------------------------------------------
void calPixelAlignment::calculate(string hash) {
  cout << "calPixelAlignment::calculate() with "
       << "fHash ->" << hash << "<-";
  fMapConstants.clear();
  string spl = fTagIOVPayloadMap[hash].fBLOB;

  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();
  
  long unsigned int header = blob2UnsignedInt(getData(ibuffer)); 
  cout << " header: " << hex << header << dec;

  int cntPrint(0);
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
    if (cntPrint < -1) {
      cout << "added ID = " << a.id << " v = " << a.vx << "/" << a.vy << "/" << a.vz << endl;
      ++cntPrint;
    }
  }
  cout << " inserted " << fMapConstants.size() << " constants" << endl;
  
  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();
}


// ----------------------------------------------------------------------
void calPixelAlignment::printBLOB(std::string sblob, int verbosity) {

  std::vector<char> buffer(sblob.begin(), sblob.end());
  std::vector<char>::iterator ibuffer = buffer.begin();
  
  long unsigned int header = blob2UnsignedInt(getData(ibuffer)); 
  cout << "calPixelAlignment::printBLOB(string)" << endl;
  cout << "   header: " << hex << header << dec << endl;
  
  while (ibuffer != buffer.end()) {
    // -- chipID
    unsigned int chipID = blob2UnsignedInt(getData(ibuffer));

    cout << "   sensor = " << chipID
         << " v = "
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
         << "n = "
         << blob2Int(getData(ibuffer)) << "/" 
         << blob2Int(getData(ibuffer)) << " " 
         << "rest = "
         << blob2Double(getData(ibuffer)) << "/" 
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
    cout << "XXXXX ERRROR in calPixelAlignment::decodeBLOB> header is wrong. Something is really messed up!" << endl;
  }
  while (ibuffer != buffer.end()) {
    // -- chipID
    unsigned int chipID = blob2UnsignedInt(getData(ibuffer));
    vector<double> vdet;
    constants a;
    a.id = chipID;
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
string calPixelAlignment::makeBLOB() {
  stringstream s;
  long unsigned int header(0xdeadface);
  s << dumpArray(uint2Blob(header));

  for (auto it: fMapConstants) {
    s << dumpArray(uint2Blob(it.first));
    constants a = it.second;
    // -- vx,vy,vz
    s << dumpArray(double2Blob(a.vx));
    s << dumpArray(double2Blob(a.vy));
    s << dumpArray(double2Blob(a.vz));
    // -- rowx,rowy,rowz
    s << dumpArray(double2Blob(a.rowx));
    s << dumpArray(double2Blob(a.rowy));
    s << dumpArray(double2Blob(a.rowz));
    // -- colx,coly,colz
    s << dumpArray(double2Blob(a.colx));
    s << dumpArray(double2Blob(a.coly));
    s << dumpArray(double2Blob(a.colz));
    // -- nrow,ncol
    s << dumpArray(int2Blob(static_cast<int>(a.nrow)));
    s << dumpArray(int2Blob(static_cast<int>(a.ncol)));
    // -- width,length,thickness,pixelSize
    s << dumpArray(double2Blob(a.width));
    s << dumpArray(double2Blob(a.length));
    s << dumpArray(double2Blob(a.thickness));
    s << dumpArray(double2Blob(a.pixelSize));
  }
  return s.str();
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


// ----------------------------------------------------------------------
string calPixelAlignment::readCsv(string filename) {
  string spl("");
  ifstream INS(filename); 
  if (!INS.is_open()) {
    return string("calPixelCablingMap::readCsv> Error, file " + filename + " not found");  
  }

  string sline;
  while (getline(INS, sline)) {
    spl += sline; 
    spl += ",";
  }
  INS.close();

  spl.pop_back();
  vector<string> tokens = split(spl, ',');

  for (unsigned int it = 0; it < tokens.size(); it += 16) {
    constants a;
    int idx = it; 
    a.id = ::stoi(tokens[idx++]);
    a.vx = ::stod(tokens[idx++]);
    a.vy = ::stod(tokens[idx++]);
    a.vz = ::stod(tokens[idx++]);

    a.rowx = ::stod(tokens[idx++]);
    a.rowy = ::stod(tokens[idx++]);
    a.rowz = ::stod(tokens[idx++]);

    a.colx = ::stod(tokens[idx++]);
    a.coly = ::stod(tokens[idx++]);
    a.colz = ::stod(tokens[idx++]);

    a.nrow = ::stoi(tokens[idx++]);
    a.ncol = ::stoi(tokens[idx++]);

    a.width     = ::stod(tokens[idx++]);
    a.length    = ::stod(tokens[idx++]);
    a.thickness = ::stod(tokens[idx++]);
    a.pixelSize = ::stod(tokens[idx++]);

    fMapConstants.insert(make_pair(a.id, a));

  }

  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();

  return spl;
}
