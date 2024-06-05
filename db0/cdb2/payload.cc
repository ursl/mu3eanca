#include "payload.hh"
#include "base64.hh"

#include <iostream>
#include <sstream>

using namespace std;


// ----------------------------------------------------------------------
payload::payload() : fHash("tag_X_iov_Y"),
  fComment("now what?"), fSchema("define this"),
  fDate("insertion date"),
  fBLOB(std::string("empty payload")) {
  
};


// ----------------------------------------------------------------------
void payload::print(bool prtAll) {
  cout << printString(prtAll) << endl;
}


// ----------------------------------------------------------------------
string payload::printString(bool prtAll) {
  std::stringstream sstr;
  sstr << "/**/payload hash ->" << fHash << endl
       << "<- /**/comment ->" << fComment << endl
       << "<- /**/schema ->" << fSchema << endl
       << "<- /**/date ->" << fDate << endl
       << "<- /**/base64_encoded(BLOB) ->"
       << (prtAll? base64_encode(fBLOB) : " BLOB printing suppressed")
       << "<-";
  return sstr.str();
}


// ----------------------------------------------------------------------
string payload::json() {
  std::stringstream sstr;
  sstr << "{\"hash\" : \"" << fHash << "\", "
       << "\"comment\" : \"" << fComment << "\", "
       << "\"schema\" : \"" << fSchema << "\", "
       << "\"date\" : \"" << fDate << "\", "
       << "\"BLOB\" : \"" << base64_encode(fBLOB) << "\"}";
  return sstr.str();
}
