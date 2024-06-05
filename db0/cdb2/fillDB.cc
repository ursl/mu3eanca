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
#include "cfgPayload.hh"

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
  string jfile("");
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-j"))  {jfile = argv[i+1];}
  }
  
  cout << "jfile = " << jfile << endl;
  
  ifstream INS;
  INS.open(jfile);
  
  std::stringstream buffer;
  buffer << INS.rdbuf();
  INS.close();
  
  string collectionContents = buffer.str();
  
  cout << "1 ----------------------------------------------------------------------" << endl;
  cout << collectionContents << endl;
  
  cfgPayload a(collectionContents);
  
  cout << "2 ----------------------------------------------------------------------" << endl;
  cout << a.getJson() << endl;
  
  a.fCfgString = base64_encode(a.fCfgString);
  string collectionContents2 = a.getJson();
  
  auto b = bsoncxx::from_json(collectionContents2);
  
  
  return 0;
}
