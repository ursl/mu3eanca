#include "cfgPayload.hh"
#include "base64.hh"
#include "cdbUtil.hh"

#include <iostream>
#include <sstream>

using namespace std;


// ----------------------------------------------------------------------
cfgPayload::cfgPayload() : fHash("cfg_X_gt"),
                           fDate("insertion date"), 
                           fCfgString(std::string("empty configuration string")) {
  
};


// ----------------------------------------------------------------------
cfgPayload::cfgPayload(string cfgPayloadFile) {
  fHash      = jsonGetValue(cfgPayloadFile, "cfgHash");
  fDate      = jsonGetValue(cfgPayloadFile, "cfgDate");
  // fCfgString = base64_decode(jsonGetCfgString(jstring, "cfgString"));
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

