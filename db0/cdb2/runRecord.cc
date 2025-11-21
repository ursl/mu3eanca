#include "runRecord.hh"
#include "cdbUtil.hh"

#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;


// ----------------------------------------------------------------------
runRecord::runRecord() :
    fBORRunNumber(-1),
    fBORStartTime("unset"),
    fBORShiftCrew("unset"),
    fBORRunClass("unset"),
    fBORMu3eMagnet(-9999.9),
    fBORPixelReadout(false),
    fBORSciFiReadout(false),
    fBORSciTileReadout(false),
    fBORBeamBlockerOpen(false),
    fEORStopTime("unset"),
    fEOREvents(0),
    fEORFileSize(0),
    fEORDataSize(0),
    fEORComments("unset"),
    fConfigurationKey("unset") { }

// ----------------------------------------------------------------------
runRecord::~runRecord() {
  fvDQ.clear();
  fvRI.clear();
}

// ----------------------------------------------------------------------
void runRecord::corrupted(string jsonString) { 
  cout << "$$$ runRecord corrupt ->" << jsonString << "<-" << endl;
  fDataQualityIdx = -1;
  fRunInfoIdx = -1; 
  fBOREORValid = false; 
}

// ---------------------------------------------------------------------- 
bool runRecord::isSignificant() const {
  if (fRunInfoIdx > -1) return fvRI[fRunInfoIdx].significant == "true";
  return false;
}

// ----------------------------------------------------------------------
std::string runRecord::getRunInfoClass() const {
  if (fRunInfoIdx > -1) return fvRI[fRunInfoIdx].Class;
  return "unset";
} 

// ----------------------------------------------------------------------
std::string runRecord::getRunInfoComments() const {
  if (fRunInfoIdx > -1) return fvRI[fRunInfoIdx].comments;
  return "unset";
} 

// ----------------------------------------------------------------------
RunInfo runRecord::getRunInfo() const {
  if (fRunInfoIdx > -1) return fvRI[fRunInfoIdx];
  return RunInfo();
}

// ----------------------------------------------------------------------
DataQuality runRecord::getDQ() const {
  if (fDataQualityIdx > -1) return fvDQ[fDataQualityIdx];
  return DataQuality();
}

// ----------------------------------------------------------------------
void runRecord::print() {
  cout << printSummary() << endl;
}

// ----------------------------------------------------------------------
string runRecord::printSummary() const {
  std::stringstream sstr;
  sstr << "/**/run " << fBORRunNumber
       << ": " << fBORStartTime
      // << ", shift: " << fBORShiftCrew
       << " (" << fEORComments << ")"
       << " nevts: " << fEOREvents
       << " DQ: " << fDataQualityIdx
       << " runInfo: " << fRunInfoIdx;
  if (fDataQualityIdx > -1) sstr << " class: " << fvRI[fDataQualityIdx].Class;
  if (fRunInfoIdx > -1) sstr << " significant: " << fvRI[fRunInfoIdx].significant;
  return sstr.str();
}


// ----------------------------------------------------------------------
string runRecord::json() const {
  std::stringstream sstr;
  sstr << fJSONString;
  return sstr.str();
} 


// ----------------------------------------------------------------------
string runRecord::jsonInterpreted() const {
  std::stringstream sstr;
    
  stringstream ssB;
  ssB << scientific << setprecision(6) << fBORMu3eMagnet;
  stringstream ssF;
  ssF << scientific << setprecision(10) << fEORFileSize;
  stringstream ssD;
  ssD << scientific << setprecision(10) << fEORDataSize;
  
  sstr << "{ \"BOR\" : {"
       << "\"Run number\" : " << fBORRunNumber << ", "
       << "\"Start time\" : \"" << fBORStartTime << "\", "
       << "\"Run Class\" : \"" << fBORRunClass << "\", "
       << "\"Mu3e Magnet\" : " << ssB.str() << ", "
       << "\"Pixel readout\" : " << fBORPixelReadout << ", "
       << "\"SciFi readout\" : " << fBORSciFiReadout << ", "
       << "\"SciTile readout\" : " << fBORSciTileReadout << ", "
       << "\"Beam Blocker Open\" : " << fBORBeamBlockerOpen << ", "
       << "\"Shift crew\" : \"" << fBORShiftCrew << "\""
       << "}, "
       << "\"EOR\" : {"
       << "\"Stop time\" : \"" << fEORStopTime << "\", "
       << "\"Events\" : " << fEOREvents << ", "
       << "\"File size\" : " << ssF.str() << ", "
       << "\"Uncompressed data size\" : " << ssD.str() << ", "
       << "\"Comments\" : \"" << fEORComments << "\" "
       << "}";
       if ((fDataQualityIdx > -1) || (fRunInfoIdx > -1)) {
         sstr << ", \"Attributes\": [";
         if (fDataQualityIdx > -1) {
           for (int i = 0; i <= fDataQualityIdx; i++) {
              sstr << fvDQ[i].json();
              if (i < fDataQualityIdx) sstr << ",";
           }
         }
         if (fRunInfoIdx > -1) {
           for (int i = 0; i <= fRunInfoIdx; i++) {
              sstr << fvRI[i].json();
              if (i < fRunInfoIdx) sstr << ",";
           }
         }
         sstr << "}]" << endl;
      }
  return sstr.str();
}

