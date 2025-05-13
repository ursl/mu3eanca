#include "runRecord.hh"
#include "cdbUtil.hh"

#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;


// ----------------------------------------------------------------------
runRecord::runRecord() :
  fDataQualityValid(false), 
  fRunInfoValid(false),
  fBORRunNumber(0),
  fBORStartTime("unset"),
  fBORSubsystems(-9999),
  fBORBeam(-9999.9),
  fBORShiftCrew("unset"),
  fEORStopTime("unset"),
  fEOREvents(0),
  fEORFileSize(0),
  fEORDataSize(0),
  fEORComments("unset"),
  fConfigurationKey("unset"), 
  fDQMu3e(-1),
  fDQBeam(-1),
  fDQVertex(-1),
  fDQPixel(-1),
  fDQTiles(-1),
  fDQFibres(-1),
  fDQCalibration(-1),
  fDQGoodLinks(-1),
  fDQVersion("unset"),
  fRIClass("unset"),
  fRISignificant("unset"),
  fRIComments("unset"),
  fRIComponents("unset"),
  fRIComponentsOut("unset"),
  fRIMidasVersion("unset"),
  fRIMidasGitRevision("unset"),
  fRIDAQVersion("unset"),
  fRIDAQGitRevision("unset"),
  fRIVtxVersion("unset"),
  fRIVtxGitRevision("unset"),
  fRIPixVersion("unset"),
  fRIPixGitRevision("unset"),
  fRITilVersion("unset"),
  fRITilGitRevision("unset"),
  fRIFibVersion("unset"),
  fRIFibGitRevision("unset"),
  fRIVersion("unset")
{

};


// ----------------------------------------------------------------------
void runRecord::corrupted(string jsonString) { 
  cout << "$$$ runRecord corrupted ->" << jsonString << "<-" << endl;
  fDataQualityValid = false; 
  fRunInfoValid = false; 
  fBOREORValid = false; 
}


// ----------------------------------------------------------------------
void runRecord::print() {
  cout << printString() << endl;
}


