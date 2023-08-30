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
// "intrun"   the integration run detector of 2022
// "mcideal"  complete detector as contained in mu3e sim files alignment/* trees
//
// 
// -j JSONDIR  output directory with subdirectories globaltags, tags, payloads
// -m MODE     "intrun", "mcideal"
//
// requires ../ascii/*.csv
//
// Usage:
// merlin> bin/mdc2023FillDB -d ~/data/mdc2023/json/
// moor>   bin/mdc2023FillDB -d ~/data/mu3e/mdc2023/json
// 
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------=
int main(int argc, char* argv[]) {

  // -- command line arguments
  string jsondir("");
  string mode("mcideal");
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i], "-j"))  {jsondir = argv[++i];;}
    if (!strcmp(argv[i], "-m"))  {mode    = argv[++i];;}
  }

  // -- check whether directories for JSONs already exist
  DIR *folder = opendir(string(jsondir + "/payloads").c_str());
  if (folder == NULL) {
    system(string("mkdir -p " + jsondir + "/payloads").c_str());
    system(string("mkdir -p " + jsondir + "/globaltags").c_str());
    system(string("mkdir -p " + jsondir + "/iovs").c_str());
  }
  closedir(folder);

 
  ofstream JS;

	auto builder = document{};

  mongocxx::database db;
	mongocxx::collection globaltags;
	mongocxx::collection iovs;
	mongocxx::collection payloads;
    
	// ----------------------------------------------------------------------
	// -- global tags
	// ----------------------------------------------------------------------
	map<string, vector<string>> iniGlobalTags = {
		{"intrun", {"pixelalignment_intrun", "pixelquality_intrun", "pixelcablingmap_intrun"} },
		{"mcideal", {"pixelalignment_mcideal", "fibrealignment_mcideal", "tilealignment_mcideal", "mppcalignment_mcideal", "pixelquality_mcideal"} }
  };
	
	string jdir  = jsondir + "/globaltags";
  
  for (auto igt : iniGlobalTags) {
    if (string::npos == igt.first.find(mode)) continue;
		auto array_builder = bsoncxx::builder::basic::array{};
		for (auto it : igt.second) array_builder.append(it);
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
	map<string, vector<int>> iniIovs = {
    // -- intrun
		{"pixelalignment_intrun", {1}}, {"pixelquality_intrun", {1}}, {"pixelcablingmap_intrun", {1}},
    // -- mcideal
		{"pixelalignment_mcideal", {1}}, {"fibrealignment_mcideal", {1}}, {"tilealignment_mcideal", {1}}, {"mppcalignment_mcideal", {1}},
    {"pixelquality_intrun", {1}}     // ,{"pixelcablingmap_intrun", {1}}
	};

	jdir  = jsondir + "/iovs";

	for (auto iiov : iniIovs) {
    if (string::npos == iiov.first.find(mode)) continue;
		auto array_builder = bsoncxx::builder::basic::array{};
		for (auto it : iiov.second) array_builder.append(it);
		bsoncxx::document::value doc_value = builder
			<< "tag" << iiov.first
			<< "iovs" << array_builder
			<< finalize; 
		
    // -- JSON
    JS.open(jdir + "/" + iiov.first);
    if (JS.fail()) {
      cout << "Error failed to open " << jdir << "/" << iiov.first << endl;
    }
    JS << bsoncxx::to_json(doc_value.view()) << endl;
    JS.close();

  }

	// ----------------------------------------------------------------------
	// -- payloads
	// ----------------------------------------------------------------------
  payload pl;
	jdir = jsondir + "/payloads";

  vector<string> tags{"intrun", "mcideal"};
  string spl(""), hash(""), result("");

  for (auto it: tags) {
    if (string::npos == it.find(mode)) continue;
    
    // -- pixelalignment
    calPixelAlignment *cpa = new calPixelAlignment();
    result = cpa->readCsv("../ascii/sensors-" + it + ".csv");
    if (string::npos == result.find("Error")) {
      spl = cpa->makeBLOB();
      hash = string("tag_pixelalignment_" + it + "_iov_1");
      pl.fHash = hash; 
      pl.fComment = it + " pixel initialization";
      pl.fBLOB = spl;
      cpa->printBLOB(spl); 
      cpa->writePayloadToFile(hash, jdir, pl); 
    }
    
    // -- fibrealignment
    calFibreAlignment *cfa = new calFibreAlignment();
    result = cfa->readCsv("../ascii/fibres-" + it + ".csv");
    if (string::npos == result.find("Error")) {
      spl = cfa->makeBLOB();
      hash = string("tag_fibrealignment_" + it + "_iov_1");
      pl.fHash = hash; 
      pl.fComment = it + " fibre detector initialization";
      pl.fBLOB = spl;
      cfa->printBLOB(spl); 
      cfa->writePayloadToFile(hash, jdir, pl); 
    }
    
    // -- tilealignment
    calTileAlignment *cta = new calTileAlignment();
    result = cta->readCsv("../ascii/tiles-" + it + ".csv");
    if (string::npos == result.find("Error")) {
      spl = cta->makeBLOB();
      hash = string("tag_tilealignment_" + it + "_iov_1");
      pl.fHash = hash; 
      pl.fComment = it + " tile detector initialization";
      pl.fBLOB = spl;
      cta->printBLOB(spl); 
      cta->writePayloadToFile(hash, jdir, pl); 
    }
    
    // -- mppcalignment
    calMppcAlignment *cma = new calMppcAlignment();
    result = cma->readCsv("../ascii/mppcs-" + it + ".csv");
    if (string::npos == result.find("Error")) {
      spl = cma->makeBLOB();
      hash = string("tag_mppcalignment_" + it + "_iov_1");
      pl.fHash = hash; 
      pl.fComment = it + " MPPC detector initialization";
      pl.fBLOB = spl;
      cma->printBLOB(spl); 
      cma->writePayloadToFile(hash, jdir, pl); 
    }
    
    // -- pixelcablingmap
    calPixelCablingMap *ccm = new calPixelCablingMap();
    result = ccm->readJson("../ascii/pixelcablingmap-" + it + ".json");
    if (string::npos == result.find("Error")) {
      spl = ccm->makeBLOB();
      hash = "tag_pixelcablingmap_" + it + "_iov_1";
      pl.fHash = hash; 
      pl.fComment = it + "pixel cabling map initialization";
      pl.fBLOB = spl;
      ccm->printBLOB(spl); 
      ccm->writePayloadToFile(hash, jdir, pl); 
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