// ----------------------------------------------------------------------
// -- fill from JSON string
void runRecord::fillFromJson(const std::string &curlReadBuffer) {
  int verbose = 0;
  if (verbose > 1) cout << "curlReadBuffer ->" << curlReadBuffer << "<-" << endl;
  fJSONString = curlReadBuffer;

  // -- a lot of catching for developing run records and missing fields
  string parseString("");

  // -- get BOR
  parseString = jsonGetValue(curlReadBuffer, vector<string>{"BOR", "Run number"});
  if (parseString == "parseError") {
    corrupted(curlReadBuffer);
    return;
  }
  fBORRunNumber = stoul(parseString);
  if (verbose > 9) cout << "  Run number = " << fBORRunNumber << endl;

  parseString = jsonGetValue(curlReadBuffer, vector<string>{"BOR", "Start time"});
  if (parseString == "parseError") {
    corrupted(curlReadBuffer);
    return;
  }
  fBORStartTime = parseString;
  if (verbose > 9) cout << "  Start time = " << fBORStartTime << endl;

  parseString = jsonGetValue(curlReadBuffer, vector<string>{"BOR", "Shift crew"});
  if (parseString == "parseError") {
    corrupted(curlReadBuffer);
    return;
  }
  fBORShiftCrew = parseString;
  if (verbose > 9) cout << "  Shift crew = " << fBORShiftCrew << endl;

  parseString = jsonGetValue(curlReadBuffer, vector<string>{"BOR", "Run Class"});
  if (parseString == "parseError") {
    corrupted(curlReadBuffer);
    return;
  }
  fBORRunClass = parseString;
  if (verbose > 9) cout << "  Run Class = " << fBORRunClass << endl;

  if (verbose > 2) cout << " after BOR" << endl;

  // -- get EOR
  parseString = jsonGetValue(curlReadBuffer, "Stop time");
  if (parseString == "parseError") {
    corrupted(curlReadBuffer);
    return;
  } 
  fEORStopTime = parseString;
  parseString = jsonGetValue(curlReadBuffer, "Events");
  if (verbose > 9) cout << "  Stop time = " << fEORStopTime << endl;
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
    fDataQualityIdx = -1;
    size_t pos = 1;
    if (verbose > 2) cout << " before DQ" << endl;
    while (pos != string::npos) {
      ++fDataQualityIdx;
      fvDQ.push_back(DataQuality());
      pos = fvDQ[fDataQualityIdx].parse(curlReadBuffer, pos);
      // -- DataQuality is only filled validly if the returned pos is valid
      if (pos == string::npos) {
        --fDataQualityIdx;
        fvDQ.pop_back();
      }
      if (verbose > 2) cout << " DQ " << fDataQualityIdx << " " << fvDQ[fDataQualityIdx].print() << endl;
    }
  }
 
  // -- get RunInfo
  string existsRI = jsonGetValue(curlReadBuffer, vector<string>{"RunInfo", "Class"});
  if (existsRI != "parseError") {
    fRunInfoIdx = -1;
    size_t pos = 1;
    if (verbose > 2) cout << " before RI" << endl;
    while (pos != string::npos) {
      ++fRunInfoIdx;
      fvRI.push_back(RunInfo());
      pos = fvRI[fRunInfoIdx].parse(curlReadBuffer, pos);
      // -- RunInfo is only filled validly if the returned pos is valid
      if (pos == string::npos) {
        --fRunInfoIdx;
        fvRI.pop_back();
      }
      if (verbose > 2) cout << " RI " << fRunInfoIdx << " " << fvRI[fRunInfoIdx].print() << endl;
    }
  }
}