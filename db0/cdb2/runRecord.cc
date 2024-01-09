#include "runRecord.hh"

#include <iostream>
#include <sstream>

using namespace std;


// ----------------------------------------------------------------------
runRecord::runRecord() :
  fRun(-9999),
  fRunStart("unset"), fRunEnd("unset"),
  fRunDescription("unset"), fRunOperators("unset"),
  fNFrames(-9999), fBeamMode(-9999),
  fBeamCurrent(-9999.),
  fMagnetCurrent(-9999.),
  fConfigurationKey("unset")
{
  
};


// ----------------------------------------------------------------------
void runRecord::print() {
  cout << printString() << endl;
}


// ----------------------------------------------------------------------
string runRecord::printString() {
  std::stringstream sstr;
  sstr << "/**/run ->" << fRun
       << "<- /**/start ->" << fRunStart
       << "<- /**/end ->" << fRunEnd
       << "<- /**/desc->" << fRunDescription
       << "<- /**/ops ->" << fRunOperators
       << "<-";
  return sstr.str();
}


// ----------------------------------------------------------------------
string runRecord::json() const {
  std::stringstream sstr;
  sstr << "{\"run\" : \"" << fRun << "\", "
       << "\"runStart\" : \"" << fRunStart << "\", "
       << "\"runEnd\" : \"" << fRunEnd << "\", "
       << "\"runDescription\" : \"" << fRunDescription << "\", "
       << "\"runOperators\" : \"" << fRunOperators << "\", "
       << "\"nFrames\" : \"" << fNFrames << "\", "
       << "\"beamMode\" : \"" << fBeamMode << "\", "
       << "\"beamCurrent\" : \"" << fBeamCurrent << "\", "
       << "\"magnetCurrent\" : \"" << fMagnetCurrent << "\", "
       << "\"configurationKey\" : \"" << fConfigurationKey
       << "\"}";
  return sstr.str();
}
