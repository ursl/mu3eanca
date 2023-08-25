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
#include "calPixelCablingMap.hh"
#include "calPixelQuality.hh"

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

using namespace std;

// ----------------------------------------------------------------------
// mdc2023FillDB [-j JSONDIR]
// --------------
//
// -j JSONDIR
//
// This creates ONLY payloads/tags/iovs/gt required for MDC2023
//
// requires ../ascii/sensors-*.csv
//
// sensors-full.csv and sensors-intrun.csv are direct CSV dumps of
// alignment/sensors in mu3e root files.
// 
// sensors-full-1.csv and sensors-intrun-1.csv are manual edits of the above
// to contain some numerical changes to see changes in the payload
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
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i], "-j"))  {jsondir = argv[++i];;}
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
		{"mdc2023",
     {"pixelalignment_mdc2023", "pixelquality_mdc2023", "pixelcablingmap_mdc2023"}
    }
  };
	
	string jdir  = jsondir + "/globaltags";
  
  for (auto igt : iniGlobalTags) {
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
		{"pixelalignment_mdc2023", {1}}, {"pixelquality_mdc2023", {1}}, {"pixelcablingmap_mdc2023", {1}}
	};

	jdir  = jsondir + "/iovs";

	for (auto iiov : iniIovs) {
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

  // -- pixelcablingmap
  calPixelCablingMap *ccm = new calPixelCablingMap();
  ccm->readJson("../ascii/pixelcablingmap-intrun.json");
  string spl = ccm->makeBLOB();
  string hash("tag_pixelcablingmap_mdc2023_iov_1");
  
  pl.fHash = hash; 
  pl.fComment = "mdc2023 initialization";
  pl.fBLOB = spl;
  ccm->printBLOB(spl); 
  ccm->writePayloadToFile(hash, jdir, pl); 
  
  // -- pixelalignment
  calPixelAlignment *cpa = new calPixelAlignment();
  cpa->readCsv("../ascii/sensors-mdc2023.csv");
  spl = cpa->makeBLOB();
  hash = string("tag_pixelalignment_mdc2023_iov_1");

  pl.fHash = hash; 
  pl.fComment = "mdc2023 initialization";
  pl.fBLOB = spl;
  cpa->printBLOB(spl); 
  cpa->writePayloadToFile(hash, jdir, pl); 

  // -- pixelquality: zero problematic pixels for all sensors present in cpa
  calPixelQuality *cpq = new calPixelQuality();
  unsigned int uid(999999);
  map<unsigned int, vector<double> > m;
  while (cpa->getNextID(uid)) {
    vector<double> v;
    m.insert(make_pair(uid, v));
    cout << "sensor = " << uid << " vector size = " << v.size() << endl;
  }
  spl = cpq->makeBLOB(m);
  hash = string("tag_pixelquality_mdc2023_iov_1");

  pl.fHash = hash; 
  pl.fComment = "mdc2023 initialization";
  pl.fBLOB = spl;
  cpq->printBLOB(spl); 
  cpq->writePayloadToFile(hash, jdir, pl); 

 
	return 0;
}
