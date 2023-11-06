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
// -json  ONLY the JSON file-based DB is written, but not the mongo DB (useful for merlin)
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
  DIR *folder = opendir("json/payloads");
  if (folder == NULL) {
    system("mkdir -p json/payloads");
    system("mkdir -p json/globaltags");
    system("mkdir -p json/iovs");
  }
  closedir(folder);
  
  ofstream JS;

	auto builder = document{};
    
  //  mongocxx::instance instance{};
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
		{"dt23intrun",
     {"pixel_dt23intrun",
      "pixelalignment_dt23intrun",
      "fibrealignment_dt23intrun",
      "tilealignment_Nada"}}, 
		{"dt23prompt",
     {"pixel_v0",
      "pixelalignment_v0",
      "fibrealignment_v0",
      "tilealignment_A"}}, 
		{"mcideal",
     {"pixel_mcideal",
      "pixelalignment_mcideal",
      "fibrealignment_mcideal",
      "mppcalignment_mcideal",
      "tilealignment_mcideal"}}, 
		{"mc23intrun",
     {"pixel_mc23intrun",
      "pixelalignment_mc23intrun",
      "fibrealignment_mc23intrun",
      "tilealignment_Nada"}}
	};
	
	string jdir  = "json/globaltags";

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
    
    // -- JSON
    JS.open(jdir + "/" + igt.first);
    if (JS.fail()) {
      cout << "Error failed to open " << jdir << "/" << igt.first << endl;
    }
    JS << bsoncxx::to_json(doc_value.view()) << endl;
    JS.close();

  }

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
		{"pixelalignment_mcideal", {1,300}},
    // 
		{"pixel_dt23intrun", {1,10,20,30,100,200}},
		{"pixel_v0", {202,210,900}}, 
		{"pixel_mcideal", {1}},
		{"pixel_mc23intrun", {200}}, 
    //
		{"tilealignment_Nada", {1}}, 
		{"tilealignment_A", {202,300,700}},
		{"tilealignment_mcideal", {1,400}},
		{"tilealignment_mc23ideal", {1}},
    //
		{"fibrealignment_dt23intrun", {1,200}},
		{"fibrealignment_v11", {202,400,800}},
		{"fibrealignment_mcideal", {1,300}},
		{"fibrealignment_mc23intrun", {150}},
    //
    {"mppcalignment_mcideal", {1,300}},
	};

	jdir  = "json/iovs";

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
  if (!json) {
    payloads = db["payloads"];
    payloads.drop();
  }

	jdir = "json/payloads";

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
      
      // -- JSON
      JS.open(jdir + "/" + sh.str());
      if (JS.fail()) {
        cout << "Error failed to open " << jdir << "/" << iiov.first << endl;
      }
      JS << bsoncxx::to_json(doc_value.view()) << endl;
      JS.close();
    }
	}
  

  // -- pixelalignment
	for (auto iiov: iniIovs) {
    if (string::npos == iiov.first.find("pixelalignment")) continue;
    int cnt(0); 
		for (auto it : iiov.second) {
			stringstream sh; 
			sh << "tag_" << iiov.first;
			sh << "_iov_" << it;

      std::ifstream file;
      // -- Note: 779 should be the same as the normal one
      if (string::npos != iiov.first.find("intrun")) {
        if (1 == cnt%2) file.open("../ascii/sensors-intrun.bin");
        if (0 == cnt%2) file.open("../ascii/sensors-intrun-1.bin");
      } else {
        if (1 == cnt%2) file.open("../ascii/sensors-full.bin");
        if (0 == cnt%2) file.open("../ascii/sensors-full-1.bin");
      }
      vector<char> buffer(std::istreambuf_iterator<char>(file), {});
      string sblob("");
      for (unsigned int i = 0; i < buffer.size(); ++i) sblob.push_back(buffer[i]);
      std::vector<char>::iterator ibuffer = buffer.begin();
      long unsigned int header = blob2UnsignedInt(getData(ibuffer)); 
      cout << "header: " << hex
           << header << endl;
      
			bsoncxx::document::value doc_value = builder
				<< "hash" << sh.str()
				<< "comment" << "testing"
				<< "BLOB" << base64_encode(sblob)
				<< finalize; 
			
      if (!json) {
        bsoncxx::stdx::optional<mongocxx::result::insert_one> result = payloads.insert_one(doc_value.view());
        if (!result)  cout << "Failed to insert into iovs" << endl;
      }
      
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


  // -- fibres
	for (auto iiov: iniIovs) {
    if (string::npos == iiov.first.find("fibrealignment")) continue;
    int cnt(0); 
		for (auto it : iiov.second) {
			stringstream sh; 
			sh << "tag_" << iiov.first;
			sh << "_iov_" << it;

      std::ifstream file;
      // -- Note: 779 should be the same as the normal one
      if (0 == cnt%2) file.open("../ascii/fibres-full-1.bin");
      if (1 == cnt%2) file.open("../ascii/fibres-full.bin");
      vector<char> buffer(std::istreambuf_iterator<char>(file), {});
      string sblob("");
      for (unsigned int i = 0; i < buffer.size(); ++i) sblob.push_back(buffer[i]);
      std::vector<char>::iterator ibuffer = buffer.begin();
      long unsigned int header = blob2UnsignedInt(getData(ibuffer)); 
      cout << "header: " << hex
           << header << endl;
      
			bsoncxx::document::value doc_value = builder
				<< "hash" << sh.str()
				<< "comment" << "testing"
				<< "BLOB" << base64_encode(sblob)
				<< finalize; 
			
      if (!json) {
        bsoncxx::stdx::optional<mongocxx::result::insert_one> result = payloads.insert_one(doc_value.view());
        if (!result)  cout << "Failed to insert into iovs" << endl;
      }
      
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

  // -- mppcs
	for (auto iiov: iniIovs) {
    if (string::npos == iiov.first.find("mppcalignment")) continue;
    int cnt(0); 
		for (auto it : iiov.second) {
			stringstream sh; 
			sh << "tag_" << iiov.first;
			sh << "_iov_" << it;

      std::ifstream file;
      // -- Note: 779 should be the same as the normal one
      if (0 == cnt%2) file.open("../ascii/mppcs-full-1.bin");
      if (1 == cnt%2) file.open("../ascii/mppcs-full.bin");
      vector<char> buffer(std::istreambuf_iterator<char>(file), {});
      string sblob("");
      for (unsigned int i = 0; i < buffer.size(); ++i) sblob.push_back(buffer[i]);
      std::vector<char>::iterator ibuffer = buffer.begin();
      long unsigned int header = blob2UnsignedInt(getData(ibuffer)); 
      cout << "header: " << hex
           << header << endl;
      
			bsoncxx::document::value doc_value = builder
				<< "hash" << sh.str()
				<< "comment" << "testing"
				<< "BLOB" << base64_encode(sblob)
				<< finalize; 
			
      if (!json) {
        bsoncxx::stdx::optional<mongocxx::result::insert_one> result = payloads.insert_one(doc_value.view());
        if (!result)  cout << "Failed to insert into iovs" << endl;
      }
      
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

  // -- tiles
	for (auto iiov: iniIovs) {
    if (string::npos == iiov.first.find("tilealignment")) continue;
    int cnt(0); 
		for (auto it : iiov.second) {
			stringstream sh; 
			sh << "tag_" << iiov.first;
			sh << "_iov_" << it;

      std::ifstream file;
      // -- Note: 779 should be the same as the normal one
      if (0 == cnt%2) file.open("../ascii/tiles-full-1.bin");
      if (1 == cnt%2) file.open("../ascii/tiles-full.bin");
      vector<char> buffer(std::istreambuf_iterator<char>(file), {});
      string sblob("");
      for (unsigned int i = 0; i < buffer.size(); ++i) sblob.push_back(buffer[i]);
      std::vector<char>::iterator ibuffer = buffer.begin();
      long unsigned int header = blob2UnsignedInt(getData(ibuffer)); 
      cout << "header: " << hex
           << header << endl;
      
			bsoncxx::document::value doc_value = builder
				<< "hash" << sh.str()
				<< "comment" << "testing"
				<< "BLOB" << base64_encode(sblob)
				<< finalize; 
			
      if (!json) {
        bsoncxx::stdx::optional<mongocxx::result::insert_one> result = payloads.insert_one(doc_value.view());
        if (!result)  cout << "Failed to insert into iovs" << endl;
      }
      
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
