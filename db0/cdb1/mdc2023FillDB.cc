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
    system("mkdir -p json/payloads");
    system("mkdir -p json/globaltags");
    system("mkdir -p json/iovs");

    system(string("mkdir -p " + jsondir + "/payloads").c_str());
    system(string("mkdir -p " + jsondir + "/globaltags").c_str());
    system(string("mkdir -p " + jsondir + "/iovs").c_str());
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
	jdir = jsondir + "/payloads";

  // -- pixelalignment
	for (auto iiov: iniIovs) {
    if (string::npos == iiov.first.find("pixelalignment")) continue;
		for (auto it : iiov.second) {
			stringstream sh; 
			sh << "tag_" << iiov.first;
			sh << "_iov_" << it;
      
      std::ifstream file;
      file.open("../ascii/sensors-mdc2023.bin");
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
			
      // -- JSON
      JS.open(jdir + "/" + sh.str());
      if (JS.fail()) {
        cout << "Error failed to open " << jdir << "/" << iiov.first << endl;
      }
      JS << bsoncxx::to_json(doc_value.view()) << endl;
      JS.close();
    }
	}

  // -- pixelquality
	for (auto iiov: iniIovs) {
    if (string::npos == iiov.first.find("pixelquality")) continue;
		for (auto it : iiov.second) {
			stringstream sh; 
			sh << "tag_" << iiov.first;
			sh << "_iov_" << it;

      std::ifstream file;
      file.open("../ascii/sensors-intrun-1.bin");
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
			
      // -- JSON
      JS.open(jdir + "/" + sh.str());
      if (JS.fail()) {
        cout << "Error failed to open " << jdir << "/" << iiov.first << endl;
      }
      JS << bsoncxx::to_json(doc_value.view()) << endl;
      JS.close();
    }
	}

	return 0;
}
