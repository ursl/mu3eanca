#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <map>
#include <set>
#include <string.h>
#include <chrono>
#include <algorithm> // for std::lower_bound
#include <dirent.h>
#include <sys/stat.h>

#include <fstream>

#include "Mu3eConditions.hh"
#include "calTileAlignment.hh"
#include "calTileQuality.hh"

#include <nlohmann/json.hpp>
using json = nlohmann::ordered_json;

#include "cdbJSON.hh"
#include "base64.hh"

#include "TCanvas.h"
#include "TStyle.h"
#include "TFile.h"
#include "TTree.h"
#include "TH2F.h"
#include "TMath.h"
#include "TKey.h"
#include "TROOT.h"

using namespace std;

// ----------------------------------------------------------------------
// fillFibresQuality
// ------------------
// Two tasks:
// 1. parse FIBRE JSON file into CSV files per run
// 2. create payloads for each run
// 
// Examples:
// ---------
// 
// ----------------------------------------------------------------------

#define JSONDIR "/Users/ursl/data/mu3e/cdb"


// ----------------------------------------------------------------------
void createPayload(string, calAbs *, string, string, string);

// ----------------------------------------------------------------------
vector<string> getFilesInDirectory(const string& dirname) {
  vector<string> files;
  DIR *dir = opendir(dirname.c_str());
  if (dir == nullptr) {
    cerr << "Error: Cannot open directory " << dirname << endl;
    return files;
  }
  
  struct dirent *entry;
  while ((entry = readdir(dir)) != nullptr) {
    // Skip . and ..
    if (entry->d_name[0] == '.') {
      continue;
    }
    
    string fullpath = dirname + "/" + string(entry->d_name);
    struct stat path_stat;
    if (stat(fullpath.c_str(), &path_stat) == 0) {
      // Only add regular files, not directories
      if (S_ISREG(path_stat.st_mode)) {
        if (fullpath.find("_quality_overview.json") != string::npos) {
          files.push_back(fullpath);
        }
      }
    }
  }
  closedir(dir);
  sort(files.begin(), files.end());
  return files;
}

// ----------------------------------------------------------------------
void parseFibreJSON(string filename, string dirname) {
  cout << "parseFibreJSON with filename = " << filename << " and dirname = " << dirname << endl;
  // -- read in the JSON file
  json j;
  ifstream ifs(filename);
  ifs >> j;
  ifs.close();
  cout << "j = " << j.dump(2) << endl;
  
  // -- Invert: collect data by run number (currently organized by ASIC ID with run ranges)
  // Structure: {"8": [[3481, 3498, 14.9375, true], [3499, 3499, 14.9375, false], ...]}
  // Each array: [startRun, endRun, threshold, lock]
  
  // First pass: collect all run numbers and their ASIC data
  map<int, map<int, pair<bool, double>>> runsData; // run -> asicID -> (lock, threshold)
  set<int> allRuns;
  
  for (auto asicItem : j.items()) {
    string asicIDStr = asicItem.key();
    // Skip special keys like "Run_number"
    if (asicIDStr == "Run_number") continue;
    
    int asicID = stoi(asicIDStr);
    
    if (asicItem.value().is_array()) {
      // For each run range in this ASIC
      for (auto rangeArray : asicItem.value()) {
        if (rangeArray.is_array() && rangeArray.size() >= 4) {
          int startRun = rangeArray[0].get<int>();
          int endRun = rangeArray[1].get<int>();
          double threshold = rangeArray[2].get<double>();
          bool lock = rangeArray[3].get<bool>();
          
          // Expand the run range to individual runs
          for (int run = startRun; run <= endRun; ++run) {
            allRuns.insert(run);
            runsData[run][asicID] = make_pair(lock, threshold);
          }
        }
      }
    }
  }
  
  // -- Write CSV files per run
  for (int runNumber : allRuns) {
    string csvfilename = dirname + "/fibre-asics-" + to_string(runNumber) + ".csv";
    
    ofstream ofs;
    ofs.open(csvfilename);
    
    // Write header: #ASIC ID, lock, has_data, threshold, efficiency
    ofs << "#ASIC ID, lock, has_data, threshold, efficiency" << endl;
    
    map<int, pair<bool, double>>& runAsics = runsData[runNumber];
    
    // Write one line per ASIC with data for this run
    // Sort ASIC IDs for consistent output
    vector<int> asicIDs;
    for (auto& entry : runAsics) {
      asicIDs.push_back(entry.first);
    }
    sort(asicIDs.begin(), asicIDs.end());
    
    for (int asicID : asicIDs) {
      bool lockBool = runAsics[asicID].first;
      double threshold = runAsics[asicID].second;
      
      // If lock is true, set lock=1 and has_data=1, otherwise both are 0
      int lock = lockBool ? 1 : 0;
      int hasData = lockBool ? 1 : 0;
      
      // Efficiency is 0 if both lock and has_data are 0, otherwise 1
      int efficiency = (lock == 0 && hasData == 0) ? 0 : 1;
      
      ofs << asicID << ", " << lock << ", " << hasData << ", " << threshold << ", " << efficiency << endl;
    }
    
    ofs.close();
    cout << "   -> wrote CSV file " << csvfilename << " for run " << runNumber << " with " << asicIDs.size() << " ASICs" << endl;
  }
}

// ----------------------------------------------------------------------
int main(int argc, char *argv[]) {
  cout << "tileFillQuality" << endl;
  // -- command line arguments
  int verbose(0), mode(1), printMode(0), check(0);
  string jsondir(JSONDIR), dirname("./"), filename("");
  string gt("datav6.3=2025V0");
  string igt("datav6.2=2025Beam");
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-f"))      {filename = argv[++i];}
    if (!strcmp(argv[i], "-d"))      {dirname = argv[++i];}
    if (!strcmp(argv[i], "-g"))      {gt = argv[++i];}
    if (!strcmp(argv[i], "-i"))      {igt = argv[++i];}
    if (!strcmp(argv[i], "-j"))      {jsondir = argv[++i];}
    if (!strcmp(argv[i], "-p"))      {printMode = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-v"))      {verbose = atoi(argv[++i]);}
  }


  if (filename != "") {
    // -- parse the JSON file into CSV files per run
    parseFibreJSON(filename, dirname);
  }

  return 0;
}


// ----------------------------------------------------------------------
void createPayload(string hash, calAbs *cal, string dirname, string schema, string comment) {
  cout << "createPayload with hash = " << hash << " comment = " << comment << " schema = " << schema << endl;
  payload pl;
  pl.fHash = hash;
  pl.fComment = comment;
  pl.fSchema = schema;
  pl.fBLOB = cal->makeBLOB();
  cal->writePayloadToFile(hash, dirname, pl);
}
