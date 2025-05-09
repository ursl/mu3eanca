#include "cdbRest.hh"
#include "runRecord.hh"
#include "cdbUtil.hh"

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string.h>
#include <dirent.h>  /// for directory reading
#include <sys/time.h>
#include <unistd.h>
#include <cctype> // for toupper()
#include <algorithm> // for transform()


using namespace std;
// ----------------------------------------------------------------------
// -- syncRunDB
// -- ---------
// -- 
// -- produce updates to either DataQuality or RunInfo attributes of run records in RDB
// -- updates a mongoDB server, does not run on the JSON backend server
// --
// -- Usage: bin/syncRunDB -m mode -f firstRun -l lastRun [-t ../../db1/rest/runInfoTemplate.json] [-u localhost:5050/rdb/addAttribute/]
// --
// -- -m mode: 0: upload template (magic words: dqTemplate.json or runInfoTemplate.json) to run records
// --          1: parse shift comments and set runInfo fields "class" and "junk"
// -- History:
// --   2025/05/08: first shot
// ------------------------------------------------------------------------

void rdbMode1(runRecord &, bool);
void rdbMode0(runRecord &, bool);


string runInfoTemplateFile = "../../db1/rest/runInfoTemplate.json";
string rdbUpdateString("localhost:5050/rdb/addAttribute");
vector<string> runInfoTemplateFileLines;

// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {

  // -- command line arguments
  string urlString("localhost:5050/cdb");

  bool debug(false);
  int firstRun(0), lastRun(-1), mode(0);
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-d"))   {debug = true;}
    if (!strcmp(argv[i], "-f"))   {firstRun = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-l"))   {lastRun = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-m"))    {mode    = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-t"))    {runInfoTemplateFile = string(argv[++i]);}
    if (!strcmp(argv[i], "-u"))    {urlString = string(argv[++i]);}
  }

  // -- read in template
  ifstream file(runInfoTemplateFile);
  if (file.is_open()) {
    string line;
    while (getline(file, line)) {
      runInfoTemplateFileLines.push_back(line);
    }
    file.close();
  } else {
    cerr << "Error: Unable to open file " << runInfoTemplateFile << endl;
  }


  cdbRest *pDB(0);

  pDB = new cdbRest("mcidealv6.1", urlString, 0);

  vector<string> vRunNumbers = pDB->getAllRunNumbers();
  for (int it = 0; it < vRunNumbers.size(); ++it) {
    int irun = stoi(vRunNumbers[it]);
    if (irun < firstRun) continue;
    if ((lastRun > 0) && (irun > lastRun)) continue;
    
    runRecord rr = pDB->getRunRecord(irun);
    if (0 == mode) rdbMode0(rr, debug);
    if (1 == mode) rdbMode1(rr, debug);
  }
  delete pDB; 
} 

// ----------------------------------------------------------------------
// -- upload template {runInfo,dataQuality} to run records 
// -- IFF they do not yet contain that attribute
void rdbMode0(runRecord &rr, bool debug) {
  if (!rr.fBOREORValid) {
    cout << "incomplete run record in RDB, skipping ................................. " << endl;
    cout << rr.printString() << endl;
    return;
  }

  // -- check whether this is about uploading the dataQuality template and whether that exists
  if (string::npos != runInfoTemplateFile.find("dqTemplate")) {
    if (!rr.fDataQualityValid) {
      cout << "no DataQuality attribute found for run number: " << rr.fBORRunNumber << endl;
      int irun = rr.fBORRunNumber;
      stringstream ss;
      ss << "curl -X PUT -H \"Content-Type: application/json\" --data-binary @" << runInfoTemplateFile << " " << rdbUpdateString << "/" << irun;
      cout << "Updating for run number: " << irun << endl; 
      cout << ss.str() << endl;      
      if (!debug) {
        system(ss.str().c_str());
      }
    }
  }

  // -- check whether this is about uploading the dataQuality template and whether that exists
  if (string::npos != runInfoTemplateFile.find("runInfoTemplate")) {
    if (!rr.fRunInfoValid) {
      cout << "no runInfo attribute found for run number: " << rr.fBORRunNumber << endl;
      int irun = rr.fBORRunNumber;
      stringstream ss;
      ss << "curl -X PUT -H \"Content-Type: application/json\" --data-binary @" << runInfoTemplateFile << " " << rdbUpdateString << "/" << irun;
      cout << "Updating for run number: " << irun << endl; 
      cout << ss.str() << endl;      
      if (!debug) {
        system(ss.str().c_str());
      }
    }
  }
}