// ----------------------------------------------------------------------
string runRecord::printString() {
  std::stringstream sstr;
  sstr << "/**/run " << fBORRunNumber
       << ": " << fBORStartTime
      // << ", shift: " << fBORShiftCrew
       << " (" << fEORComments << ")"
       << " nevts: " << fEOREvents
       << " beam: " << fBORBeam
       << " DQ: " << fDataQualityValid
       << " runInfo: " << fRunInfoValid;
  if (fDataQualityValid) sstr << " class: " << fRIClass;
  if (fRunInfoValid) sstr << " significant: " << fRISignificant;
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
  int verbose = 0;
  if (verbose > 1) cout << "curlReadBuffer ->" << curlReadBuffer << "<-" << endl;

  // -- a lot of catching for developing run records and missing fields
  string parseString("");

  // -- get BOR
  parseString = jsonGetValue(curlReadBuffer, vector<string>{"BOR", "Run number"});
  if (parseString == "parseError") {
    corrupted(curlReadBuffer);
    return;
  }
  if (verbose > 9) cout << "  Run number" << endl;
  fBORRunNumber = stoul(parseString);

  parseString = jsonGetValue(curlReadBuffer, vector<string>{"BOR", "Start time"});
  if (parseString == "parseError") {
    corrupted(curlReadBuffer);
    return;
  }
  if (verbose > 9) cout << "  Start time" << endl;
  fBORStartTime = parseString;

  parseString = jsonGetValue(curlReadBuffer, vector<string>{"BOR", "Subsystems"});
  if (parseString == "parseError") {
    corrupted(curlReadBuffer);
    return;
  }
  if (verbose > 9) cout << "  Subsystems ->" << parseString << "<-" << endl;
  fBORSubsystems = stoi(parseString);

  parseString = jsonGetValue(curlReadBuffer, vector<string>{"BOR", "Beam"});
  if (parseString == "parseError") {
    corrupted(curlReadBuffer);
    return;
  }
  if (verbose > 9) cout << "  Beam" << endl;
  fBORBeam = stod(parseString);

  parseString = jsonGetValue(curlReadBuffer, vector<string>{"BOR", "Shift crew"});
  if (parseString == "parseError") {
    corrupted(curlReadBuffer);
    return;
  }
  if (verbose > 9) cout << "  Shift crew" << endl;
  fBORShiftCrew = parseString;

  if (verbose > 2) cout << " after BOR" << endl;

  // -- get EOR
  parseString = jsonGetValue(curlReadBuffer, "Stop time");
  if (parseString == "parseError") {
    corrupted(curlReadBuffer);
    return;
  } 
  fEORStopTime = parseString;

  parseString = jsonGetValue(curlReadBuffer, "Events");
  if (parseString == "parseError") {
    corrupted(curlReadBuffer);
    return;
  }
  fEOREvents = stoul(parseString);

  parseString = jsonGetValue(curlReadBuffer, "File size");
  if (parseString == "parseError") {
    corrupted(curlReadBuffer);
    return;
  }
  fEORFileSize = stod(parseString);

  parseString = jsonGetValue(curlReadBuffer, "Uncompressed data size");
  if (parseString == "parseError") {
    corrupted(curlReadBuffer);
    return;
  }
  fEORDataSize = stod(parseString);

  parseString = jsonGetValue(curlReadBuffer, vector<string>{"EOR", "Comments"});
  if (parseString == "parseError") {
    corrupted(curlReadBuffer);
    return;
  }
  fEORComments = parseString;

  fBOREORValid = true;

  if (verbose > 2) cout << " after EOR" << endl;

  // -- get dataQuality
  string existsDQ = jsonGetValue(curlReadBuffer, vector<string>{"DataQuality", "mu3e"});
  if (existsDQ != "parseError") {
    fDataQualityValid = true;
    if (verbose > 2) cout << " before DQ -> " << existsDQ << "<-" << endl;
    fDQMu3e = stoi(jsonGetValue(curlReadBuffer, "mu3e")); 
    fDQBeam = stoi(jsonGetValue(curlReadBuffer, "beam"));
    fDQVertex = stoi(jsonGetValue(curlReadBuffer, "vertex"));
    fDQPixel = stoi(jsonGetValue(curlReadBuffer, "pixel"));
    fDQTiles = stoi(jsonGetValue(curlReadBuffer, "tiles"));
    fDQFibres = stoi(jsonGetValue(curlReadBuffer, "fibres"));
    fDQCalibration = stoi(jsonGetValue(curlReadBuffer, "calibration"));
    fDQGoodLinks = stoi(jsonGetValue(curlReadBuffer, "goodLinks"));
    fDQVersion = jsonGetString(curlReadBuffer, "version");
  }
  if (verbose > 2) cout << " after DQ" << endl;

  // -- get RunInfo
  string existsRI = jsonGetValue(curlReadBuffer, vector<string>{"RunInfo", "Class"});
  if (existsRI != "parseError") {
    fRunInfoValid = true;
    if (verbose > 2) cout << " before RI" << endl;
    fRIMidasVersion = jsonGetString(curlReadBuffer, "MidasVersion");
    fRIMidasGitRevision = jsonGetString(curlReadBuffer, "MidasGitRevision");
    fRIDAQVersion = jsonGetString(curlReadBuffer, "DAQVersion");
    fRIDAQGitRevision = jsonGetString(curlReadBuffer, "DAQGitRevision");
    fRIVtxVersion = jsonGetString(curlReadBuffer, "VtxVersion");
    fRIVtxGitRevision = jsonGetString(curlReadBuffer, "VtxGitRevision");
    fRIPixVersion = jsonGetString(curlReadBuffer, "PixVersion");
    fRIPixGitRevision = jsonGetString(curlReadBuffer, "PixGitRevision");
    fRITilVersion = jsonGetString(curlReadBuffer, "TilVersion");
    fRITilGitRevision = jsonGetString(curlReadBuffer, "TilGitRevision");
    fRIFibVersion = jsonGetString(curlReadBuffer, "FibVersion");
    fRIFibGitRevision = jsonGetString(curlReadBuffer, "FibGitRevision");

    fRIClass = jsonGetString(curlReadBuffer, "Class");
    fRISignificant = jsonGetString(curlReadBuffer, "Significant");
    fRIComments = jsonGetString(curlReadBuffer, vector<string>{"RunInfo", "Comments"});
  }
}