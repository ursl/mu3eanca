#include <iostream>
#include <string.h>
#include <stdio.h>

#include <fstream>
#include <vector>
#include <sstream>
#include <dirent.h>  /// for directory reading
#include <iomanip>

#include <chrono>
#include <glob.h>

#include "cdbUtil.hh"
#include "base64.hh"

#include "calPixelAlignment.hh"
#include "calFibreAlignment.hh"
#include "calMppcAlignment.hh"
#include "calTileAlignment.hh"
#include "calPixelCablingMap.hh"
#include "calPixelQualityLM.hh"

#include "calDetConfV1.hh" // decrepit!
#include "calDetSetupV1.hh"

using namespace std;

// Forward declarations
void writeDetSetupV1(string jsondir, string gt);
void writePixelQualityLM(string jsondir, string gt, string payloaddir);
void writeAlignmentInformation(string jsondir, string gt, string alignmentTag);
void writeInitialTag(string jsondir, string gt, string initialTag);
void insertIovTag(const std::string &jsondir, const std::string &tag,
                  int insertRun /*-i*/, int removeRun /*-r*/, bool clear /*-c*/);

// ----------------------------------------------------------------------
// cdbInitGT -g datav6.2=2025DataV0 -j CDBJSONDIR -p payloadDir
// ---------
//
//
// Usage examples
// --------------
//
// -- create a global tag with all tags/payloads 
// merlin> ./bin/cdbInitGT -g v6.3=2025DataV0 -j ~/data/mu3e/cdb -p ~/data/tmp/cdb/payloads
//
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
int main(int argc, const char* argv[]) {

  // ----------------------------------------------------------------------
  // -- global tags
  // ----------------------------------------------------------------------
  map<string, vector<string>> iniGlobalTags = {
    {"datav6.3=2025V0", {"pixelalignment_", 
                         "fibrealignment_", 
                         "tilealignment_", 
                         "mppcalignment_", 
                         "pixelqualitylm_", 
                         "detsetupv1_"
                        }  
    }
  };    
   
  // -- complete the tags by replacing trailing _ with the _GT
  for (auto &it: iniGlobalTags) {
    for (unsigned int i = 0; i < it.second.size(); i++) { 
      if (it.second[i].back() == '_') {
        it.second[i] = it.second[i] + it.first;
      }
    }
  } 
  

  // -- command line arguments
  string jsondir("");
  string gt("");
  string payloaddir("");
  int verbose(0);
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-j"))  {jsondir    = argv[++i];}
    if (!strcmp(argv[i], "-g"))  {gt         = argv[++i];}
    if (!strcmp(argv[i], "-p"))  {payloaddir = argv[++i];}
    if (!strcmp(argv[i], "-v"))  {verbose    = 1;}
  }
  
  cout << "===============" << endl;
  cout << "== cdbInitGT ==" << endl;
  cout << "===============" << endl;
  cout << "== installing in directory " << jsondir << endl;
  cout << "== global tag " << gt << endl;
  cout << "== payload directory " << payloaddir << endl;
    
  // -- check whether directories for JSONs already exist
  vector<string> testdirs{jsondir,
                          jsondir + "/globaltags",
                          jsondir + "/tags",
                          jsondir + "/payloads",
                          jsondir + "/runrecords",
                          jsondir + "/configs",
                         };
  for (auto it: testdirs) {
    DIR *folder = opendir(it.c_str());
    if (folder == NULL) {
      cout << "creating " << it << endl;
      system(string("mkdir -p " + it).c_str());
    } else {
      closedir(folder);
    }
  }
  
  ofstream JS;
  
  string jdir  = jsondir + "/globaltags";
  
  for (auto igt : iniGlobalTags) {
    if (igt.first != gt) continue;
    
    vector<string> arrayBuilder;
    for (auto it : igt.second) {
      string tag = ('_' == it.back()? it + igt.first: it);
      arrayBuilder.push_back(tag);
    }
    stringstream sstr;
    sstr << "{ \"gt\" : \"" << igt.first << "\", \"tags\" : ";
    sstr << jsFormat(arrayBuilder);
    sstr << " }" << endl;
    
    
    // -- JSON
    JS.open(jdir + "/" + igt.first);
    if (JS.fail()) {
      cout << "Error failed to open " << jdir << "/" << igt.first << endl;
    }
    JS << sstr.str();
    cout << sstr.str();
    JS.close();
  }

  // -- write alignment information payloads and tags
  writeAlignmentInformation(jsondir, gt, "datav6.3=2025V0");
  // -- write detsetupv1 (basically magnet status) payloads and tags
  writeDetSetupV1(jsondir, gt);
  // -- write pixelqualitylm payloads and tags
  writePixelQualityLM(jsondir, gt, payloaddir);

  return 0;
}

