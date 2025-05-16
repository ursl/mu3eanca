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
    fBORSubsystems(-9999),
    fBORBeam(-9999.9),
    fBORShiftCrew("unset"),
    fEORStopTime("unset"),
    fEOREvents(0),
    fEORFileSize(0),
    fEORDataSize(0),
    fEORComments("unset"),
    fConfigurationKey("unset") { }

runRecord::~runRecord() {
  fDQ.clear();
  fRI.clear();
}

// ----------------------------------------------------------------------
void runRecord::corrupted(string jsonString) { 
  cout << "$$$ runRecord corrupted ->" << jsonString << "<-" << endl;
  fDataQualityIdx = -1;
  fRunInfoIdx = -1; 
  fBOREORValid = false; 
}

// ---------------------------------------------------------------------- 
bool runRecord::isSignificant() const {
  if (fRunInfoIdx > -1) return fRI[0].significant == "true";
  return false;
}

// ----------------------------------------------------------------------
std::string runRecord::getRunInfoClass() const {
  if (fRunInfoIdx > -1) return fRI[0].Class;
  return "unset";
} 

// ----------------------------------------------------------------------
std::string runRecord::getRunInfoComments() const {
  if (fRunInfoIdx > -1) return fRI[0].comments;
  return "unset";
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
       << " beam: " << fBORBeam
       << " DQ: " << fDataQualityIdx
       << " runInfo: " << fRunInfoIdx;
  if (fDataQualityIdx > -1) sstr << " class: " << fRI[fDataQualityIdx].Class;
  if (fRunInfoIdx > -1) sstr << " significant: " << fRI[fRunInfoIdx].significant;
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
       << "}";
       if ((fDataQualityIdx > -1) || (fRunInfoIdx > -1)) {
         sstr << ", \"Attributes\": [";
         if (fDataQualityIdx > -1) {
           for (int i = 0; i <= fDataQualityIdx; i++) {
              sstr << fDQ[i].json();
              if (i < fDataQualityIdx) sstr << ",";
           }
         }
         if (fRunInfoIdx > -1) {
           for (int i = 0; i <= fRunInfoIdx; i++) {
              sstr << fRI[i].json();
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
    fDataQualityIdx = -1;
    size_t pos = 1;
    if (verbose > 2) cout << " before DQ" << endl;
    while (pos != string::npos) {
      ++fDataQualityIdx;
      fDQ.push_back(DataQuality());
      pos = fDQ[fDataQualityIdx].parse(curlReadBuffer, pos);
      if (verbose > 2) cout << " DQ " << fDataQualityIdx << " " << fDQ[fDataQualityIdx].print() << endl;
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
      fRI.push_back(RunInfo());
      pos = fRI[fRunInfoIdx].parse(curlReadBuffer, pos);
      if (verbose > 2) cout << " RI " << fRunInfoIdx << " " << fRI[fRunInfoIdx].print() << endl;
    }
  }
}