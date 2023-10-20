#include "cdbROOT.hh"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <dirent.h>  /// for directory reading

#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::basic::sub_array;
using bsoncxx::builder::basic::sub_document;
using bsoncxx::builder::basic::make_document;



using namespace std;

// ----------------------------------------------------------------------
cdbROOT::cdbROOT(string gt, string uri, int verbose) : cdbAbs(gt, uri, verbose) {
  init();
}


// ----------------------------------------------------------------------
cdbROOT::~cdbROOT() { }


// ----------------------------------------------------------------------
void cdbROOT::init() {
  fName = "ROOT"; 
  cdbAbs::init();
}


// ----------------------------------------------------------------------
payload cdbROOT::getPayload(string hash) {
  // -- initialize with default
  std::stringstream sspl;
  sspl << "(cdbROOT>  hash = " << hash 
       << " not found)";
  payload pl;
  pl.fComment = sspl.str();
  
  // -- read payload for hash 
  ifstream INS;
  string filename = fURI + "/payloads/" + hash;
  INS.open(filename);
  if (INS.fail()) {
    cout << "Error failed to open ->" << filename << "<-" << endl;
    return pl;
  }

  std::stringstream buffer;
  buffer << INS.rdbuf();
  INS.close();
  
  cout << "cdbROOT::getPayload() Read " << filename << " hash ->" << hash << "<-" << endl;
  bsoncxx::document::value doc = bsoncxx::from_json(buffer.str());
  pl.fComment = string(doc["comment"].get_string().value).c_str();
  pl.fHash    = string(doc["hash"].get_string().value).c_str();
  pl.fBLOB    = string(doc["BLOB"].get_string().value).c_str();

  return pl;
}


