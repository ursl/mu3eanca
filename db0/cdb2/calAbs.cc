#include "calAbs.hh"

#include "base64.hh"
#include "cdbUtil.hh"

#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>


using namespace std;

// ----------------------------------------------------------------------
calAbs::calAbs(cdbAbs *db) : fDB(db), fTag("unset"), fError("unset") {
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

  if (fVerbose > 4) cout << "calAbs::update() hash = " << hash << endl;

  if (fTagIOVPayloadMap.find(hash) == fTagIOVPayloadMap.end()) {
    if (fVerbose > 2) cout << "calAbs::getPayload(" << hash
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
    if (fVerbose > 4) cout << "calAbs::getPayload(" << hash
                             << ") cached."
                             << endl;
  }
  if (hash != fHash) {
    calculate(hash);
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
    cout << "calAbs::readPayloadFromFile> Error failed to open ->" << filename << "<-" << endl;
    fError = "Error: file not found";
    return;
  }

  std::stringstream buffer;
  buffer << INS.rdbuf();
  INS.close();
  fPayloadString = buffer.str();
  pl.fHash    = jsonGetString(fPayloadString, "hash");
  pl.fComment = jsonGetString(fPayloadString, "comment");
  pl.fBLOB    = base64_decode(jsonGetString(fPayloadString, "BLOB"));
  pl.fSchema  = jsonGetString(fPayloadString, "schema");

  fTagIOVPayloadMap.insert(make_pair(hash, pl));
  if (fVerbose > 0) cout << "calAbs::readPayloadFromFile() Inserted "
                           << " hash ->" << hash << "<-"
                           << " blob.size() = " << pl.fBLOB.size()
                           << " into fTagIOVPayloadMap"
                           << endl;
}


// ----------------------------------------------------------------------
void calAbs::writePayloadToFile(string hash, string dir) {

  payload pl = fTagIOVPayloadMap[hash];
  pl.fDate = timeStamp();
  stringstream sstr;
  sstr << pl.json();
  // sstr << "{ \"hash\" : \"" << hash << "\", ";
  // sstr << "\"comment\" : " << pl.fComment << "\", ";
  // sstr << "\"schema\" : " << pl.fSchema << "\", ";
  // sstr << "\"date\" : " << pl.fDate << "\", ";
  // sstr << "\"BLOB\" : \"" << base64_encode(pl.fBLOB) << "\" }" << endl;

  // -- JSON
  ofstream JS;
  JS.open(dir + "/" + hash);
  if (JS.fail()) {
    cout << "calAbs::writePayloadToFile> Error failed to open "
         << dir << "/" << hash
         << endl;
  }
  JS << sstr.str();
  JS.close();

}


// ----------------------------------------------------------------------
void calAbs::writePayloadToFile(string hash, string dir, payload &pl) {

  pl.fDate = timeStamp();
  stringstream sstr;
  sstr << pl.json();
  // sstr << "{ \"hash\" : \"" << hash << "\", \"comment\" : ";
  // sstr << "\"" << pl.fComment << "\", ";
  // sstr << "\"BLOB\" : \"" << base64_encode(pl.fBLOB) << "\" }" << endl;

  // -- JSON
  ofstream JS;
  JS.open(dir + "/" + hash);
  if (JS.fail()) {
    cout << "calAbs::writePayloadToFile> Error failed to open " << dir << "/" << hash <<  endl;
  }
  JS << sstr.str();
  JS.close();

}


// ----------------------------------------------------------------------
void calAbs::insertPayload(string hash, payload pl) {
  if (fTagIOVPayloadMap.find(hash) == fTagIOVPayloadMap.end()) {
    fTagIOVPayloadMap.insert(make_pair(hash, pl));
    calculate(hash);
    fHash = hash;
    cout << "hash ->" << hash << "<- inserted into fTagIOVPayloadMap" << endl;
  } else {
    cout << "hash ->" << hash << "<- already in fTagIOVPayloadMap" << endl;
  }
}
