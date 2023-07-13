#include "calAbs.hh"

#include "TFile.h"

#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>

#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>

#include "base64.hh"


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
calAbs::calAbs(cdbAbs *db) : fDB(db), fTag("unset") {
}


// ----------------------------------------------------------------------
calAbs::calAbs(cdbAbs *db, string tag) :
  fDB(db), fTag(tag) {
}



// ----------------------------------------------------------------------
calAbs::~calAbs() {
  cout << "this is the end of calAbs with tag = " << fTag
       << endl;
}


// ----------------------------------------------------------------------
void calAbs::update(string hash) {
	if (!fDB) {
    cout << "ERROR: no database handle provided" << endl;
    return;
  }

  if (fVerbose > 0) cout << "calAbs::update() hash = " << hash << endl;
  
  if (fTagIOVPayloadMap.find(hash) == fTagIOVPayloadMap.end()) {
    if (fVerbose > 0) cout << "calAbs::getPayload(" << hash
                           << ") not cached, retrieve from DB"
                           << endl;
    auto tbegin = std::chrono::high_resolution_clock::now();
    payload pl = fDB->getPayload(hash);
    auto tend = std::chrono::high_resolution_clock::now();
    if (fPrintTiming) cout << chrono::duration_cast<chrono::microseconds>(tend-tbegin).count()
                           << "us ::timing::" << hash << " getpayload"
                           << endl;
    
    fTagIOVPayloadMap.insert(make_pair(hash, pl));
    calculate(hash);
    fHash = hash;
  } else {
    if (fVerbose > 0) cout << "calAbs::getPayload(" << hash
                           << ") cached."
                           << endl;
  }
  if (hash != fHash) {
    calculate(hash);
  }
}


// ----------------------------------------------------------------------
void calAbs::dump2Root(TDirectory *d) {
  TDirectory *pOld = gFile->CurrentDirectory();
  d->cd();
  for (auto it: fTagIOVPayloadMap) {
    TNamed o(it.first.c_str(), it.second.json().c_str());
    o.Write();
  }
}
  

// ----------------------------------------------------------------------
void calAbs::readPayloadFromFile(string hash, string dir) {
  // -- check whether this payload is stored already and delete if found
  if (fTagIOVPayloadMap.find(hash) == fTagIOVPayloadMap.end()) {
    // -- not found, do nothing
  } else {
    // -- found, delete it
    fTagIOVPayloadMap.erase(hash);
  }
  
  
  // -- initialize with default
  std::stringstream sspl;
  sspl << "(calAbs>  hash = " << hash 
       << " not found)";
  payload pl;
  pl.fComment = sspl.str();
  
  // -- read payload for hash 
  ifstream INS;
  string filename = dir + "/" + hash;
  INS.open(filename);
  if (INS.fail()) {
    cout << "Error failed to open ->" << filename << "<-" << endl;
    return;
  }

  std::stringstream buffer;
  buffer << INS.rdbuf();
  INS.close();
  
  cout << "calAbs::readPayloadFromFile() Read " << filename << " hash ->" << hash << "<-" << endl;
  bsoncxx::document::value doc = bsoncxx::from_json(buffer.str());
  pl.fComment = string(doc["comment"].get_string().value).c_str();
  pl.fHash    = string(doc["hash"].get_string().value).c_str();
  pl.fBLOB    = base64_decode(string(doc["BLOB"].get_string().value));

  fTagIOVPayloadMap.insert(make_pair(hash, pl));
}


// ----------------------------------------------------------------------
void calAbs::writePayloadToFile(string hash, string dir, const payload &pl) {

	auto builder = document{};

  bsoncxx::document::value doc_value = builder
    << "hash" << pl.fHash
    << "comment" << pl.fComment
    << "BLOB" << base64_encode(pl.fBLOB)
    << finalize; 

  // -- JSON
  ofstream JS;
  JS.open(dir + "/" + hash);
  if (JS.fail()) {
    cout << "Error failed to open " << "json/XXXXX" <<  endl;
  }
  JS << bsoncxx::to_json(doc_value.view()) << endl;
  JS.close();
  
}
