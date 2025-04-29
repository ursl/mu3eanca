#include <iostream>
#include <string.h>
#include <stdio.h>

#include <fstream>
#include <vector>
#include <sstream>
#include <dirent.h>  /// for directory reading
#include <iomanip>

#include <chrono>

#include "cdbUtil.hh"
#include "base64.hh"

#include "calPixelAlignment.hh"
#include "calFibreAlignment.hh"
#include "calMppcAlignment.hh"
#include "calTileAlignment.hh"
#include "calPixelCablingMap.hh"
#include "calPixelQuality.hh"

#include "calDetConfV1.hh" // decrepit!
#include "calDetSetupV1.hh"

using namespace std;

// ----------------------------------------------------------------------
// cdbInitDB [-j JSONDIR] [-m MODE]
// ---------
//
// initialize the JSON filesystem-based CDB for all starting points
// "mcidealv5.0" old MC
// "mcidealv5.1" MC after tag v5.1
// "qc2024v1.0"  pixel sensor chipIds for first vertex module (aka half-shell)
//
// -j JSONDIR  output directory with subdirectories globaltags, tags, payloads
// -m MODE     "mcidealv5.1", ...
//
//
// Usage examples
// --------------
//
// -- NOTE: This executable is called during "make install"!
//          The JSON CDB will be installed in /install/cdb
//
// -- create all global tags with everything or with a single tag only
// merlin> _build/conddb/test/cdbInitDB -j ~/data/cdb -m all
// moor>   ./bin/cdbInitDB -j ~/data/cdb -m mcidealv5.4
//
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
int main(int argc, const char* argv[]) {

  // ----------------------------------------------------------------------
  // -- global tags
  // ----------------------------------------------------------------------
  map<string, vector<string>> iniGlobalTags = {
    {"mcidealv5.0", {"pixelalignment_", "fibrealignment_", "tilealignment_", "mppcalignment_", "detconfv1_mcidealv5.1"} },
    {"mcidealv5.1", {"pixelalignment_", "fibrealignment_", "tilealignment_", "mppcalignment_", "detconfv1_"} },
    {"mcidealv5.3", {"pixelalignment_", "fibrealignment_", "tilealignment_", "mppcalignment_", "detconfv1_mcidealv5.1"} },
    {"mcidealv5.4", {"pixelalignment_mcidealv5.3", "fibrealignment_mcidealv5.3", "tilealignment_mcidealv5.3", "mppcalignment_mcidealv5.3", "detconfv1_mcidealv5.1"} },
    {"mcidealv5.4=2025CosmicsVtxOnly", {"pixelalignment_", "detconfv1_mcidealv5.4=2025CosmicsVtxOnly"} },
    {"mcidealv6.1", {"pixelalignment_", "fibrealignment_", "tilealignment_", "mppcalignment_", "detsetupv1_", "detconfv1_mcidealv5.1", } },
    {"mcidealv6.1=2025CosmicsVtxOnly", {"pixelalignment_", "detsetupv1_mcidealv6.1=2025CosmicsVtxOnly", "detconfv1_mcidealv5.4=2025CosmicsVtxOnly"} },
      // -- data
    {"qc2024v1.0",  {"pixelalignment_", "fibrealignment_mcidealv5.1", "tilealignment_mcidealv5.1", "mppcalignment_mcidealv5.1", "detconfv1_mcidealv5.1"} }
  };
  
  
  // -- command line arguments
  string jsondir("");
  string mode("mcideal");
  int verbose(0), banner(0);
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-j"))  {jsondir = argv[++i];}
    if (!strcmp(argv[i], "-m"))  {mode    = argv[++i];}
    if (!strcmp(argv[i], "-v"))  {verbose = 1;}
    if (!strcmp(argv[i], "-b"))  {banner = 1;}
  }
  
  if (banner > 0) {
    cout << "===============" << endl;
    cout << "== cdbInitDB ==" << endl;
    cout << "===============" << endl;
    cout << "== installing in directory " << jsondir << endl;
  }
  
  // -- handle meta-mode
  if (string::npos != mode.find("all")) {
    for (auto it: iniGlobalTags) {
      system(string(string(argv[0]) + " -j " + jsondir + " -m " + it.first).c_str());
    }
  }
  
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
    if (string::npos == igt.first.find(mode)) continue;
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
  
  
  // ----------------------------------------------------------------------
  // -- tags/iovs
  // ----------------------------------------------------------------------
  jdir  = jsondir + "/tags";
  vector<int> vIni{1};
  for (auto igt : iniGlobalTags) {
    if (string::npos == igt.first.find(mode)) continue;
    for (auto it : igt.second) {
      string tag = ('_' == it.back()? it + igt.first: it);
      vector<int> arrayBuilder;
      for (auto it : vIni) arrayBuilder.push_back(it);
      
      stringstream sstr;
      sstr << "  { \"tag\" : \"" << tag << "\", \"iovs\" : ";
      sstr << jsFormat(arrayBuilder);
      sstr << " }" << endl;
      
      // -- JSON
      JS.open(jdir + "/" + tag);
      if (JS.fail()) {
        cout << "Error failed to open " << jdir << "/" << tag << endl;
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
  
  map<string, int> mPayloadCount;

  string filename("");
  for (auto igt: iniGlobalTags) {
    // string it = igt.first;
    // if (string::npos == it.find(mode)) continue;
    if (string::npos == igt.first.find(mode)) continue;
    for (auto it : igt.second) {
      string tag = ('_' == it.back()? it + igt.first: it);
      string tagLess = tag.substr(tag.rfind('_') + 1);
      
      // -- skip if this payload has already been written 
      mPayloadCount[tag]++;
      if (mPayloadCount[tag] > 1) continue;

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

      // -- detconfv1
      if (string::npos != tag.find("detconfv1_")) {
        calDetConfV1 *cdc = new calDetConfV1();
        filename = string(LOCALDIR) + "/ascii/detector-" + tagLess + ".json";
        result = cdc->readJSON(filename);
        if (string::npos == result.find("Error")) {
          spl = cdc->makeBLOB();
          hash = string("tag_detconfv1_" + tagLess + "_iov_1");
          pl.fHash = hash;
          pl.fComment = tagLess + " detector conf";
          pl.fSchema  = cdc->getSchema();
          pl.fBLOB = spl;
          if (verbose) cdc->printBLOB(spl);
          cdc->writePayloadToFile(hash, jdir, pl);
        } else {
          cout << "cdbInitDB> Error, file " << filename << " not found" << endl;
        }
      }
      
      // -- detsetupv1
      if (string::npos != tag.find("detsetupv1_")) {
        calDetSetupV1 *cdc = new calDetSetupV1();
        filename = string(LOCALDIR) + "/ascii/detector-" + tagLess + ".json";
        result = cdc->readJSON(filename);
        if (string::npos == result.find("Error")) {
          spl = cdc->makeBLOB();
          hash = string("tag_detsetupv1_" + tagLess + "_iov_1");
          pl.fHash = hash;
          pl.fComment = tagLess + " detector setup";
          pl.fSchema  = cdc->getSchema();
          pl.fBLOB = spl;
          if (verbose) cdc->printBLOB(spl);
          cdc->writePayloadToFile(hash, jdir, pl);
        } else {
          cout << "cdbInitDB> Error, file " << filename << " not found" << endl;
        }
      }
      
      
      // -- pixelquality: zero problematic pixels for all sensors present in cpa
      if (string::npos != tag.find("pixelquality_")) {
        calPixelQuality *cpq = new calPixelQuality();
        unsigned int uid(999999);
        map<unsigned int, vector<double> > m;
        while (cpq->getNextID(uid)) {
          vector<double> v;
          if (0) {
            if (uid%2 == 0) {
              v.push_back(uid%7);
              v.push_back(uid%5);
              v.push_back(1.);
            } else {
              v.push_back(uid%7);
              v.push_back(uid%5);
              v.push_back(1.);
              v.push_back(uid%17);
              v.push_back(uid%25);
              v.push_back(1.);
            }
          }
          m.insert(make_pair(uid, v));
          //      cout << "sensor = " << uid << " vector size = " << v.size() << endl;
        }
        spl = cpq->makeBLOB(m);
        hash = string("tag_pixelquality_" + tagLess + "_iov_1");
        
        pl.fHash = hash;
        pl.fComment = tagLess + " pixel quality initialization";
        pl.fBLOB = spl;
        pl.fSchema  = cpq->getSchema();
        if (verbose) cpq->printBLOB(spl);
        cpq->writePayloadToFile(hash, jdir, pl);
        cpq->insertPayload(hash, pl);
        cpq->writeCsv("pixelquality-example.csv");
      }
      
      // -- pixelcablingmap
      if (string::npos != tag.find("pixelcablingmap_")) {
        calPixelCablingMap *ccm = new calPixelCablingMap();
        filename = "./ascii/pixelcablingmap-" + tagLess + ".json";
        result = ccm->readJson(filename);
        if (string::npos == result.find("Error")) {
          spl = ccm->makeBLOB();
          hash = "tag_pixelcablingmap_" + tagLess + "_iov_1";
          pl.fHash = hash;
          pl.fComment = tagLess + "pixel cabling map initialization";
          pl.fSchema  = ccm->getSchema();
          pl.fBLOB = spl;
          if (verbose) ccm->printBLOB(spl);
          ccm->writePayloadToFile(hash, jdir, pl);
        } else {
          cout << "cdbInitDB> Error, file " << filename << " not found" << endl;
        }
      }
    }
  }
  
  // -- create a runRecord
  filename = "runlog_004001.json";
  ifstream INS;
  INS.open(string(LOCALDIR) + "/ascii/" + filename);
  std::stringstream buffer;
  buffer << INS.rdbuf();
  INS.close();
  
  jdir = jsondir + "/runrecords";
  string orr = jdir + "/" + filename;
  
  
  JS.open(orr);
  if (JS.fail()) {
    cout << "cdbInitDB> Error failed to open " << orr <<  endl;
  }
  JS << buffer.str();
  JS.close();
  
  return 0;
}