// ----------------------------------------------------------------------
void writeDetSetupV1(string jsondir, string gt) {
  cout << "   ->cdbInitGT> writing local template detsetupv1 payloads" << endl;
  // -- create (local template) payloads for no field and with field
  calDetSetupV1 *cdc = new calDetSetupV1();
  string filename = string(LOCALDIR) + "/ascii/detector-MagnetOff-v6.2.json";
  string result = cdc->readJSON(filename);
  if (string::npos == result.find("Error")) {
    string spl = cdc->makeBLOB();
    string hash = "detsetupv1_noField";
    payload pl;
    if (fileExists(string(LOCALDIR) + "/" + hash)) {
      cout << "   ->cdbInitGT> payload " << hash << " already exists, skipping" << endl;
    } else {
      pl.fHash = hash;
      pl.fComment = "detector setup with magnet off (no magnet)";
      pl.fSchema  = cdc->getSchema();
      pl.fBLOB = spl;
      cdc->writePayloadToFile(hash, string(LOCALDIR), pl);
    }
  }

  filename = string(LOCALDIR) + "/ascii/detector-MagnetOn-v6.2.json";
  result = cdc->readJSON(filename);
  if (string::npos == result.find("Error")) {
    string spl = cdc->makeBLOB();
    string hash = "detsetupv1_MagnetOn";
    payload pl;
    if (fileExists(string(LOCALDIR) + "/" + hash)) {
      cout << "   ->cdbInitGT> payload " << hash << " already exists, skipping" << endl;
    } else {
      pl.fHash = hash;
      pl.fComment = "detector setup with magnet on";
      pl.fSchema  = cdc->getSchema();
      pl.fBLOB = spl;
      cdc->writePayloadToFile(hash, string(LOCALDIR), pl);
    }
  }

  // -- now the payloads
  vector<pair<int, int>> iovMagnet = { {1, 0}, {2177, 1}, {6302, 0}};
  vector<int> iovs;
  for (auto it: iovMagnet) {
    string templateHash = "detsetupv1_";
    string hash = "tag_" + templateHash + gt + "_iov_" + to_string(it.first);
    iovs.push_back(it.first);
    if (it.second == 1) {
      templateHash = templateHash + "MagnetOn";
    } else {
      templateHash = templateHash + "noField";
    }
    cdc->readPayloadFromFile(templateHash, string(LOCALDIR));
    payload pl = cdc->getPayload(templateHash);
    pl.fSchema = cdc->getSchema();
    pl.fHash = hash;
    cdc->writePayloadToFile(hash, jsondir + "/payloads", pl);
    cout << "   ->cdbInitGT> writing IOV " << it.first << " with " << templateHash << endl;
  }

  // -- and the tag/IOVs
  string tag = "detsetupv1_" + gt;
  stringstream sstr;
  sstr << "  { \"tag\" : \"" << tag << "\", \"iovs\" : ";
  sstr << jsFormat(iovs);
  sstr << " }" << endl;
  cout << sstr.str();
  ofstream ONS;
  ONS.open(jsondir + "/tags/" + tag);
  if (ONS.fail()) {
    cout << "Error failed to open " << jsondir + "/tags/" + tag << endl;
  }
  ONS << sstr.str();
  cout << sstr.str();
  ONS.close();

  delete cdc;
}