// ----------------------------------------------------------------------
// -- set the following flags by parsing the shift comments
// -- RunInfo class: source/cosmic/daq
// -- RunInfo junk:  bad
void rdbMode1(runRecord &rr, bool debug) {

  vector<string> newRunInfo = runInfoTemplateFileLines;
  
  string xstring = rr.fEORComments;
  transform(xstring.begin(), xstring.end(), xstring.begin(), [](unsigned char c) { return tolower(c); });

  string classFromComments = "not found";
  string junkFromComments = "not found";

  cout << "run number: " << rr.fBORRunNumber << ": ";

  // -- check for junk indicators stored in vector
  vector<string> vClassIndicators = {"beam", "source", "cosmics", "daq", "calibration", "tuning"};
  for (const auto &indicator : vClassIndicators) {
    if (xstring.find(indicator) != string::npos) {
     cout << " found class indicator: " << indicator;
      classFromComments = indicator;
      if (indicator == "daq") {
        junkFromComments = "true";
      } else if (indicator == "calibration") {
        junkFromComments = "true";
      } else if (indicator == "tuning") {
        junkFromComments = "true";
      }
      break;
    }
  }

  // -- check for junk indicators stored in vector
  if (junkFromComments == "not found") {
    vector<string> vJunkIndicators = {"bad", "error", "problem", "dbx", "fail", "debug", "test", "dummy"};
    for (const auto &indicator : vJunkIndicators) {
      if (xstring.find(indicator) != string::npos) {
      cout << " and junk indicator: " << indicator;
        junkFromComments = "true";
        break;
      }
    }
  } else {
    cout << " and junk indicator: " << junkFromComments;
  }
  cout << endl;

  if (junkFromComments == "not found") {
    junkFromComments = "false";
  }

  for (auto &it: newRunInfo) {
    // -- replace Class value
    if (it.find("Class") != string::npos) {
      size_t pos =  it.rfind(":");
      if (string::npos != pos) {
        stringstream ss;
        ss << " \"" << classFromComments << "\",";
        it.replace(pos + 1, string::npos, ss.str());
      }
    }
    // -- replace Junk value
    if (it.find("Junk") != string::npos) {
      size_t pos =  it.rfind(":");
      if (string::npos != pos) {
        stringstream ss;
        ss << " \"" << junkFromComments << "\",";
        it.replace(pos + 1, string::npos, ss.str());
      }
    }
  }

  // -- write to file
  stringstream ss;
  ss << "rdb/runInfo_" << rr.fBORRunNumber << ".json";
  ofstream ofs(ss.str());
  for (const auto &line : newRunInfo) {
    ofs << line << endl;
  }
  ofs.close();

  if (!debug) {
    // curl -X PUT -H "Content-Type: application/json" --data-binary @/Users/ursl/mu3e/mu3eanca/db1/rest/runInfoTemplate.json http://localhost:5050/rdb/addAttribute/513
    int irun = rr.fBORRunNumber;
    stringstream ss;
    ss << "curl -X PUT -H \"Content-Type: application/json\" --data-binary @rdb/runInfo_" << irun << ".json " << rdbUpdateString << "/" << irun;
    cout << "Updating for run number: " << irun << endl; 
    cout << ss.str() << endl;      
    system(ss.str().c_str());
  }
}