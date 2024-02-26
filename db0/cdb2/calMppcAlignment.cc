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
  if (0) 	cout << "calMppcAlignment created and registered with tag ->" << fTag << "<-"
               << endl;
}


// ----------------------------------------------------------------------
calMppcAlignment::~calMppcAlignment() {
  cout << "this is the end of calMppcAlignment with tag ->" << fTag << "<-" << endl;
}


// ----------------------------------------------------------------------
void calMppcAlignment::calculate(string hash) {
  cout << "calMppcAlignment::calculate() with "
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
    a.mppc = blob2UnsignedInt(getData(ibuffer));
    a.vx = blob2Double(getData(ibuffer));
    a.vy = blob2Double(getData(ibuffer));
    a.vz = blob2Double(getData(ibuffer));
    a.colx = blob2Double(getData(ibuffer));
    a.coly = blob2Double(getData(ibuffer));
    a.colz = blob2Double(getData(ibuffer));
    a.ncol = blob2Int(getData(ibuffer));

    fMapConstants.insert(make_pair(a.mppc, a));
    if (cntPrint < -1) {
      cout << "added mppc = " << a.mppc
           << " v = " << a.vx << "/" << a.vy << "/" << a.vz
           << " col = " << a.colx << "/" << a.coly << "/" << a.colz
           << endl;
      ++cntPrint;
    }
  }
  cout << " inserted " << fMapConstants.size() << " constants" << endl;

  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();
}


// ----------------------------------------------------------------------
void calMppcAlignment::printBLOB(std::string sblob, int verbosity) {

  std::vector<char> buffer(sblob.begin(), sblob.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  long unsigned int header = blob2UnsignedInt(getData(ibuffer));
  cout << "calMppcAlignment::printBLOB(string)" << endl;
  cout << "   header: " << hex << header << dec << endl;

  int cnt(0);
  while (ibuffer != buffer.end()) {
    if (verbosity > 0) ++cnt;
    if (cnt > verbosity) break;
    cout << "   mppc = "
         << blob2UnsignedInt(getData(ibuffer))
         << " v = "
         << blob2Double(getData(ibuffer)) << "/"
         << blob2Double(getData(ibuffer)) << "/"
         << blob2Double(getData(ibuffer)) << " "
         << "col = "
         << blob2Double(getData(ibuffer)) << "/"
         << blob2Double(getData(ibuffer)) << "/"
         << blob2Double(getData(ibuffer)) << " "
         << "ncol = " << blob2Int(getData(ibuffer))
         << endl;
  }
}


// ----------------------------------------------------------------------
map<unsigned int, vector<double> > calMppcAlignment::decodeBLOB(string spl) {
  map<unsigned int, vector<double> > vmap;

  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  long unsigned int header = blob2UnsignedInt(getData(ibuffer));
  if (0xdeadface != header) {
    cout << "XXXXX ERRROR in calMppcAlignment::decodeBLOB> header is wrong. Something is really messed up!" << endl;
  }
  while (ibuffer != buffer.end()) {
    unsigned int mppc = blob2UnsignedInt(getData(ibuffer));
    vector<double> vdet;
    constants a;
    a.mppc = mppc;
    a.vx   = blob2Double(getData(ibuffer));
    a.vy   = blob2Double(getData(ibuffer));
    a.vz   = blob2Double(getData(ibuffer));
    a.colx = blob2Double(getData(ibuffer));
    a.coly = blob2Double(getData(ibuffer));
    a.colz = blob2Double(getData(ibuffer));
    a.ncol = blob2Int(getData(ibuffer));
    vdet.push_back(a.vx);        vdet.push_back(a.vy);     vdet.push_back(a.vz);
    vdet.push_back(a.colx);      vdet.push_back(a.coly);   vdet.push_back(a.colz);
    vdet.push_back(static_cast<double>(a.ncol));
    vmap.insert(make_pair(mppc, vdet));
  }

  return vmap;
}


// ----------------------------------------------------------------------
string calMppcAlignment::makeBLOB() {
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
    // -- colx,coly,colz
    s << dumpArray(double2Blob(a.colx));
    s << dumpArray(double2Blob(a.coly));
    s << dumpArray(double2Blob(a.colz));
    // -- rest
    s << dumpArray(int2Blob(static_cast<int>(a.ncol)));
  }
  return s.str();
}


// ----------------------------------------------------------------------
string calMppcAlignment::makeBLOB(map<unsigned int, vector<double> > m) {
  stringstream s;
  long unsigned int header(0xdeadface);
  s << dumpArray(uint2Blob(header));

  // -- format of m
  // mppc => vx,vy,vz,colx,coly,colz,ncol
  for (auto it: m) {
    s << dumpArray(uint2Blob(it.first));
    // -- vx,vy,vz
    s << dumpArray(double2Blob(it.second[0]));
    s << dumpArray(double2Blob(it.second[1]));
    s << dumpArray(double2Blob(it.second[2]));
    // -- colx,coly,colz
    s << dumpArray(double2Blob(it.second[3]));
    s << dumpArray(double2Blob(it.second[4]));
    s << dumpArray(double2Blob(it.second[5]));
    // -- ncol
    s << dumpArray(int2Blob(static_cast<int>(it.second[6])));
  }
  return s.str();
}


// ----------------------------------------------------------------------
string calMppcAlignment::readCsv(string filename) {
  string spl("");
  ifstream INS(filename);
  if (!INS.is_open()) {
    return string("calMppcAlignment::readCsv> Error, file " + filename + " not found");
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
    a.mppc = static_cast<unsigned int>(::stoi(tokens[idx++]));
    a.vx = ::stod(tokens[idx++]);
    a.vy = ::stod(tokens[idx++]);
    a.vz = ::stod(tokens[idx++]);

    a.colx = ::stod(tokens[idx++]);
    a.coly = ::stod(tokens[idx++]);
    a.colz = ::stod(tokens[idx++]);

    a.ncol    = ::stoi(tokens[idx++]);

    fMapConstants.insert(make_pair(a.mppc, a));

  }

  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();

  return spl;
}
