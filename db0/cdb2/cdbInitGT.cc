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


void writeDetSetupV1(string jsondir, string gt);
void writePixelQualityLM(string jsondir, string gt, string payloaddir);

// ----------------------------------------------------------------------
int main(int argc, const char* argv[]) {

  // ----------------------------------------------------------------------
  // -- global tags
  // ----------------------------------------------------------------------
  map<string, vector<string>> iniGlobalTags = {
    {"datav6.3=2025DataV0", {"pixelalignment_", 
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

  // -- write detsetupv1 (basically magnet status) payloads and tags
  writeDetSetupV1(jsondir, gt);
  // -- write pixelqualitylm payloads and tags
  writePixelQualityLM(jsondir, gt, payloaddir);

  jdir  = jsondir + "/tags";
  vector<int> vIni{1};
  for (auto igt : iniGlobalTags) {
    //if (string::npos == igt.first.find(mode)) continue;
    if (igt.first != gt) continue;
 
    for (auto it : igt.second) {
      // string tag = ('_' == it.back()? it + igt.first: it);
      string tag = it;
      vector<int> arrayBuilder;
      for (auto it : vIni) arrayBuilder.push_back(it);
      
      stringstream sstr;
      sstr << "  { \"tag\" : \"" << tag << "\", \"iovs\" : ";
      sstr << jsFormat(arrayBuilder);
      sstr << " }" << endl;
      
      // -- JSON - do NOT overwrite already existing tags 
      string tagFile = jdir + "/" + tag;
      if (fileExists(tagFile)) {
        cout << "->cdbInitDB> tag " << tag << " already exists, skipping" << endl;
        continue;
      }
      JS.open(tagFile);
      if (JS.fail()) {
        cout << "Error failed to open " << tagFile << endl;
      }
      JS << sstr.str();
      cout << sstr.str();
      JS.close();
    }
  }
  
  // ----------------------------------------------------------------------
  // -- payloads
  // ----------------------------------------------------------------------
  payload pl;
  jdir = jsondir + "/payloads";
  
  string spl(""), hash(""), result("");
  
  string filename("");
  for (auto igt: iniGlobalTags) {

    if (igt.first != gt) continue;

    for (auto it : igt.second) {
      string tag = it;
      string tagLess = tag.substr(tag.rfind('_') + 1);

      if (string::npos != tag.find("detsetupv1_")) continue;

      cout << "cdbInitDB> tag = " << tag << " tagLess = " << tagLess << " it = " << it << endl; 
      bool exactFind(false);
      for (auto it2: igt.second) {
        if (it2 == tag) {
          exactFind = true;
          break;
        }
      }
      if (!exactFind) continue;

      stringstream sstr;
      sstr << "    { \"payload\" : \"" << tag << " tagless = " << tagLess << " it = " << it; 
      sstr << " } " << endl;
      cout << sstr.str();

      // -- pixelalignment
      if (string::npos != tag.find("pixelalignment_")) {
        calPixelAlignment *cpa = new calPixelAlignment();
        filename = string(LOCALDIR) + "/ascii/sensors-" + tagLess + ".csv";
        result = cpa->readCsv(filename);
        if (string::npos == result.find("Error")) {
          spl = cpa->makeBLOB();
          hash = string("tag_pixelalignment_" + tagLess + "_iov_1");
          if (fileExists(jdir + "/" + hash)) {
            cout << "   ->cdbInitDB> payload " << hash << " already exists, skipping" << endl;
            continue;
          }
          pl.fHash = hash;
          pl.fComment = tagLess + " pixel initialization";
          pl.fSchema  = cpa->getSchema();
          pl.fBLOB = spl;
          if (verbose) cpa->printBLOB(spl, 10000);
          cpa->writePayloadToFile(hash, jdir, pl);
        } else {
          cout << "cdbInitDB> Error, file " << filename << " not found" << endl;
        }
      }
      
      // -- fibrealignment
      if (string::npos != tag.find("fibrealignment_")) {
        calFibreAlignment *cfa = new calFibreAlignment();
        filename = string(LOCALDIR) + "/ascii/fibres-" + tagLess + ".csv";
        result = cfa->readCsv(filename);
        if (string::npos == result.find("Error")) {
          spl = cfa->makeBLOB();
          hash = string("tag_fibrealignment_" + tagLess + "_iov_1");
          if (fileExists(jdir + "/" + hash)) {
            cout << "   ->cdbInitDB> payload " << hash << " already exists, skipping" << endl;
            continue;
          }
          pl.fHash = hash;
          pl.fComment = tagLess + " fibre detector initialization";
          pl.fSchema  = cfa->getSchema();
          pl.fBLOB = spl;
          if (verbose) cfa->printBLOB(spl);
          cfa->writePayloadToFile(hash, jdir, pl);
        } else {
          cout << "cdbInitDB> Error, file " << filename << " not found" << endl;
        }
      }
      
      // -- tilealignment
      if (string::npos != tag.find("tilealignment_")) {
        calTileAlignment *cta = new calTileAlignment();
        filename = string(LOCALDIR) + "/ascii/tiles-" + tagLess + ".csv";
        result = cta->readCsv(filename);
        if (string::npos == result.find("Error")) {
          spl = cta->makeBLOB();
          hash = string("tag_tilealignment_" + tagLess + "_iov_1");
          if (fileExists(jdir + "/" + hash)) {
            cout << "   ->cdbInitDB> payload " << hash << " already exists, skipping" << endl;
            continue;
          }
          pl.fHash = hash;
          pl.fComment = tagLess + " tile detector initialization";
          pl.fSchema  = cta->getSchema();
          pl.fBLOB = spl;
          if (verbose) cta->printBLOB(spl);
          cta->writePayloadToFile(hash, jdir, pl);
        } else {
          cout << "cdbInitDB> Error, file " << filename << " not found" << endl;
        }
      }
      
      // -- mppcalignment
      if (string::npos != tag.find("mppcalignment_")) {
        calMppcAlignment *cma = new calMppcAlignment();
        filename = string(LOCALDIR) + "/ascii/mppcs-" + tagLess + ".csv";
        result = cma->readCsv(filename);
        if (string::npos == result.find("Error")) {
          spl = cma->makeBLOB();
          hash = string("tag_mppcalignment_" + tagLess + "_iov_1");
          if (fileExists(jdir + "/" + hash)) {
            cout << "   ->cdbInitDB> payload " << hash << " already exists, skipping" << endl;
            continue;
          }
          pl.fHash = hash;
          pl.fComment = tagLess + " MPPC detector initialization";
          pl.fSchema  = cma->getSchema();
          pl.fBLOB = spl;
          if (verbose) cma->printBLOB(spl);
          cma->writePayloadToFile(hash, jdir, pl);
        } else {
          cout << "cdbInitDB> Error, file " << filename << " not found" << endl;
        }
      }
    
      
      // -- pixelqualitylm: zero problematic pixels for all sensors present in cpa
      if (string::npos != tag.find("pixelqualitylm_")) {
        calPixelQualityLM *cpq = new calPixelQualityLM();
        filename = string(LOCALDIR) + "/ascii/pixelqualitylm-" + tagLess + ".csv";
        //cout << "cdbInitDB> reading " << filename << endl;
        cpq->readCsv(filename);
        string blob = cpq->makeBLOB();

        hash = string("tag_pixelqualitylm_" + tagLess + "_iov_1");
        if (fileExists(jdir + "/" + hash)) {
          cout << "   ->cdbInitDB> payload " << hash << " already exists, skipping" << endl;
          continue;
        }
        pl.fHash = hash;
        pl.fComment = tagLess + " pixelqualitylm initialization";
        pl.fBLOB = blob;
        pl.fSchema  = cpq->getSchema();
        if (verbose) cpq->printBLOB(blob);
        cpq->writePayloadToFile(hash, jdir, pl);
      }
      
    }
  }
  
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
  }
  delete cpq;
}