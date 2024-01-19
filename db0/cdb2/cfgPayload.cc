#include "cfgPayload.hh"
#include "base64.hh"

#include <iostream>
#include <sstream>

using namespace std;


// ----------------------------------------------------------------------
cfgPayload::cfgPayload() : fHash("cfg_X_gt"),
                           fDate("insertion date"), 
                           fCfgString(std::string("empty configuration string")) {
  
};


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
string cfgPayload::json() {
  std::stringstream sstr;
  sstr << "{\"cfgHash\" : \"" << fHash << "\", "
       << "\"cfgDate\" : \"" << fDate << "\", "
       << "\"cfgString\" : \"" << fCfgString << "\"}";
  return sstr.str();
}
