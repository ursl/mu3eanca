#include "calAbs.hh"

#include <iostream>

using namespace std;

// ----------------------------------------------------------------------
calAbs::calAbs(cdb *db) :
  fDB(db),
  fGlobalTag("unset"), fTag("unset") {
}


// ----------------------------------------------------------------------
calAbs::calAbs(cdb *db, string gt) :
  fDB(db), fGlobalTag(gt), fTag("unset") {
}


// ----------------------------------------------------------------------
calAbs::calAbs(cdb *db, string gt, string tag) :
  fDB(db), fGlobalTag(gt), fTag(tag) {
}


// ----------------------------------------------------------------------
calAbs::~calAbs() {
  cout << "this is the end of calAbs with global tag = " << fGlobalTag
       << " with tag = " << fTag
       << endl;
}


// ----------------------------------------------------------------------
string calAbs::getPayload(int irun) {
	if (!fDB) return string("ERROR: no database handle provided");
  string hash = fDB->getHash(irun, fTag);
  if (fTagIovPayloadMap.find(hash) == fTagIovPayloadMap.end()) {
    cout << "calPixel::getPayload(" << irun
         << ") not cached, retrieve from DB"
         << endl;
    string payload = fDB->getPayload(hash);
    fTagIovPayloadMap.insert(make_pair(hash, payload));
    return payload;
  }
  cout << "calPixel::getPayload(" << irun
       << ") cached"
       << endl;
  return fTagIovPayloadMap[hash];
}
