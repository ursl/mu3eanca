#include "calAbs.hh"

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
    payload pl = fDB->getPayload(hash);
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

