#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <dirent.h>  /// for directory reading

#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>
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


using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

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
// -m MODE     "cr2022", "mcideal", "dc2023"
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

// ----------------------------------------------------------------------=
int main(int argc, char* argv[]) {

	// ----------------------------------------------------------------------
	// -- global tags
	// ----------------------------------------------------------------------
	map<string, vector<string>> iniGlobalTags = {
		{"cr2022", {"pixelalignment", "pixelquality", "pixelcablingmap"} },
		{"mcideal", {"pixelalignment", "fibrealignment", "tilealignment", "mppcalignment", "pixelquality"} },
		{"dc2023", {"pixelalignment", "fibrealignment", "tilealignment", "mppcalignment", "pixelquality"} }
  };

  
  // -- command line arguments
  string jsondir("");
  string mode("mcideal");
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i], "-j"))  {jsondir = argv[++i];;}
    if (!strcmp(argv[i], "-m"))  {mode    = argv[++i];;}
  }
  
  // -- handle meta-mode
  if (string::npos != mode.find("all")) {
    for (auto it: iniGlobalTags) {
      system(string("bin/cdbInitDB -j " + jsondir + " -m " + it.first).c_str());
    }
  }
  
  // -- check whether directories for JSONs already exist
  DIR *folder = opendir(string(jsondir).c_str());
  if (folder == NULL) {
    system(string("mkdir -p " + jsondir + "/payloads").c_str());
    system(string("mkdir -p " + jsondir + "/globaltags").c_str());
    system(string("mkdir -p " + jsondir + "/iovs").c_str());
    folder = opendir(string(jsondir).c_str());
  }
  closedir(folder);

 
  ofstream JS;

	auto builder = document{};

  mongocxx::database db;
	mongocxx::collection globaltags;
	mongocxx::collection iovs;
	mongocxx::collection payloads;
    
	string jdir  = jsondir + "/globaltags";
  
  for (auto igt : iniGlobalTags) {
    if (string::npos == igt.first.find(mode)) continue;
		auto array_builder = bsoncxx::builder::basic::array{};
		for (auto it : igt.second) {
      string tag = it + "_" + igt.first; 
      array_builder.append(tag);
    }
		bsoncxx::document::value doc_value = builder
			<< "gt" << igt.first
			<< "tags" << array_builder
			<< finalize; 
    
    // -- JSON
    JS.open(jdir + "/" + igt.first);
    if (JS.fail()) {
      cout << "Error failed to open " << jdir << "/" << igt.first << endl;
    }
    JS << bsoncxx::to_json(doc_value.view()) << endl;
    cout << bsoncxx::to_json(doc_value.view()) << endl;
    JS.close();
  }

 
	// ----------------------------------------------------------------------
	// -- iovs
	// ----------------------------------------------------------------------
	jdir  = jsondir + "/iovs";
  vector<int> vIni{1};
  for (auto igt : iniGlobalTags) {
    if (string::npos == igt.first.find(mode)) continue;
		for (auto it : igt.second) {
      string tag = it + "_" + igt.first; 

      auto array_builder = bsoncxx::builder::basic::array{};
      for (auto it : vIni) array_builder.append(it);
      bsoncxx::document::value doc_value = builder
        << "tag" << tag
        << "iovs" << array_builder
        << finalize; 
      
      // -- JSON
      JS.open(jdir + "/" + tag);
      if (JS.fail()) {
        cout << "Error failed to open " << jdir << "/" << tag << endl;
      }
      JS << bsoncxx::to_json(doc_value.view()) << endl;
      JS.close();
      
    }
  }
  
  // ----------------------------------------------------------------------
	// -- payloads
	// ----------------------------------------------------------------------
  payload pl;
	jdir = jsondir + "/payloads";

  vector<string> tags{"cr2022", "mcideal", "dc2023"};
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
      cpa->printBLOB(spl); 
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
      cfa->printBLOB(spl); 
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
      cta->printBLOB(spl); 
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
      cma->printBLOB(spl); 
      cma->writePayloadToFile(hash, jdir, pl); 
    } else {
      cout << "cdbInitDB> Error, file " << filename << " not found" << endl;
    }
    
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
      ccm->printBLOB(spl); 
      ccm->writePayloadToFile(hash, jdir, pl); 
    } else {
      cout << "cdbInitDB> Error, file " << filename << " not found" << endl;
    }

    // -- pixelquality: zero problematic pixels for all sensors present in cpa
    calPixelQuality *cpq = new calPixelQuality();
    unsigned int uid(999999);
    map<unsigned int, vector<double> > m;
    while (cpa->getNextID(uid)) {
      vector<double> v;
      m.insert(make_pair(uid, v));
      // cout << "sensor = " << uid << " vector size = " << v.size() << endl;
    }
    spl = cpq->makeBLOB(m);
    hash = string("tag_pixelquality_" + it + "_iov_1");
    
    pl.fHash = hash; 
    pl.fComment = it + " pixel quality initialization";
    pl.fBLOB = spl;
    cpq->printBLOB(spl); 
    cpq->writePayloadToFile(hash, jdir, pl); 

  }

	return 0;
}
