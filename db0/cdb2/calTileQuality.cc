#include "calTileQuality.hh"

#include "cdbUtil.hh"

#include <iostream>
#include <sstream>


using namespace std;

// ----------------------------------------------------------------------
calTileQuality::calTileQuality(cdbAbs *db) : calAbs(db) {
}


// ----------------------------------------------------------------------
bool calTileQuality::getNextID(uint32_t &ID) {
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
calTileQuality::calTileQuality(cdbAbs *db, string tag) : calAbs(db, tag) {
  cout << "calTileQuality created and registered with tag ->" << fTag << "<-"
       << endl;
}


// ----------------------------------------------------------------------
calTileQuality::~calTileQuality() {
  fMapConstants.clear();
  cout << "this is the end of calTileQuality with tag ->" << fTag << "<-" << endl;
}


// ----------------------------------------------------------------------
void calTileQuality::calculate(string hash) {
  cout << "calTileQuality::calculate() with "
       << "fHash ->" << hash << "<-"
       << endl;
  fMapConstants.clear();
  string spl = fTagIOVPayloadMap[hash].fBLOB;

  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  cout << "calTileQuality header: " << hex << header << dec << endl;

  int npix(0);
  while (ibuffer != buffer.end()) {
    constants a;
    // -- chipID
    a.id = blob2UnsignedInt(getData(ibuffer));
    // -- get number of pixel entries
    npix = blob2Int(getData(ibuffer));
    // -- initialize matrix with zero before filling specified pixels
    a.quality = 0;
    for (int i = 0; i < npix; ++i) {
      int channelIdx     = blob2Int(getData(ibuffer));
      unsigned int iqual = blob2UnsignedInt(getData(ibuffer));
      a.vChannelQuality[channelIdx] = static_cast<int>(iqual);
    }
    // cout << "inserting " << a.id << " with size = " << sizeof(a) << endl;
    fMapConstants.insert(make_pair(a.id, a));
  }

  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();
}


// ----------------------------------------------------------------------
calTileQuality::Status calTileQuality::getStatus(unsigned int chipid, int channelIdx) {
  if (fMapConstants.find(chipid) == fMapConstants.end()) {
    return Status::ChannelNotFound; // -- channel not found
  }
  return static_cast<Status>(fMapConstants[chipid].vChannelQuality[channelIdx]);
}


// ----------------------------------------------------------------------
void calTileQuality::printChannelQuality(unsigned int chipid, int channelIdx) {
  if (fMapConstants[chipid].vChannelQuality[channelIdx] > 0) {
    cout << "chipID = " << chipid
         << " channelIdx = " << channelIdx
         << " status = " << static_cast<unsigned int>(fMapConstants[chipid].vChannelQuality[channelIdx])
         << endl;
  }
}


// ----------------------------------------------------------------------
string calTileQuality::makeBLOB() {
  stringstream s;
  unsigned int header(0xdeadface);
  s << dumpArray(uint2Blob(header));

  // -- format of fMapConstants
  // chipID => [quality]
  for (auto it: fMapConstants) {
    s << dumpArray(uint2Blob(it.first));
    constants a = it.second;
    s << dumpArray(int2Blob(a.quality));
  }
  return s.str();
}

// ----------------------------------------------------------------------
map<unsigned int, vector<double> > decodeBLOB(std::string) {
  std::map<unsigned int, calTileQuality::constants> vmap;
  return vmap;
}


// ----------------------------------------------------------------------
std::string calTileQuality::makeBLOB(const std::map<unsigned int, std::vector<double>>&) {
  return "";
}


// ----------------------------------------------------------------------
map<unsigned int, vector<double> > decodeBLOB() {
  std::map<unsigned int, calTileQuality::constants> vmap;
  return vmap;
}

// ----------------------------------------------------------------------
void calTileQuality::writeCsv(string filename) {
  string spl("");
  ofstream OUT(filename);
  if (!OUT.is_open()) {
    cout << ("calTileQuality::writeCsv> Error, file "
             + filename + " not opened for output")
         << endl;
  }

  OUT << "# Format: channelID,quality"
      << endl;

  for (auto it: fMapConstants) {
    OUT << it.first << "," << it.second.quality << endl;
  }
  OUT.close();
}


// ----------------------------------------------------------------------
void calTileQuality::readCsv(string filename) {
  cout << "calTileQuality::readCsv> reset fMapConstants" << endl;
  fMapConstants.clear();

  string spl("");
  ifstream INS(filename);
  if (!INS.is_open()) {
    cout << ("calTileQuality::readCsv> Error, file "
             + filename + " not found")
         << endl;
  }

  vector<string> vline;
  while (getline(INS, spl)) {
    if (string::npos == spl.find("#")) {
      vline.push_back(spl);
    }
  }
  INS.close();

  for (unsigned int it = 0; it < vline.size(); ++it) {
    constants a;
    vector<string> tokens = split(vline[it], ',');
    a.id = stoi(tokens[0]);
    a.quality = stoi(tokens[1]);
    fMapConstants.insert(make_pair(a.id, a));
  }

  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();
}
