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


using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

using namespace std;

// ----------------------------------------------------------------------
// fillDB [-json]
// --------------
//
// -json  ONLY the JSON file-based DB is written, but not the mongo DB
//
// requires ascii/sensors-*.csv
//
// sensors-full.csv and sensors-intrun.csv are direct CSV dumps of
// alignment/sensors in mu3e root files.
// 
// sensors-full-1.csv and sensors-intrun-1.csv are manual edits of the above
// to contain some numerical changes to see changes in the payload
//
// Usage:
// bin/fillDB
// 
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------=
int main(int argc, char* argv[]) {


  // -- command line arguments
  bool json(0);
  string password("fixme");
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i], "-json"))  {json = true;}
    if (!strcmp(argv[i], "-pw"))  {password = argv[++i];}
  }

  // -- check whether directories for JSONs already exist
  DIR *folder = opendir("ascii/payloads");
  if (folder == NULL) {
    system("mkdir -p ascii/payloads");
    system("mkdir -p ascii/globaltags");
    system("mkdir -p ascii/iovs");
  }
  closedir(folder);
  
  ofstream ONS, JS;

	auto builder = document{};
    
  mongocxx::instance instance{};
  mongocxx::uri URI;
  if (string::npos != password.find("fixme")) {
    URI = mongocxx::uri("mongodb://127.0.0.1:27017/?directConnection=true&serverSelectionTimeoutMS=2000&appName=mongosh+1.7.1");
  } else {
    URI = mongocxx::uri("mongodb+srv://urslangenegger:" + password + "@cdb0.fmlmtd8.mongodb.net/?retryWrites=true&w=majority");
  }
  mongocxx::client client(URI);

  
  mongocxx::database db;
	mongocxx::collection globaltags;
	mongocxx::collection iovs;
	mongocxx::collection payloads;
  if (!json) db = client["mu3e"];
    
	// ----------------------------------------------------------------------
	// -- global tags
	// ----------------------------------------------------------------------
  if (!json) {
    globaltags = db["globaltags"];
    globaltags.drop();
  }

	map<string, vector<string>> iniGlobalTags = {
		{"dt23intrun", {"pixel_dt23intrun", "pixelalignment_dt23intrun", "fibres_start",      "tiles_Nada"}}, 
		{"dt23prompt", {"pixel_v0",         "pixelalignment_v0",         "fibres_v11",        "tiles_A"}}, 
		{"mcideal",    {"pixel_mcideal",    "pixelalignment_mcideal",    "fibres_mcideal",    "tiles_mcideal"}}, 
		{"mc23intrun", {"pixel_mc23intrun", "pixelalignment_mc23intrun", "fibres_mc23intrun", "tiles_Nada"}}
	};
	
	string fname = "ascii/globaltags.txt";
	ONS.open(fname);
	if (ONS.fail()) {
		cout << "Error failed to open ->" << fname << "<-" << endl;
	}

	string jdir  = "ascii/globaltags";

  for (auto igt : iniGlobalTags) {
		auto array_builder = bsoncxx::builder::basic::array{};
		for (auto it : igt.second) array_builder.append(it);
		bsoncxx::document::value doc_value = builder
			<< "gt" << igt.first
			<< "tags" << array_builder
			<< finalize; 

		if (!json) {
      bsoncxx::stdx::optional<mongocxx::result::insert_one> result = globaltags.insert_one(doc_value.view());
      if (!result)  cout << "Failed to insert" << endl;
    }
    
		// -- ASCII
		ONS << igt.first;
		for (auto it : igt.second) {
			ONS << "," << it;
		}
		ONS << endl;

    // -- JSON
    JS.open(jdir + "/" + igt.first);
    if (JS.fail()) {
      cout << "Error failed to open " << jdir << "/" << igt.first << endl;
    }
    JS << bsoncxx::to_json(doc_value.view()) << endl;
    JS.close();

  }
	ONS.close();

	// ----------------------------------------------------------------------
	// -- iovs
	// ----------------------------------------------------------------------
  if (!json) {
    iovs = db["iovs"];
    iovs.drop();
  }
  
	map<string, vector<int>> iniIovs = {
		{"pixelalignment_dt23intrun", {1,200}},
		{"pixelalignment_v0", {1,200}},
		{"pixelalignment_mc23intrun", {200}}, 
		{"pixelalignment_mcideal", {1}},
    // 
		{"pixel_dt23intrun", {1,10,20,30,100,200}},
		{"pixel_v0", {202,210,900}}, 
		{"pixel_mcideal", {1}},
		{"pixel_mc23intrun", {200}}, 
    //
		{"tiles_Nada", {1}}, 
		{"tiles_A", {202,300,700}},
		{"tiles_mcideal", {1}},
		{"tiles_mc23ideal", {1}},
    //
		{"fibres_start", {1,50}},
		{"fibres_v11", {202,400,800}},
		{"fibres_mcideal", {1}},
		{"fibres_mc23intrun", {150}}
	};

	fname = "ascii/iovs.txt";
	jdir  = "ascii/iovs";
	ONS.open(fname);
	if (ONS.fail()) {
		cout << "Error failed to open ->" << fname << "<-" << endl;
	}
	for (auto iiov : iniIovs) {
		auto array_builder = bsoncxx::builder::basic::array{};
		for (auto it : iiov.second) array_builder.append(it);
		bsoncxx::document::value doc_value = builder
			<< "tag" << iiov.first
			<< "iovs" << array_builder
			<< finalize; 
		
    if (!json) {
      bsoncxx::stdx::optional<mongocxx::result::insert_one> result = iovs.insert_one(doc_value.view());
      if (!result)  cout << "Failed to insert into iovs" << endl;
		}
    
		// -- ASCII
		ONS << iiov.first;
		for (auto it : iiov.second) {
			ONS << "," << it;
		}
		ONS << endl;

    // -- JSON
    JS.open(jdir + "/" + iiov.first);
    if (JS.fail()) {
      cout << "Error failed to open " << jdir << "/" << iiov.first << endl;
    }
    JS << bsoncxx::to_json(doc_value.view()) << endl;
    JS.close();

  }
	ONS.close();

	// ----------------------------------------------------------------------
	// -- payloads
	// ----------------------------------------------------------------------
  if (!json) {
    payloads = db["payloads"];
    payloads.drop();
  }

	fname = "ascii/payloads.txt";
	jdir = "ascii/payloads";
	ONS.open(fname);
	if (ONS.fail()) {
		cout << "Error failed to open ->" << fname << "<-" << endl;
	}

	for (auto iiov: iniIovs) {
    // -- do the pixelalignment separately below
    if (string::npos != iiov.first.find("pixelalignment")) continue;
		for (auto it : iiov.second) {
			stringstream sh; 
			sh << "tag_" << iiov.first;
			sh << "_iov_" << it;
			stringstream sp;
			sp << "payload " << iiov.first << " for iov " << it;

			cout << sh.str() << "-> " << sp.str() << endl;
			
			bsoncxx::document::value doc_value = builder
				<< "hash" << sh.str()
				<< "comment" << "testing"
				<< "BLOB" << sp.str()
				<< finalize; 

      if (!json) {
        bsoncxx::stdx::optional<mongocxx::result::insert_one> result = payloads.insert_one(doc_value.view());
        if (!result)  cout << "Failed to insert into iovs" << endl;
      }
      
			// -- ASCII
			ONS << sh.str()
          << "," << "testing"
					<< "," << sp.str()
					<< endl;

      // -- JSON
      JS.open(jdir + "/" + sh.str());
      if (JS.fail()) {
        cout << "Error failed to open " << jdir << "/" << iiov.first << endl;
      }
      JS << bsoncxx::to_json(doc_value.view()) << endl;
      JS.close();
    }
	}
	ONS.close();
  

  // -- pixelalignment
	ONS.open(fname, std::ios_base::app);
	for (auto iiov: iniIovs) {
    if (string::npos == iiov.first.find("pixelalignment")) continue;
    int cnt(0); 
		for (auto it : iiov.second) {
			stringstream sh; 
			sh << "tag_" << iiov.first;
			sh << "_iov_" << it;
			stringstream sp0, sp1;

      std::ifstream file;
      if (string::npos != iiov.first.find("intrun")) {
        if (0 == cnt%2) file.open("ascii/sensors-intrun.csv");
        if (1 == cnt%2) file.open("ascii/sensors-intrun-1.csv");
      } else {
        if (0 == cnt%2) file.open("ascii/sensors-full.csv");
        if (1 == cnt%2) file.open("ascii/sensors-full-1.csv");
      }
      string sline; 
      bool first(true);
      while (getline(file, sline)) {
        if (first) {
          first = false;
        } else {
          sp0 << ",";
        }
        sp0 << sline;
      }
      file.close();
      // -- kludge to remove trailing ","
      string stmp = sp0.str();
      cout << "stmp.size() = " << stmp.size() << endl;
      stmp.erase(stmp.size(), 1);
      sp1 << stmp; 
			cout << sh.str() << "-> " << sp1.str() << endl;
			
			bsoncxx::document::value doc_value = builder
				<< "hash" << sh.str()
				<< "comment" << "testing"
				<< "BLOB" << sp1.str()
				<< finalize; 
			
      if (!json) {
        bsoncxx::stdx::optional<mongocxx::result::insert_one> result = payloads.insert_one(doc_value.view());
        if (!result)  cout << "Failed to insert into iovs" << endl;
      }
      
			// -- ASCII
			ONS << sh.str()
          << "," << "testing"
					<< "," << sp1.str()
					<< endl;

      // -- JSON
      JS.open(jdir + "/" + sh.str());
      if (JS.fail()) {
        cout << "Error failed to open " << jdir << "/" << iiov.first << endl;
      }
      JS << bsoncxx::to_json(doc_value.view()) << endl;
      JS.close();
      ++cnt;
    }
	}
  
	return 0;
}
