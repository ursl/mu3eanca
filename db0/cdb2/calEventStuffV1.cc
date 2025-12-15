#include "calEventStuffV1.hh"

#include "cdbUtil.hh"

#include <iostream>
#include <sstream>


using namespace std;

// ----------------------------------------------------------------------
calEventStuffV1::calEventStuffV1(cdbAbs *db) : calAbs(db) {
}


// ----------------------------------------------------------------------
calEventStuffV1::calEventStuffV1(cdbAbs *db, string tag) : calAbs(db, tag) {
  if (0) cout << "calEventStuffV1 created and registered with tag ->"
              << tag << "<-"
              << endl;
}


// ----------------------------------------------------------------------
calEventStuffV1::~calEventStuffV1() {
  cout << "this is the end of calEventStuffV1 with tag ->" << fTag << "<-" << endl;
  }


// ----------------------------------------------------------------------
void calEventStuffV1::calculate(string hash) {
  cout << "calEventStuffV1::calculate() with "
       << "fHash ->" << hash << "<-";
  string spl = fTagIOVPayloadMap[hash].fBLOB;

  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  cout << " header: " << hex << header << dec << endl;

  fConstants.pixelData.startFrame = blob2Uint64(getData(ibuffer));
  fConstants.pixelData.endFrame = blob2Uint64(getData(ibuffer));
}


// ----------------------------------------------------------------------
void calEventStuffV1::printBLOB(std::string sblob, int verbosity) {
  cout << printBLOBString(sblob, verbosity) << endl;
}

// ----------------------------------------------------------------------
string calEventStuffV1::printBLOBString(std::string sblob, int verbosity) {
  stringstream ss;

  std::vector<char> buffer(sblob.begin(), sblob.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  ss << "calEventStuffV1::printBLOB(string)" << endl;
  ss << "   header: " << hex << header << dec << endl;

  if (0 == verbosity) return ss.str();

  ss << "pixeldata"  << endl
     << "  .startframe = " << blob2Uint64(getData(ibuffer)) << endl
     << "  .endframe = " << blob2Uint64(getData(ibuffer)) << endl;
  return ss.str();
}


// ----------------------------------------------------------------------
string calEventStuffV1::makeBLOB() {
  stringstream s;
  unsigned int header(0xdeadface);
  s << dumpArray(uint2Blob(header));

  s << dumpArray(uint642Blob(fConstants.pixelData.startFrame));
  s << dumpArray(uint642Blob(fConstants.pixelData.endFrame));
  return s.str();
}


// ----------------------------------------------------------------------
string calEventStuffV1::readJSON(string filename) {
  string spl("");
  ifstream INS(filename);
  if (!INS.is_open()) {
    return "calDetSetupV1::readJSON> Error, file " + filename + " not found";
  }

  string sline;
  while (getline(INS, sline)) {
    replaceAll(sline, "\n", " ");
    spl += sline;
  }
  INS.close();


  fConstants.pixelData.startFrame = ::stoull(jsonGetValue(spl, vector<string> {"pixeldata", "startframe"}));
  fConstants.pixelData.endFrame    = ::stoul(jsonGetValue(spl, vector<string> {"pixeldata", "endframe"}));

  return spl;
}
