#include "calAbs.hh"

#include <iostream>

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
void calAbs::update() {
  int irun = fDB->getRunNumber();
	if (!fDB) {
    cout << "ERROR: no database handle provided" << endl;
    return;
  }
  string hash = fDB->getHash(irun, fTag);

  if (fVerbose > 0) cout << "calAbs::update() hash = " << hash << endl;
  
  if (fTagIOVPayloadMap.find(hash) == fTagIOVPayloadMap.end()) {
    if (fVerbose > 0) cout << "calAbs::getPayload(" << irun
                           << ") not cached, retrieve from DB"
                           << " irun ->" << irun << "<-"
                           << " fTag ->" << fTag << "<-"
                           << " hash ->" << hash << "<-"
                           << endl;
    payload pl = fDB->getPayload(hash);
    fTagIOVPayloadMap.insert(make_pair(hash, pl));
    fHash = hash; 
    calculate();
  } else {
    if (fVerbose > 0) cout << "calAbs::getPayload(" << irun
                           << ") cached. fHash = " << fHash
                           << endl;
  }
  if (hash != fHash) {
    calculate();
  }
}
