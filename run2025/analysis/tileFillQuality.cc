#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <vector>
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
// tileFillQuality
// ------------------
// Determine (placeholder/offline) DQM information for tileQuality
//
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
int main(int argc, char *argv[]) {
  cout << "tileFillQuality" << endl;
  // -- command line arguments
  int verbose(0), mode(1), printMode(0), check(0);
  // note: mode = 1 PixelQuality, 2 PixelQualityV, 3 PixelQualityM
  string jsondir(JSONDIR), dirname("");
  string gt("datav6.3=2025V0");
  string igt("datav6.2=2025Beam");
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-c"))      {check = 1;}
    if (!strcmp(argv[i], "-d"))      {dirname = argv[++i];}
    if (!strcmp(argv[i], "-g"))      {gt = argv[++i];}
    if (!strcmp(argv[i], "-i"))      {igt = argv[++i];}
    if (!strcmp(argv[i], "-j"))      {jsondir = argv[++i];}
    if (!strcmp(argv[i], "-p"))      {printMode = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-v"))      {verbose = atoi(argv[++i]);}
  }

  // -- this is just to get the list of all chipIDs
  cdbAbs *pDB = new cdbJSON(igt, jsondir, verbose);
  Mu3eConditions* pDC = Mu3eConditions::instance(igt, pDB);
  pDC->setRunNumber(1);
  if(!pDC->getDB()) {
      std::cout << "CDB database not found" << std::endl;
  }

  if (printMode == 4) {
    // -- create turned on CSV for all chipIDs
    ofstream ofs;
    string filename = Form("csv/run1_quality_overview.json");
    json j;
    j["Run_number"] = 1;

    calAbs* cal = pDC->getCalibration("tilealignment_");
    calTileAlignment* cpa = dynamic_cast<calTileAlignment*>(cal);
    uint32_t i = 0;
    cpa->resetIterator();
    while(cpa->getNextID(i)) {
      uint32_t tileID = cpa->id(i);
      j[to_string(tileID)] = {
        {"tileID", tileID},
        {"Good", 1},
        {"Dead", 0},
        {"Noisy", 0}
      };
    }

    ofs.open(filename);
    ofs << j.dump(2);
    ofs.close();

    string hash = string("tag_tilequality_") + gt + string("_iov_1");
    calTileQuality *ctq = new calTileQuality();
    ctq->readJSON(filename);
    string blob = ctq->makeBLOB();
    string schema = ctq->getSchema();
    string comment = "perfect tile quality for all channels";
    cout << "XXXXXXXXX createPayload with hash = " << hash << " comment = " << comment << " schema = " << schema << endl;
    createPayload(hash, ctq, "./payloads", schema, comment);

    return 0; 
  }

  if (dirname != "") {
    // -- read in all files in dirname
    vector<string> files = getFilesInDirectory(dirname);
    for (auto file : files) {
      int runNumber = stoi(file.substr(file.find("run") + 3, file.find("_quality_overview.json") - file.find("run") - 3));
      cout << "file = " << file << " runNumber = " << runNumber << endl;
      string hash = string("tag_tilequality_") + gt + string("_iov_") + to_string(runNumber);
      calTileQuality *ctq = new calTileQuality();
      ctq->readJSON(file);
      string blob = ctq->makeBLOB();
      string schema = ctq->getSchema();
      string comment = "Elizaveta's tile quality as of 2025/11/18";
      cout << "XXXXXXXXX createPayload with hash = " << hash << " comment = " << comment << " schema = " << schema << endl;
      createPayload(hash, ctq, "./payloads", schema, comment);
      delete ctq;
    }
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
