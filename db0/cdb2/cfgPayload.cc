#include "cfgPayload.hh"
#include "base64.hh"
#include "cdbUtil.hh"

#include <iostream>
#include <sstream>

using namespace std;


// ----------------------------------------------------------------------
cfgPayload::cfgPayload() : fHash("cfg_X_gt"),
                           fDate("insertion date"),
                           fCfgString(std::string("empty configuration string")),
                           fError("unset") {
};


// ----------------------------------------------------------------------
cfgPayload::cfgPayload(string cfgPayloadFile) {
  fHash      = jsonGetValue(cfgPayloadFile, "cfgHash");
  fDate      = jsonGetValue(cfgPayloadFile, "cfgDate");
  fCfgString = jsonGetCfgStringEsc(cfgPayloadFile, "cfgString");
}


// ----------------------------------------------------------------------
void cfgPayload::print(bool prtAll) {
  cout << getString(prtAll) << endl;
}


// ----------------------------------------------------------------------
string cfgPayload::getString(bool prtAll) {
  std::stringstream sstr;
  sstr << "/**/hash ->" << fHash << endl
       << "<- /**/date ->" << fDate << endl
       << "<- /**/cfg string ->"
       << (prtAll? fCfgString : " BLOB printing suppressed")
       << "<-";
  return sstr.str();
}


// ----------------------------------------------------------------------
string cfgPayload::getJson() {
  std::stringstream sstr;
  sstr << "{\"cfgHash\" : \"" << fHash << "\", " << endl
       << "\"cfgDate\" : \"" << fDate << "\", " << endl
       << "\"cfgString\" : \"" << endl
       << fCfgString << endl
       << "\"}";
  return sstr.str();
}


// ----------------------------------------------------------------------
void cfgPayload::readFromFile(string hash, string dir) {
  // -- initialize with default
  std::stringstream sspl;
  sspl << "(cdbJSON>  config for hash = " << hash
       << " not found)";

  fCfgString = sspl.str();

  // -- read config
  ifstream INS;
  string filename = dir + "/" + hash;
  INS.open(filename);
  if (INS.fail()) {
    cout << "Error failed to open ->" << filename << "<-" << endl;
    fError = "Error: file not found";
    return;
  }

  std::stringstream buffer;
  buffer << INS.rdbuf();
  INS.close();

  string jstring = buffer.str();
  fHash      = jsonGetString(jstring, "cfgHash");
  fDate      = jsonGetString(jstring, "cfgDate");
  fCfgString = jsonGetCfgStringEsc(jstring, "cfgString");

}
