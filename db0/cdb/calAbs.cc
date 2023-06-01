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
