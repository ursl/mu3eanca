#include "runRecord.hh"

#include <iostream>
#include <sstream>

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
    "Run number" : 1002,
    "Start time" : "Thu Jan 11 09:40:41 2024",
    "Subsystems" : 0,
    "Beam" : 0,
    "Shift crew" : "Urs & Nik"
    },
    "EOR": {
    "Stop time" : "Thu Jan 11 09:41:06 2024",
    "Events" : 2,
    "File size" : 172794,
    "Data size" : 1709318,
    "Comments" : "Just a test of a test"
    }  
    }
  */

  sstr << "{ \"BOR\" : {"
       << "\"Run number\" : " << fBORRunNumber << ", "
       << "\"Start time\" : \"" << fBORStartTime << "\", " 
       << "\"Subsystems\" : " << fBORSubsystems << ", " 
       << "\"Beam\" : " << fBORBeam << ", " 
       << "\"Shift crew\" : \"" << fBORShiftCrew << "\""
       << "}, "

       << "\"EOR\" : {"
       << "\"Stop time\" : \"" << fEORStopTime << "\", " 
       << "\"Events\" : " << fEOREvents << ", " 
       << "\"File size\" : " << fEORFileSize << ", " 
       << "\"Data size\" : " << fEORDataSize << ", " 
       << "\"Comments\" : \"" << fEORComments << "\" " 
       << "} }";
  return sstr.str();
}
