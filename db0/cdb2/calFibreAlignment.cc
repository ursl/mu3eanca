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
  if (fMapConstants.size() < 1) {
    cout << "calFibreAlignment::getNextID> ERROR: no constants loaded. Did you set a runnumber in Mu3eConditions?"
         << endl;
  }
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
  if (0) cout << "calFibreAlignment created and registered with tag ->" << fTag << "<-"
              << endl;
}


// ----------------------------------------------------------------------
calFibreAlignment::~calFibreAlignment() {
  cout << "this is the end of calFibreAlignment with tag ->" << fTag << "<-" << endl;
}


// ----------------------------------------------------------------------
void calFibreAlignment::calculate(string hash) {
  cout << "calFibreAlignment::calculate() with "
       << "fHash ->" << hash << "<- ";
  fMapConstants.clear();
  string spl = fTagIOVPayloadMap[hash].fBLOB;

  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  cout << " header: " << hex << header << dec;

  int cntPrint(0);
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
    if (cntPrint < -1) {
      cout << "added fibre ID = " << a.id << " c = " << a.cx << "/" << a.cy << "/" << a.cz << endl;
      ++cntPrint;
    }
  }
  cout << " inserted " << fMapConstants.size() << " constants" << endl;

  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();
}


// ----------------------------------------------------------------------
void calFibreAlignment::printBLOB(std::string sblob, int verbosity) {

  std::vector<char> buffer(sblob.begin(), sblob.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  cout << "calFibreAlignment::printBLOB(string)" << endl;
  cout << "   header: " << hex << header << dec << endl;

  int cnt(0);
  while (ibuffer != buffer.end()) {
    if (verbosity > 0) ++cnt;
    if (cnt > verbosity) break;
    // -- chipID
    unsigned int chipID = blob2UnsignedInt(getData(ibuffer));
    cout << "   sensor = " << chipID
         << " c = "
         << blob2Double(getData(ibuffer)) << "/"
         << blob2Double(getData(ibuffer)) << "/"
         << blob2Double(getData(ibuffer)) << " "
         << "f = "
         << blob2Double(getData(ibuffer)) << "/"
         << blob2Double(getData(ibuffer)) << "/"
         << blob2Double(getData(ibuffer)) << " "
         << "round = " << blob2UnsignedInt(getData(ibuffer)) << "/"
         << "square = " << blob2UnsignedInt(getData(ibuffer)) << "/"
         << "diameter = " << blob2Double(getData(ibuffer))
         << endl;
  }
}


// ----------------------------------------------------------------------
map<unsigned int, vector<double> > calFibreAlignment::decodeBLOB(string spl) {
  map<unsigned int, vector<double> > vmap;

  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  if (0xdeadface != header) {
    cout << "XXXXX ERRROR in calFibreAlignment::decodeBLOB> header is wrong. Something is really messed up!" << endl;
  }
  while (ibuffer != buffer.end()) {
    // -- chipID
    unsigned int chipID = blob2UnsignedInt(getData(ibuffer));
    vector<double> vdet;
    constants a;
    a.id = chipID;
    a.cx = blob2Double(getData(ibuffer));
    a.cy = blob2Double(getData(ibuffer));
    a.cz = blob2Double(getData(ibuffer));
    a.fx = blob2Double(getData(ibuffer));
    a.fy = blob2Double(getData(ibuffer));
    a.fz = blob2Double(getData(ibuffer));
    a.round = blob2Double(getData(ibuffer));
    a.square = blob2Double(getData(ibuffer));
    a.diameter = blob2Double(getData(ibuffer));
    vdet.push_back(a.cx);
    vdet.push_back(a.cy);
    vdet.push_back(a.cz);
    vdet.push_back(a.fx);
    vdet.push_back(a.fy);
    vdet.push_back(a.fz);
    vdet.push_back(static_cast<double>(a.round));
    vdet.push_back(static_cast<double>(a.square));
    vdet.push_back(a.diameter);
    vmap.insert(make_pair(chipID, vdet));
  }

  return vmap;
}


// ----------------------------------------------------------------------
string calFibreAlignment::makeBLOB() {
  stringstream s;
  unsigned int header(0xdeadface);
  s << dumpArray(uint2Blob(header));

  for (auto it: fMapConstants) {
    s << dumpArray(uint2Blob(it.first));
    constants a = it.second;
    // -- cx,cy,cz
    s << dumpArray(double2Blob(a.cx));
    s << dumpArray(double2Blob(a.cy));
    s << dumpArray(double2Blob(a.cz));
    // -- fx,fy,fz
    s << dumpArray(double2Blob(a.fx));
    s << dumpArray(double2Blob(a.fy));
    s << dumpArray(double2Blob(a.fz));
    // -- rest
    s << dumpArray(int2Blob(static_cast<int>(a.round)));
    s << dumpArray(int2Blob(static_cast<int>(a.square)));
    s << dumpArray(double2Blob(a.diameter));
  }
  return s.str();
}


// ----------------------------------------------------------------------
string calFibreAlignment::makeBLOB(const map<unsigned int, vector<double>>& m) {
  stringstream s;
  unsigned int header(0xdeadface);
  s << dumpArray(uint2Blob(header));

  // -- format of m
  // chipID => cx,cy,cz,fx,fy,fz,round,square,diameter
  for (auto it: m) {
    s << dumpArray(uint2Blob(it.first));
    // -- cx,cy,cz
    s << dumpArray(double2Blob(it.second[0]));
    s << dumpArray(double2Blob(it.second[1]));
    s << dumpArray(double2Blob(it.second[2]));
    // -- fx,fy,fz
    s << dumpArray(double2Blob(it.second[3]));
    s << dumpArray(double2Blob(it.second[4]));
    s << dumpArray(double2Blob(it.second[5]));
    // -- rest
    s << dumpArray(int2Blob(static_cast<int>(it.second[6])));
    s << dumpArray(int2Blob(static_cast<int>(it.second[7])));
    s << dumpArray(double2Blob(it.second[8]));
  }
  return s.str();
}


// ----------------------------------------------------------------------
string calFibreAlignment::readCsv(string filename) {
  string spl("");
  ifstream INS(filename);
  if (!INS.is_open()) {
    return "calFibreAlignment::readCsv> Error, file " + filename + " not found";
  }

  string sline;
  while (getline(INS, sline)) {
    spl += sline;
    spl += ",";
  }
  INS.close();

  spl.pop_back();
  vector<string> tokens = split(spl, ',');

  for (unsigned int it = 0; it < tokens.size(); it += 10) {
    constants a;
    int idx = it;
    a.id = ::stoi(tokens[idx++]);
    a.cx = ::stod(tokens[idx++]);
    a.cy = ::stod(tokens[idx++]);
    a.cz = ::stod(tokens[idx++]);

    a.fx = ::stod(tokens[idx++]);
    a.fy = ::stod(tokens[idx++]);
    a.fz = ::stod(tokens[idx++]);

    a.round    = static_cast<bool>(::stoi(tokens[idx++]));
    a.square   = static_cast<bool>(::stoi(tokens[idx++]));
    a.diameter = ::stod(tokens[idx++]);

    fMapConstants.insert(make_pair(a.id, a));

  }

  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();

  return spl;
}