// ----------------------------------------------------------------------
void writePixelQualityLM(string jsondir, string gt, string payloaddir) {
  cout << "   ->cdbInitGT> writing local template pixelqualitylm payloads" << endl;
  // -- create (local template) payloads for no problematic pixels
  calPixelQualityLM *cpq = new calPixelQualityLM();
  string filename = string(LOCALDIR) + "/ascii/pixelqualitylm-" + gt + ".csv";
  cpq->readCsv(filename);
  string spl = cpq->makeBLOB();
  string hash = "tag_pixelqualitylm_" + gt + "_iov_1";
  payload pl;
  if (fileExists(jsondir + "/" + hash)) {
    cout << "   ->cdbInitGT> payload " << hash << " already exists, skipping" << endl;
  }
  pl.fHash = hash;
  pl.fComment = "pixelqualitylm with no problematic pixels";
  pl.fSchema  = cpq->getSchema();
  pl.fBLOB = spl;
  cpq->writePayloadToFile(hash, jsondir + "/payloads", pl);

  // -- and the tag/IOVs
  string tag = "pixelqualitylm_" + gt;
  stringstream sstr;
  sstr << "  { \"tag\" : \"" << tag << "\", \"iovs\" : ";
  sstr << jsFormat({static_cast<int>(1)});
  sstr << " }" << endl;
  cout << sstr.str();
  ofstream ONS;
  ONS.open(jsondir + "/tags/" + tag);
  if (ONS.fail()) {
    cout << "Error failed to open " << jsondir + "/tags/" + tag << endl;
  }
  ONS << sstr.str();
  cout << sstr.str();
  ONS.close();


  // -- now the other payloads
  glob_t globbuf;
  int err = glob((payloaddir + "/tag_pixelqualitylm_*").c_str(), 0, NULL, &globbuf);
  map<int, string> mRunToHash;
  if (err == 0)  {
    for (size_t i = 0; i < globbuf.gl_pathc; i++)  {
        cout << "   ->cdbInitGT> processing file: " << globbuf.gl_pathv[i] << endl;
        string hash = globbuf.gl_pathv[i];
        hash = hash.substr(hash.rfind('/') + 1);
        string run = hash.substr(hash.rfind('_') + 1);
        int irun = ::stoi(run);
        // -- skip run 1 (because that is created above)
        if (irun == 1) continue;
        // -- check if the run already exists in the map with the correct hash (containing the gt)
        if ((mRunToHash.find(irun) != mRunToHash.end()) 
            && (string::npos != mRunToHash[irun].find(gt))) {
          cout << "   ->cdbInitGT> run " << irun 
               << " already exists with " << mRunToHash[irun] 
               << ", skipping" 
               << endl;
          continue;
        }
        mRunToHash[::stoi(run)] = hash;
    }
    globfree(&globbuf);
  }
  for (auto it: mRunToHash) {
    cout << it.first << " -> " << it.second << " ";
    cpq->readPayloadFromFile(it.second, payloaddir);
    payload pl = cpq->getPayload(it.second);
    string hash = "tag_pixelqualitylm_" + gt + "_iov_" + to_string(it.first);
    pl.fHash = hash;
    pl.fComment = "source: " + payloaddir + "/" + it.second;
    pl.fSchema  = cpq->getSchema();
    pl.fBLOB = pl.fBLOB;
    cout << " writing payload " << hash << " for run " << it.first << endl;
    cpq->writePayloadToFile(hash, jsondir + "/payloads", pl);
    insertIovTag(jsondir, "pixelqualitylm_" + gt, it.first, 0, false);
  }
  delete cpq;
}


