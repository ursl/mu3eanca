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
void payload::print() {
  cout << printString() << endl;
}


// ----------------------------------------------------------------------
string payload::printString() {
  std::stringstream sstr;
  sstr << "/**/hash ->" << fHash
       << "<- /**/comment ->" << fComment
       << "<- /**/schema ->" << fSchema
       << "<- /**/date ->" << fDate
       << "<- /**/BLOB ->" << fBLOB
       << "<-";
  return sstr.str();
}


// ----------------------------------------------------------------------
string payload::json() {
  std::stringstream sstr;
  sstr << "{\"hash\" : \"" << fHash << "\", "
       << "\"comment\" : \"" << fComment << "\","
       << "\"schema\" : \"" << fSchema << "\","
       << "\"date\" : \"" << fDate << "\","
       << "\"BLOB\" : \"" << base64_encode(fBLOB) << "\"}";
  return sstr.str();
}
