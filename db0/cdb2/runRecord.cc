#include "runRecord.hh"
#include "cdbUtil.hh"

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
       << ": " << fBORStartTime
       << ", shift: " << fBORShiftCrew
       << " (" << fEORComments << ")"
       << " nevts: " << fEOREvents
       << " beam: " << fBORBeam
       << " class: " << fRIClass
       << " junk: " << fRIJunk
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

// ----------------------------------------------------------------------
// -- fill from JSON string
void runRecord::fillFromJson(const std::string &curlReadBuffer) {
  if (0) cout << "curlReadBuffer ->" << curlReadBuffer << "<-" << endl;

  // -- get BOR
  fBORRunNumber = stoi(jsonGetValue(curlReadBuffer, "Run number"));
  fBORStartTime = jsonGetString(curlReadBuffer, "Start time");
  fBORSubsystems = stoi(jsonGetValue(curlReadBuffer, "Subsystems"));
  fBORBeam = stof(jsonGetValue(curlReadBuffer, "Beam"));
  fBORShiftCrew = jsonGetString(curlReadBuffer, "Shift crew");

  // -- get EOR
  fEORStopTime = jsonGetString(curlReadBuffer, "Stop time");
  fEOREvents = stoi(jsonGetValue(curlReadBuffer, "Events"));
  fEORFileSize = stod(jsonGetValue(curlReadBuffer, "File size"));
  fEORDataSize = stod(jsonGetValue(curlReadBuffer, "Uncompressed data size"));
  fEORComments = jsonGetString(curlReadBuffer, vector<string>{"EOR", "Comments"});

  // -- get RunInfo
  fRIClass = jsonGetString(curlReadBuffer, "Class");
  fRIJunk = jsonGetString(curlReadBuffer, "Junk");
  fRIComments = jsonGetString(curlReadBuffer, vector<string>{"RunInfo", "Comments"});
}