// ----------------------------------------------------------------------
void writeAlignmentInformation(string jsondir, string gt, string alignmentTag) {
  cout << "   ->cdbInitGT> writing alignment payloads" << endl;
  // -- pixel sensor alignment
  calPixelAlignment *cpa = new calPixelAlignment();
  string filename = string(LOCALDIR) + "/ascii/sensors-" + alignmentTag + ".csv";
  cout << "   ->cdbInitGT> reading sensor alignment from " << filename << endl;
  string result = cpa->readCsv(filename);
  if (string::npos == result.find("Error")) {
    string spl = cpa->makeBLOB();
    string hash = "tag_pixelalignment_" + gt + "_iov_1";
    payload pl;
    if (fileExists(jsondir + "/" + hash)) {
      cout << "   ->cdbInitGT> payload " << hash << " already exists, skipping" << endl;
    } else {
      pl.fHash = hash;
      pl.fComment = "pixel alignment";
      pl.fSchema  = cpa->getSchema();
      pl.fBLOB = spl;
      cpa->writePayloadToFile(hash, jsondir + "/payloads", pl);
    }
  }
  writeInitialTag(jsondir, gt, "pixelalignment_");


  // -- tile sensor alignment
  calTileAlignment *cta = new calTileAlignment();
  filename = string(LOCALDIR) + "/ascii/tiles-" + alignmentTag + ".csv";
  cout << "   ->cdbInitGT> reading tile alignment from " << filename << endl;
  result = cta->readCsv(filename);
  if (string::npos == result.find("Error")) {
    string spl = cta->makeBLOB();
    string hash = "tag_tilealignment_" + gt + "_iov_1";
    payload pl;
    if (fileExists(jsondir + "/" + hash)) {
      cout << "   ->cdbInitGT> payload " << hash << " already exists, skipping" << endl;
    } else {
      pl.fHash = hash;
      pl.fComment = "tile alignment";
      pl.fSchema  = cta->getSchema();
      pl.fBLOB = spl;
      cta->writePayloadToFile(hash, jsondir + "/payloads", pl);
    }
  }
  writeInitialTag(jsondir, gt, "tilealignment_");
  
  // -- fiber sensor alignment
  calFibreAlignment *cfa = new calFibreAlignment();
  filename = string(LOCALDIR) + "/ascii/fibres-" + alignmentTag + ".csv";
  cout << "   ->cdbInitGT> reading fibre alignment from " << filename << endl;
  result = cfa->readCsv(filename);
  if (string::npos == result.find("Error")) {
    string spl = cfa->makeBLOB();
    string hash = "tag_fibrealignment_" + gt + "_iov_1";
    payload pl;
    if (fileExists(jsondir + "/" + hash)) {
      cout << "   ->cdbInitGT> payload " << hash << " already exists, skipping" << endl;
    } else {
      pl.fHash = hash;
      pl.fComment = "fibre alignment";
      pl.fSchema  = cfa->getSchema();
      pl.fBLOB = spl;
      cfa->writePayloadToFile(hash, jsondir + "/payloads", pl);
    }
  }
  writeInitialTag(jsondir, gt, "fibrealignment_");
  
  // -- mppc sensor alignment
  calMppcAlignment *cma = new calMppcAlignment();
  filename = string(LOCALDIR) + "/ascii/mppcs-" + alignmentTag + ".csv";
  cout << "   ->cdbInitGT> reading mppc alignment from " << filename << endl;
  result = cma->readCsv(filename);
  if (string::npos == result.find("Error")) {
    string spl = cma->makeBLOB();
    string hash = "tag_mppcalignment_" + gt + "_iov_1";
    payload pl;
    if (fileExists(jsondir + "/" + hash)) {
      cout << "   ->cdbInitGT> payload " << hash << " already exists, skipping" << endl;
    } else {
      pl.fHash = hash;
      pl.fComment = "mppc alignment";
      pl.fSchema  = cma->getSchema();
      pl.fBLOB = spl;
      cma->writePayloadToFile(hash, jsondir + "/payloads", pl);
    }
  }
  writeInitialTag(jsondir, gt, "mppcalignment_");
}


