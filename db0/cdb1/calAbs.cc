#include "calAbs.hh"

#include "TFile.h"

#include <chrono>
#include <iostream>
#include <sstream>

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
  
