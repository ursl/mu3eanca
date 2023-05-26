#include "calAbs.hh"

#include <iostream>

using namespace std;

// ----------------------------------------------------------------------
calAbs::calAbs(cdb *db) : fDB(db) {
}


// ----------------------------------------------------------------------
calAbs::calAbs(cdb *db, string gt) : fDB(db), fGlobalTag(gt) {
}


// ----------------------------------------------------------------------
calAbs::~calAbs() {
  cout << "this is the end of calAbs with global tag = " << fGlobalTag << endl;
}
