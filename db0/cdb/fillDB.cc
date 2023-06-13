#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

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
// fillDB
// ------
//
// Examples:
// bin/fillDB
// 
// ----------------------------------------------------------------------



int main(int argc, char* argv[]) {
	ofstream ONS, JS;

	auto builder = document{};
    
  mongocxx::instance instance{};
  mongocxx::uri uri("mongodb://127.0.0.1:27017/?directConnection=true&serverSelectionTimeoutMS=2000&appName=mongosh+1.7.1");
  mongocxx::client client(uri);

  
  mongocxx::database db = client["mu3e"];
    
	// ----------------------------------------------------------------------
	// -- global tags
	// ----------------------------------------------------------------------
	mongocxx::collection globaltags = db["globaltags"];
	globaltags.drop();

	map<string, vector<string>> iniGlobalTags = {
		{"dt23intrun", {"pixel_ir", "pixelalignment_ir", "fibres_start", "tiles_Nada"}}, 
		{"dt23prompt", {"pixel_v0", "pixelalignment_v0", "fibres_v11", "tiles_A"}}, 
		{"mcideal", {"pixel_mcideal", "pixelalignment_mcideal", "fibres_mcideal", "tiles_mcideal"}}, 
		{"mc23intrun", {"pixel_mc23intrun", "pixelalignment_mc23intrun", "fibres_mc23intrun", "tiles_mc23ideal"}}
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

		bsoncxx::stdx::optional<mongocxx::result::insert_one> result = globaltags.insert_one(doc_value.view());
		if (!result)  cout << "Failed to insert" << endl;

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
	mongocxx::collection iovs = db["iovs"];
	iovs.drop();

	map<string, vector<int>> iniIovs = {
		{"pixel_ir", {1,10,20,30,100,200}},
		{"pixelalignment_ir", {1,200}},
		{"fibres_start", {1,2,3,4,15,45,90,150}},
		{"tiles_Nada", {1}}, 
		{"pixel_v0", {202,210,900}}, 
		{"pixelalignment_v0", {202,210,900}}, 
		{"fibres_v11", {202,400,800}},
		{"tiles_A", {202,300,700}},
		{"pixel_mcideal", {1}},
		{"pixelalignment_mcideal", {1}},
		{"fibres_mcideal", {1}},
		{"tiles_mcideal", {1}},
		{"pixel_mc23intrun", {200}}, 
		{"pixelalignment_mc23intrun", {200}}, 
		{"fibres_mc23intrun", {150}},
		{"tiles_mc23ideal", {1}}
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
		
		bsoncxx::stdx::optional<mongocxx::result::insert_one> result = iovs.insert_one(doc_value.view());
		if (!result)  cout << "Failed to insert into iovs" << endl;
		
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
	mongocxx::collection payloads = db["payloads"];
	payloads.drop();

	fname = "ascii/payloads.txt";
	jdir = "ascii/payloads";
	ONS.open(fname);
	if (ONS.fail()) {
		cout << "Error failed to open ->" << fname << "<-" << endl;
	}
	for (auto iiov: iniIovs) {
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
			
			bsoncxx::stdx::optional<mongocxx::result::insert_one> result = payloads.insert_one(doc_value.view());
			if (!result)  cout << "Failed to insert into iovs" << endl;

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

	return 0;
}
