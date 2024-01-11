#include "calTileAlignment.hh"

#include "cdbUtil.hh"

#include <iostream>
#include <sstream>


using namespace std;

// ----------------------------------------------------------------------
calTileAlignment::calTileAlignment(cdbAbs *db) : calAbs(db) {
}


// ----------------------------------------------------------------------
bool calTileAlignment::getNextID(uint32_t &ID) {
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
calTileAlignment::calTileAlignment(cdbAbs *db, string tag) : calAbs(db, tag) {
  if (0) 	cout << "calTileAlignment created and registered with tag ->" << fTag << "<-" 
               << endl;
}


// ----------------------------------------------------------------------
calTileAlignment::~calTileAlignment() {
  cout << "this is the end of calTileAlignment with tag ->" << fTag << "<-" << endl;
}


// ----------------------------------------------------------------------
void calTileAlignment::calculate(string hash) {
  cout << "calTileAlignment::calculate() with "
       << "fHash ->" << hash << "<- ";
  fMapConstants.clear();
  string spl = fTagIOVPayloadMap[hash].fBLOB;

  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();
  
  long unsigned int header = blob2UnsignedInt(getData(ibuffer)); 
  cout << "header: " << hex << header << dec;

  int cntPrint(0);
  while (ibuffer != buffer.end()) {
    constants a; 
    a.id = blob2UnsignedInt(getData(ibuffer));
    a.sensor = blob2Int(getData(ibuffer));
    a.posx = blob2Double(getData(ibuffer));
    a.posy = blob2Double(getData(ibuffer));
    a.posz = blob2Double(getData(ibuffer));
    a.dirx = blob2Double(getData(ibuffer));
    a.diry = blob2Double(getData(ibuffer));
    a.dirz = blob2Double(getData(ibuffer));

    fMapConstants.insert(make_pair(a.id, a));
    if (cntPrint < -1) {
      cout << "added tile ID = " << a.id << " pos = " << a.posx << "/" << a.posy << "/" << a.posz << endl;
      ++cntPrint;
    }
  }
  cout << " inserted " << fMapConstants.size() << " constants" << endl;

  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();
}


// ----------------------------------------------------------------------
void calTileAlignment::printBLOB(std::string sblob, int verbosity) {

  std::vector<char> buffer(sblob.begin(), sblob.end());
  std::vector<char>::iterator ibuffer = buffer.begin();
  
  long unsigned int header = blob2UnsignedInt(getData(ibuffer)); 
  cout << "calTileAlignment::printBLOB(string)" << endl;
  cout << "   header: " << hex << header << dec << endl;
  
  while (ibuffer != buffer.end()) {
    cout << "   id = " << blob2UnsignedInt(getData(ibuffer))
         << " sensor = " << blob2Int(getData(ibuffer))
         << " pos = "
         << blob2Double(getData(ibuffer)) << "/" 
         << blob2Double(getData(ibuffer)) << "/" 
         << blob2Double(getData(ibuffer)) << " "
         << "dir = " 
         << blob2Double(getData(ibuffer)) << "/"
         << blob2Double(getData(ibuffer)) << "/"
         << blob2Double(getData(ibuffer)) << " "
         << endl;
  }
}


// ----------------------------------------------------------------------
map<unsigned int, vector<double> > calTileAlignment::decodeBLOB(string spl) {
  map<unsigned int, vector<double> > vmap;
  
  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();
  
  long unsigned int header = blob2UnsignedInt(getData(ibuffer)); 
  if (0xdeadface != header) {
    cout << "XXXXX ERRROR in calTileAlignment::decodeBLOB> header is wrong. Something is really messed up!" << endl;
  }
  while (ibuffer != buffer.end()) {
    unsigned int id = blob2UnsignedInt(getData(ibuffer));
    int sensor      = blob2Int(getData(ibuffer));
    vector<double> vdet;
    constants a;
    a.sensor = sensor;
    a.id = id;
    a.posx = blob2Double(getData(ibuffer));
    a.posy = blob2Double(getData(ibuffer));
    a.posz = blob2Double(getData(ibuffer));
    a.dirx = blob2Double(getData(ibuffer));
    a.diry = blob2Double(getData(ibuffer));
    a.dirz = blob2Double(getData(ibuffer));
    vdet.push_back(static_cast<double>(sensor));
    vdet.push_back(a.posx);      vdet.push_back(a.posy);   vdet.push_back(a.posz);
    vdet.push_back(a.dirx);      vdet.push_back(a.diry);   vdet.push_back(a.dirz);
    vmap.insert(make_pair(id, vdet));
  }

  return vmap;
}


// ----------------------------------------------------------------------
string calTileAlignment::makeBLOB() {
  stringstream s;
  long unsigned int header(0xdeadface);
  s << dumpArray(uint2Blob(header));

  for (auto it: fMapConstants) {
    // id => sensor,posx,posy,posz,dirx,diry,dirz
    s << dumpArray(uint2Blob(it.first));
    constants a = it.second;
    s << dumpArray(int2Blob(a.sensor));

    s << dumpArray(double2Blob(a.posx));
    s << dumpArray(double2Blob(a.posy));
    s << dumpArray(double2Blob(a.posz));

    s << dumpArray(double2Blob(a.dirx));
    s << dumpArray(double2Blob(a.diry));
    s << dumpArray(double2Blob(a.dirz));
  }
  return s.str();
}


// ----------------------------------------------------------------------
string calTileAlignment::makeBLOB(map<unsigned int, vector<double> > m) {
  stringstream s;
  long unsigned int header(0xdeadface);
  s << dumpArray(uint2Blob(header));

  // -- format of m
  // id => sensor,posx,posy,posz,dirx,diry,dirz
  for (auto it: m) {
    // -- id and sensor
    s << dumpArray(uint2Blob(it.first));    
    s << dumpArray(int2Blob(static_cast<int>(it.second[0]))); 
    // -- posx,posy,posz
    s << dumpArray(double2Blob(it.second[1]));
    s << dumpArray(double2Blob(it.second[2]));
    s << dumpArray(double2Blob(it.second[3]));
    // -- dirx,diry,dirz
    s << dumpArray(double2Blob(it.second[4]));
    s << dumpArray(double2Blob(it.second[5]));
    s << dumpArray(double2Blob(it.second[6]));
  }
  return s.str();
}


// ----------------------------------------------------------------------
string calTileAlignment::readCsv(string filename) {
  string spl("");
  ifstream INS(filename); 
  if (!INS.is_open()) {
    return string("calTileAlignment::readCsv> Error, file " + filename + " not found");  
  }

  string sline;
  while (getline(INS, sline)) {
    spl += sline; 
    spl += ",";
  }
  INS.close();

  spl.pop_back();
  vector<string> tokens = split(spl, ',');

  for (unsigned int it = 0; it < tokens.size(); it += 8) {
    constants a;
    int idx = it; 
    a.id     = static_cast<unsigned int>(::stoi(tokens[idx++]));
    a.sensor = ::stoi(tokens[idx++]);
    a.posx   = ::stod(tokens[idx++]);
    a.posy   = ::stod(tokens[idx++]);
    a.posz   = ::stod(tokens[idx++]);
    a.dirx   = ::stod(tokens[idx++]);
    a.diry   = ::stod(tokens[idx++]);
    a.dirz   = ::stod(tokens[idx++]);

    fMapConstants.insert(make_pair(a.id, a));

  }

  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();

  return spl;
}

