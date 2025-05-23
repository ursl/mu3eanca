#include "cdbAbs.hh"
#include "cdbRest.hh"
#include "runRecord.hh"
#include "cdbUtil.hh"
#include "Mu3eConditions.hh"

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
// -- Usage: bin/syncRDB -m mode -f firstRun -l lastRun [-t ../../db1/rest/runInfoTemplate.json] [-h localhost]
// --
// -- Examples: bin/syncRDB -m 2 -g tkar -c cosmic -s significant -h pc11740
// --
// -- -m mode: 0: upload template (magic words: dqTemplate.json or runInfoTemplate.json) to run records
// --          1: parse shift comments and set runInfo fields "class" and "junk"
// --          2: select runs from RDB based on selection string and class string
// -- History:
// --   2025/05/08: first shot
// --   2025/05/12: replace junk with significant
// --   2025/05/20: add mode 2
// 
// ------------------------------------------------------------------------

void rdbMode1(runRecord &, bool);
void rdbMode0(runRecord &, bool);
void rdbMode2(string &, string &, string &, bool);

string runInfoTemplateFile = "../../db1/rest/runInfoTemplate.json";
string rdbUpdateString(":5050/rdb/addAttribute");
vector<string> runInfoTemplateFileLines;

// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {

  // -- command line arguments
  string hostString("pc11740");
  string urlString(":5050/cdb");
  string selectionString("significant"), classString("cosmic"), goodString("");

  bool debug(false);
  int firstRun(0), lastRun(-1), mode(0);
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-d"))    {debug = true;}
    if (!strcmp(argv[i], "-m"))    {mode    = atoi(argv[++i]);}
    // -- run range selection
    if (!strcmp(argv[i], "-f"))    {firstRun = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-l"))    {lastRun = atoi(argv[++i]);}
    // -- stuff
    if (!strcmp(argv[i], "-h"))    {hostString = string(argv[++i]);}
    if (!strcmp(argv[i], "-t"))    {runInfoTemplateFile = string(argv[++i]);}
    // -- selection for runlist output (mode = 2)
    if (!strcmp(argv[i], "-g"))    {goodString = string(argv[++i]);}
    if (!strcmp(argv[i], "-c"))    {classString = string(argv[++i]);}
    if (!strcmp(argv[i], "-s"))    {selectionString = string(argv[++i]);}
  }

  urlString = hostString + urlString; 
  rdbUpdateString = hostString + rdbUpdateString;

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


  cdbAbs *pDB(0);

  pDB = new cdbRest("mcidealv6.1", urlString, 0);
  Mu3eConditions *pDC = Mu3eConditions::instance("mcidealv6.1", pDB);


  if (2 == mode) {
    rdbMode2(selectionString, classString, goodString, debug);
    delete pDB;
    return 0; 
  }


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
    cout << rr.printSummary() << endl;
    return;
  }

  // -- check whether this is about uploading the dataQuality template and whether that exists
  if (string::npos != runInfoTemplateFile.find("dqTemplate")) {
    if (rr.fDataQualityIdx < 0) {
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

  // -- check whether this is about uploading the RunInfo template and whether that exists
  if (string::npos != runInfoTemplateFile.find("runInfoTemplate")) {
    if (rr.fRunInfoIdx < 0) {
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
// -- RunInfo significant:  bad
void rdbMode1(runRecord &rr, bool debug) {

  RunInfo ri = rr.getRunInfo();

  string xstring = rr.fEORComments;
  transform(xstring.begin(), xstring.end(), xstring.begin(), [](unsigned char c) { return tolower(c); });

  string classFromComments = "not found";
  string significantFromComments = "not found";

  cout << "run number: " << rr.fBORRunNumber << ": ";

  // -- check for run class indicators stored in vector
  vector<string> vClassIndicators = {"beam", "source", "cosmic"};
  for (const auto &indicator : vClassIndicators) {
    if (xstring.find(indicator) != string::npos) {
     cout << " found class indicator: " << indicator;
      classFromComments = indicator;
      significantFromComments = "true";
      break;
    }
  }

  // -- modify significantFromComments for special tags
  vector<string> vJunkIndicators = {"bad", "unstable", "error", "problem", "dbx", "fail", "debug", "test", "dummy"};
  for (const auto &indicator : vJunkIndicators) {
    if (xstring.find(indicator) != string::npos) {
      cout << ", overriding junk significant indicator: " << indicator;
      classFromComments = "junk";
      significantFromComments = "false";
      break;
    }
  }

  // -- modify classFromComments for special tags
  if (classFromComments == "not found") {
    vector<string> vClass2Indicators = {"mask", "tune", "tuning", "calib"};
    for (const auto &indicator : vClass2Indicators) {
      if (xstring.find(indicator) != string::npos) {
        cout << ", overriding class2 indicator: " << indicator << " -> " << "calib";
        classFromComments = "calib";
        break;
      }
    }
  }
  cout << endl;

  if (significantFromComments == "not found") {
    significantFromComments = "false";
  } else {
    if (significantFromComments != ri.significant) {
      ri.significant = significantFromComments;
    }
  }
    
  if (classFromComments == "not found") { 
    classFromComments = "junk";
  } else {
    if (ri.Class == "not found") {
      ri.Class = classFromComments;
    } else {
      if (ri.Class != classFromComments) {
        if (ri.comments == "unset") ri.comments = "";
        ri.comments += " from " + ri.Class + " to: " + classFromComments;
        ri.Class = classFromComments;
      }
    }
  }
  
  // -- write to file
  stringstream ss;
  ss << "rdb/runInfo_" << rr.fBORRunNumber << ".json";
  ofstream ofs(ss.str());
  // for (const auto &line : newRunInfo) {
  //   ofs << line << endl;
  // }
  ofs << ri.json() << endl;
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

// ----------------------------------------------------------------------
void rdbMode2(string &selectionString, string &classString, string &goodString, bool debug) {
  Mu3eConditions *pDC = Mu3eConditions::instance();
  vector<string> vRunNumbers = pDC->getAllRunNumbers();
  vector<int> vSelectedRuns;
  for (const auto &runNumber : vRunNumbers) {
    int irun = stoi(runNumber); 
    runRecord rr = pDC->getRunRecord(irun);
    if (!rr.fBOREORValid) continue;
    if (selectionString == "significant") {
      if (rr.isSignificant()) {
        if (classString != "") {
          if (rr.getRunInfoClass() == classString) {
            if (goodString != "") {
              string commentsLower = rr.getRunInfoComments();
              string eorCommentsLower = rr.fEORComments;
              string goodStringLower = goodString;
              transform(commentsLower.begin(), commentsLower.end(), commentsLower.begin(), [](unsigned char c) { return tolower(c); });
              transform(eorCommentsLower.begin(), eorCommentsLower.end(), eorCommentsLower.begin(), [](unsigned char c) { return tolower(c); });
              transform(goodStringLower.begin(), goodStringLower.end(), goodStringLower.begin(), [](unsigned char c) { return tolower(c); });
              if ((string::npos != commentsLower.find(goodStringLower))
                || (string::npos != eorCommentsLower.find(goodStringLower))
              ) {
                cout << "added run number: " << runNumber << " selectionString: " << selectionString << " classString: " << classString << " goodString: " << goodString << endl;
                vSelectedRuns.push_back(irun);
              }
            } else {
              cout << "added run number: " << runNumber << " selectionString: " << selectionString << " classString: " << classString << endl;
              vSelectedRuns.push_back(irun);
            }
          }
        } 
      }
    }
  }

  ofstream ofs("selectedRuns-" + selectionString + (classString != "" ? string("-" + classString) : "")
                               + (goodString != "" ? string("-" + goodString) : "") + ".txt");
  ofs << "{"; 
  for (const auto &irun : vSelectedRuns) {
    ofs << irun;
    if (irun != vSelectedRuns.back()) ofs << ", ";
  }
  ofs << "}" << endl;
  ofs.close();
}
