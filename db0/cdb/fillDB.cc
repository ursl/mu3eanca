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


int main(int argc, char* argv[]) {
	ofstream ONS;

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
		{"dt23intrun", {"pixelir", "fibresstart", "tilesNada"}}, 
		{"dt23prompt", {"pixelv0", "fibresv11", "tilesA"}}, 
		{"mcideal", {"pixelmcideal", "fibresmcideal", "tilesmcideal"}}, 
		{"mc23intrun", {"pixelmc23intrun", "fibresmc23intrun", "tilesmc23ideal"}}
	};
	
	string fname = "ascii/globaltags.txt";
	ONS.open(fname);
	if (ONS.fail()) {
		cout << "Error failed to open ->" << fname << "<-" << endl;
	}
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
	}
	ONS.close();

	// ----------------------------------------------------------------------
	// -- iovs
	// ----------------------------------------------------------------------
	mongocxx::collection iovs = db["iovs"];
	iovs.drop();

	map<string, vector<int>> iniIovs = {
		{"pixelir", {1,10,20,30,100,200}},
		{"fibresstart", {1,2,3,4,15,45,90,150}},
		{"tilesNada", {1}}, 
		{"pixelv0", {202,210,900}}, 
		{"fibresv11", {202,400,800}},
		{"tilesA", {202,300,700}},
		{"pixelmcideal", {1}},
		{"fibresmcideal", {1}},
		{"tilesmcideal", {1}},
		{"pixelmc23intrun", {200}}, 
		{"fibresmc23intrun", {150}},
		{"tilesmc23ideal", {1}}
	};

	fname = "ascii/iovs.txt";
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
	}
	ONS.close();

	// ----------------------------------------------------------------------
	// -- payloads
	// ----------------------------------------------------------------------
	mongocxx::collection payloads = db["payloads"];
	payloads.drop();

	fname = "ascii/payloads.txt";
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
				<< "payload" << sp.str()
				<< finalize; 
			
			bsoncxx::stdx::optional<mongocxx::result::insert_one> result = payloads.insert_one(doc_value.view());
			if (!result)  cout << "Failed to insert into iovs" << endl;

			// -- ASCII
			ONS << sh.str()
					<< "," << sp.str()
					<< endl;
		}
	}
	ONS.close();

	return 0;
}
