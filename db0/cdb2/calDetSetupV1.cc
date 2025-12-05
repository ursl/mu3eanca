#include "calDetSetupV1.hh"

#include "cdbUtil.hh"

#include <iostream>
#include <sstream>


using namespace std;

// ----------------------------------------------------------------------
calDetSetupV1::calDetSetupV1(cdbAbs *db) : calAbs(db) {
}


// ----------------------------------------------------------------------
calDetSetupV1::calDetSetupV1(cdbAbs *db, string tag) : calAbs(db, tag) {
  if (0) cout << "calDetSetupV1 created and registered with tag ->"
                       << fTag << "<-"
                       << endl;
}


// ----------------------------------------------------------------------
calDetSetupV1::~calDetSetupV1() {
  cout << "this is the end of calDetSetupV1 with tag ->" << fTag << "<-" << endl;
}


// ----------------------------------------------------------------------
void calDetSetupV1::calculate(string hash) {
  cout << "calDetSetupV1::calculate() with "
       << "fHash ->" << hash << "<-";
  string spl = fTagIOVPayloadMap[hash].fBLOB;

  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  cout << " header: " << hex << header << dec << endl;

  fConstants.target.shape = blob2UnsignedInt(getData(ibuffer));
  fConstants.target.thickness1 = blob2Double(getData(ibuffer));
  fConstants.target.thickness2 = blob2Double(getData(ibuffer));
  fConstants.target.length = blob2Double(getData(ibuffer));
  fConstants.target.radius = blob2Double(getData(ibuffer));
  fConstants.target.offsetX = blob2Double(getData(ibuffer));
  fConstants.target.offsetY  = blob2Double(getData(ibuffer));
  fConstants.target.offsetZ  = blob2Double(getData(ibuffer));
  fConstants.magnet.fieldStrength = blob2Double(getData(ibuffer));

}


// ----------------------------------------------------------------------
void calDetSetupV1::printBLOB(std::string sblob, int verbosity) {
  cout << printBLOBString(sblob, verbosity) << endl;
}

// ----------------------------------------------------------------------
string calDetSetupV1::printBLOBString(std::string sblob, int verbosity) {
  stringstream ss;

  std::vector<char> buffer(sblob.begin(), sblob.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  ss << "calDetSetupV1::printBLOB(string)" << endl;
  ss << "   header: " << hex << header << dec << endl;

  if (0 == verbosity) return ss.str();

  ss << "target"  << endl
     << "  .shape = " << blob2UnsignedInt(getData(ibuffer)) << endl
     << "  .thickness1 = " << blob2Double(getData(ibuffer)) << endl
     << "  .thickness2 = " << blob2Double(getData(ibuffer)) << endl
     << "  .length = " << blob2Double(getData(ibuffer)) << endl
     << "  .radius = " << blob2Double(getData(ibuffer)) << endl
     << "  .Offset x/y/z = " << blob2Double(getData(ibuffer))
     << "/" << blob2Double(getData(ibuffer))
     << "/" << blob2Double(getData(ibuffer)) << endl
     << "magnet.field" << endl
     << "  .strength = " << blob2Double(getData(ibuffer)) << endl;
  return ss.str();
}


// ----------------------------------------------------------------------
string calDetSetupV1::makeBLOB() {
  stringstream s;
  unsigned int header(0xdeadface);
  s << dumpArray(uint2Blob(header));

  s << dumpArray(uint2Blob(fConstants.target.shape));
  s << dumpArray(double2Blob(fConstants.target.thickness1));
  s << dumpArray(double2Blob(fConstants.target.thickness2));
  s << dumpArray(double2Blob(fConstants.target.length));
  s << dumpArray(double2Blob(fConstants.target.radius));
  s << dumpArray(double2Blob(fConstants.target.offsetX));
  s << dumpArray(double2Blob(fConstants.target.offsetY));
  s << dumpArray(double2Blob(fConstants.target.offsetZ));
  s << dumpArray(double2Blob(fConstants.magnet.fieldStrength));
  return s.str();
}


// ----------------------------------------------------------------------
string calDetSetupV1::makeBLOB(const map<unsigned int, vector<double>>& m) {
  stringstream s;
  unsigned int header(0xdeadface);
  s << dumpArray(uint2Blob(header));

  // -- format of m
  // 0 => target.(shape,thickness1,thickness2,length,radius,offsetX,offsetY,offsetZ),magnet.fieldStrength
  for (auto it: m) {
    s << dumpArray(int2Blob(static_cast<int>(it.second[0])));
    s << dumpArray(double2Blob(it.second[1]));
    s << dumpArray(double2Blob(it.second[2]));
    s << dumpArray(double2Blob(it.second[3]));
    s << dumpArray(double2Blob(it.second[4]));
    s << dumpArray(double2Blob(it.second[5]));
    s << dumpArray(double2Blob(it.second[6]));
    s << dumpArray(double2Blob(it.second[7]));
    s << dumpArray(double2Blob(it.second[8]));
  }
  return s.str();
}


// ----------------------------------------------------------------------
string calDetSetupV1::readJSON(string filename) {
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

  fConstants.target.shape         = ::stoi(jsonGetValue(spl, vector<string> {"detector", "target", "shape"}));
  fConstants.target.thickness1    = ::stod(jsonGetValue(spl, vector<string> {"detector", "target", "thickness1"}));
  fConstants.target.thickness2    = ::stod(jsonGetValue(spl, vector<string> {"detector", "target", "thickness2"}));
  fConstants.target.length        = ::stod(jsonGetValue(spl, vector<string> {"detector", "target", "length"}));
  fConstants.target.radius        = ::stod(jsonGetValue(spl, vector<string> {"detector", "target", "radius"}));
  fConstants.target.offsetX       = ::stod(jsonGetValue(spl, vector<string> {"detector", "target", "offset", "x"}));
  fConstants.target.offsetY       = ::stod(jsonGetValue(spl, vector<string> {"detector", "target", "offset", "y"}));
  fConstants.target.offsetZ       = ::stod(jsonGetValue(spl, vector<string> {"detector", "target", "offset", "z"}));
  fConstants.magnet.fieldStrength = ::stod(jsonGetValue(spl, vector<string> {"detector", "magnet", "field", "strength"}));

  return spl;
}
