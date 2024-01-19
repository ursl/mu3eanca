#include <iostream>
#include <string.h>
#include <stdio.h>

#include <fstream>
#include <vector>
#include <sstream>
#include <dirent.h>  /// for directory reading

#include <chrono>

#include "cdbUtil.hh"
#include "base64.hh"

#include "calPixelAlignment.hh"
#include "calFibreAlignment.hh"
#include "calMppcAlignment.hh"
#include "calTileAlignment.hh"
#include "calPixelCablingMap.hh"
#include "calPixelQuality.hh"

#include "calFibreAlignment.hh"

using namespace std;

// ----------------------------------------------------------------------
// cdbInitDB [-j JSONDIR] [-m MODE]
// ---------
//
// initialize the JSON filesystem-based CDB for several starting points
// "cr2022"      the cosmic run detector of 2022
// "mcideal"     complete detector as contained in mu3e sim files alignment/* trees
// "dc2023"      data challenge 2023 (with new pixel chip ID naming scheme)
//
// 
// -j JSONDIR  output directory with subdirectories globaltags, tags, payloads
// -m MODE     "mcideal", "dc2023"
//
// requires ../ascii/*.csv
//
// Usage examples
//
// merlin> bin/cdbInitDB -j ~/data/mdc2023/json/ -m dc2023
//
// moor>   bin/cdbInitDB -j ~/data/mu3e/json -m cr2022
// moor>   bin/cdbInitDB -j ~/data/mu3e/json -m mcideal
// moor>   bin/cdbInitDB -j ~/data/mu3e/json -m dc2023
// 
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {

	// ----------------------------------------------------------------------
	// -- global tags
	// ----------------------------------------------------------------------
	map<string, vector<string>> iniGlobalTags = {
		{"mcidealv5.0", {"pixelalignment", "fibrealignment", "tilealignment", "mppcalignment"} },
		{"mcidealv5.1", {"pixelalignment", "fibrealignment", "tilealignment", "mppcalignment"} }
  };

  
  // -- command line arguments
  string jsondir("");
  string mode("mcideal");
  int verbose(0); 
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i], "-j"))  {jsondir = argv[++i];}
    if (!strcmp(argv[i], "-m"))  {mode    = argv[++i];}
    if (!strcmp(argv[i], "-v"))  {verbose = 1;}
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
      string tag = it + "_" + igt.first; 
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
      string tag = it + "_" + igt.first; 
      vector<int> arrayBuilder;
      for (auto it : vIni) arrayBuilder.push_back(it);

      stringstream sstr;
      sstr << "{ \"tag\" : \"" << tag << "\", \"iovs\" : ";
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

  for (auto igt: iniGlobalTags) {
    string it = igt.first;
    if (string::npos == it.find(mode)) continue;

    string filename("");

    // -- pixelalignment
    calPixelAlignment *cpa = new calPixelAlignment();
    filename = "../ascii/sensors-" + it + ".csv";
    result = cpa->readCsv(filename);
    if (string::npos == result.find("Error")) {
      spl = cpa->makeBLOB();
      hash = string("tag_pixelalignment_" + it + "_iov_1");
      pl.fHash = hash; 
      pl.fComment = it + " pixel initialization";
      pl.fBLOB = spl;
      if (verbose) cpa->printBLOB(spl); 
      cpa->writePayloadToFile(hash, jdir, pl); 
    } else {
      cout << "cdbInitDB> Error, file " << filename << " not found" << endl;
    }
    
    // -- fibrealignment
    calFibreAlignment *cfa = new calFibreAlignment();
    filename = "../ascii/fibres-" + it + ".csv";
    result = cfa->readCsv(filename);
    if (string::npos == result.find("Error")) {
      spl = cfa->makeBLOB();
      hash = string("tag_fibrealignment_" + it + "_iov_1");
      pl.fHash = hash; 
      pl.fComment = it + " fibre detector initialization";
      pl.fBLOB = spl;
      if (verbose) cfa->printBLOB(spl); 
      cfa->writePayloadToFile(hash, jdir, pl); 
    } else {
      cout << "cdbInitDB> Error, file " << filename << " not found" << endl;
    }
    
    // -- tilealignment
    calTileAlignment *cta = new calTileAlignment();
    filename = "../ascii/tiles-" + it + ".csv";
    result = cta->readCsv(filename);
    if (string::npos == result.find("Error")) {
      spl = cta->makeBLOB();
      hash = string("tag_tilealignment_" + it + "_iov_1");
      pl.fHash = hash; 
      pl.fComment = it + " tile detector initialization";
      pl.fBLOB = spl;
      if (verbose) cta->printBLOB(spl); 
      cta->writePayloadToFile(hash, jdir, pl); 
    } else {
      cout << "cdbInitDB> Error, file " << filename << " not found" << endl;
    }
    
    // -- mppcalignment
    calMppcAlignment *cma = new calMppcAlignment();
    filename = "../ascii/mppcs-" + it + ".csv";
    result = cma->readCsv(filename);
    if (string::npos == result.find("Error")) {
      spl = cma->makeBLOB();
      hash = string("tag_mppcalignment_" + it + "_iov_1");
      pl.fHash = hash; 
      pl.fComment = it + " MPPC detector initialization";
      pl.fBLOB = spl;
      if (verbose) cma->printBLOB(spl); 
      cma->writePayloadToFile(hash, jdir, pl); 
    } else {
      cout << "cdbInitDB> Error, file " << filename << " not found" << endl;
    }


    // -- pixelquality: zero problematic pixels for all sensors present in cpa
    calPixelQuality *cpq = new calPixelQuality();
    unsigned int uid(999999);
    map<unsigned int, vector<double> > m;
    while (cpa->getNextID(uid)) {
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
    hash = string("tag_pixelquality_" + it + "_iov_1");
    
    pl.fHash = hash; 
    pl.fComment = it + " pixel quality initialization";
    pl.fBLOB = spl;
    if (verbose) cpq->printBLOB(spl); 
    cpq->writePayloadToFile(hash, jdir, pl); 
    cpq->insertPayload(hash, pl);
    cpq->writeCsv("pixelquality-example.csv");
    

    if (0) {
      // -- pixelcablingmap
      calPixelCablingMap *ccm = new calPixelCablingMap();
      filename = "../ascii/pixelcablingmap-" + it + ".json";
      result = ccm->readJson(filename);
      if (string::npos == result.find("Error")) {
        spl = ccm->makeBLOB();
        hash = "tag_pixelcablingmap_" + it + "_iov_1";
        pl.fHash = hash; 
        pl.fComment = it + "pixel cabling map initialization";
        pl.fBLOB = spl;
        if (verbose) ccm->printBLOB(spl); 
        ccm->writePayloadToFile(hash, jdir, pl); 
      } else {
        cout << "cdbInitDB> Error, file " << filename << " not found" << endl;
      }
    }
  }

  // -- create a runRecord
  runRecord rr;
  rr.fRun = 12;
  rr.fRunStart = timeStamp();

  jdir = jsondir + "/runrecords";
  JS.open(jdir + "/" + to_string(rr.fRun));
  if (JS.fail()) {
    cout << "cdbInitDB> Error failed to open " << jdir << "/" << to_string(rr.fRun) <<  endl;
  }
  JS << rr.json() << endl;
  JS.close();



  // -- create configs
  vector<string> conffiles = {
    "detector.json", 
    "trirec.conf", "vertex.conf"
  };
  
  for (auto igt: iniGlobalTags) {
    for (auto ic: conffiles) {
      string cfgname = ic.substr(0, ic.find(".")); 
      string filename = "run/" + ic;
      ifstream INS;
      INS.open(filename);
      if (INS.fail()) {
        cout << "Error failed to open ->" << filename << "<-" << endl;
        continue;
      }
      
      std::stringstream buffer;
      buffer << INS.rdbuf();
      INS.close();
      
      jdir = jsondir + "/configs";
      hash = "cfg_" + cfgname + "_" + igt.first;
      
      JS.open(jdir + "/" + hash);
      if (JS.fail()) {
        cout << "cdbInitDB> Error failed to open " << jdir << "/" << hash <<  endl;
      }

      cfgPayload cfg;
      cfg.fHash = hash;
      cfg.fDate = timeStamp();
      cfg.fCfgString = buffer.str();
      
      JS << cfg.json();
      JS.close();
    }
  }
  
	return 0;
}