// ----------------------------------------------------------------------
void writeInitialTag(string jsondir, string gt, string initialTag) {
  // -- and the tag/IOVs
  string tag = initialTag + gt;
  stringstream sstr;
  sstr << "  { \"tag\" : \"" << tag << "\", \"iovs\" : ";
  sstr << jsFormat({static_cast<int>(1)});
  sstr << " }" << endl;
  cout << sstr.str();
  ofstream ONS;
  ONS.open(jsondir + "/tags/" + tag);
  if (ONS.fail()) {
    cout << "Error failed to open " << jsondir + "/tags/" + tag << endl;
  }
  ONS << sstr.str();
  cout << sstr.str();
  ONS.close();
}

// ----------------------------------------------------------------------
// Translated from  run2025/scripts/insertIovTag into C++
// Reads jsondir/tags/<tag>, updates the IOV list by inserting (-i) or
// removing (-r) a run, or clears to single '1' when clear (-c) is true.
// Writes a .bac backup and rewrites the file in compact JSON.
void insertIovTag(const std::string &jsondir, const std::string &tag,
                  int insertRun, int removeRun, bool clear) {
  const std::string file = jsondir + "/tags/" + tag;

  // Read whole file
  std::ifstream in(file);
  if (!in) {
    std::cerr << "insertIovTag: Cannot open ->" << file << "<-" << std::endl;
    return;
  }
  std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  in.close();

  // Remove spaces and newlines to simplify parsing
  content.erase(std::remove(content.begin(), content.end(), '\n'), content.end());
  content.erase(std::remove(content.begin(), content.end(), ' '), content.end());

  // Find the iovs array between '[' and ']'
  std::vector<int> runs;
  size_t lbr = content.find("[");
  size_t rbr = content.find("]", lbr == std::string::npos ? 0 : lbr + 1);
  if (lbr != std::string::npos && rbr != std::string::npos && rbr > lbr) {
    std::string arr = content.substr(lbr + 1, rbr - lbr - 1);
    std::stringstream ss(arr);
    std::string tok;
    while (std::getline(ss, tok, ',')) {
      if (!tok.empty()) {
        try { runs.push_back(std::stoi(tok)); } catch (...) {}
      }
    }
  }

  // Modify runs per options
  if (removeRun > 0) {
    std::vector<int> filtered;
    filtered.reserve(runs.size());
    for (int r : runs) if (r != removeRun) filtered.push_back(r);
    runs.swap(filtered);
  } else if (insertRun > 0) {
    // Insert keeping ascending order and unique
    bool inserted = false;
    for (auto it = runs.begin(); it != runs.end(); ++it) {
      if (*it == insertRun) { inserted = true; break; }
      if (insertRun < *it) { runs.insert(it, insertRun); inserted = true; break; }
    }
    if (!inserted) runs.push_back(insertRun);
  }

  // Backup existing file
  {
    std::ifstream src(file, std::ios::binary);
    std::ofstream dst(file + ".bac", std::ios::binary);
    if (src && dst) dst << src.rdbuf();
  }

  // Write back
  std::ofstream out(file);
  if (!out) {
    std::cerr << "insertIovTag: Cannot open " << file << " for output" << std::endl;
    return;
  }
  out << "{\"tag\":\"" << tag << "\", \"iovs\": [";
  if (clear) {
    out << 1;
  } else {
    for (size_t i = 0; i < runs.size(); ++i) {
      out << runs[i];
      if (i + 1 < runs.size()) out << ", ";
    }
  }
  out << "]}\n";
  out.close();

  // Optional: log
  std::cout << "insertIovTag: " << tag << ": ";
  for (size_t i = 0; i < runs.size(); ++i) {
    std::cout << runs[i] << (i + 1 < runs.size() ? " " : "\n");
  }
}
