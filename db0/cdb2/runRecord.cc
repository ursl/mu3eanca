#include "runRecord.hh"

#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;


// ----------------------------------------------------------------------
runRecord::runRecord() :
  fBORRunNumber(-9999),
  fBORStartTime("unset"),
  fBORSubsystems(-9999),
  fBORBeam(-9999.9),
  fBORShiftCrew("unset"),
  fEORStopTime("unset"),
  fEOREvents(0),
  fEORFileSize(0),
  fEORDataSize(0),
  fEORComments("unset"),
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
  sstr << "/**/run " << fBORRunNumber
       << " (" << fBORStartTime
       << " .. " << fEORStopTime
       << ") desc: " << fEORComments
       << ", shift: " << fBORShiftCrew
       << ", nevts: " << fEOREvents
       << ", beam( " << fBORBeam
       << "), comments: " << fEORComments
       << "), cfgkey(" << fConfigurationKey
       << ")"
       ;
  return sstr.str();
}


// ----------------------------------------------------------------------
string runRecord::json() const {
  std::stringstream sstr;
  /*
    {
    "BOR": {
    "Run number" : 5017,
    "Start time" : "Thu Jan 18 04:48:51 2024",
    "Subsystems" : 0,
    "Beam" : 0,
    "Shift crew" : "The data challenge crew"
    },
    "EOR": {
    "Stop time" : "Thu Jan 18 04:49:44 2024",
    "Events" : 2587814,
    "File size" : 4.0360677850000000e+09,
    "Uncompressed data size" : 5.1365311020000000e+09,
    "Comments" : "Test data from the data challenge"
    }
    }
  */
  
  stringstream ssB;
  ssB << scientific << setprecision(6) << fBORBeam;
  stringstream ssF;
  ssF << scientific << setprecision(10) << fEORFileSize;
  stringstream ssD;
  ssD << scientific << setprecision(10) << fEORDataSize;
  
  sstr << "{ \"BOR\" : {"
       << "\"Run number\" : " << fBORRunNumber << ", "
       << "\"Start time\" : \"" << fBORStartTime << "\", "
       << "\"Subsystems\" : " << fBORSubsystems << ", "
       << "\"Beam\" : " << ssB.str() << ", "
       << "\"Shift crew\" : \"" << fBORShiftCrew << "\""
       << "}, "
       
       << "\"EOR\" : {"
       << "\"Stop time\" : \"" << fEORStopTime << "\", "
       << "\"Events\" : " << fEOREvents << ", "
       << "\"File size\" : " << ssF.str() << ", "
       << "\"Uncompressed data size\" : " << ssD.str() << ", "
       << "\"Comments\" : \"" << fEORComments << "\" "
       << "} }";
  return sstr.str();
}
