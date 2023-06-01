#include "calAbs.hh"

#include <iostream>

using namespace std;

// ----------------------------------------------------------------------
calAbs::calAbs(cdb *db) : fDB(db), fTag("unset") {
}


// ----------------------------------------------------------------------
calAbs::calAbs(cdb *db, string tag) :
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
  if (fTagIOVPayloadMap.find(hash) == fTagIOVPayloadMap.end()) {
    cout << "calPixel::getPayload(" << irun
         << ") not cached, retrieve from DB"
         << " irun ->" << irun << "<-"
         << " fTag ->" << fTag << "<-"
         << " hash ->" << hash << "<-"
         << endl;
    string payload = fDB->getPayload(hash);
    fTagIOVPayloadMap.insert(make_pair(hash, payload));
  } else {
    cout << "calPixel::getPayload(" << irun
         << ") cached"
         << endl;
  }
  if (hash != fHash) {
    calculate();
  }
  fHash = hash; 
}
