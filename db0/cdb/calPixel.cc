#include "calPixel.hh"

#include <iostream>

using namespace std;

// ----------------------------------------------------------------------
calPixel::calPixel(cdb *db) : calAbs(db) {
  //  db->registerCalibration("thisshouldnothappen", this);
}


// ----------------------------------------------------------------------
calPixel::calPixel(cdb *db, string tag) : calAbs(db, tag) {
	cout << "calPixel with tag ->" << fTag << "<-" 
			 << endl;
  db->registerCalibration(fTag, this);
}


// ----------------------------------------------------------------------
calPixel::~calPixel() {
  cout << "this is the end of calPixel with tag ->" << fTag << "<-" << endl;
}


// ----------------------------------------------------------------------
void calPixel::calculate() {
  if (fVerbose > 1) cout << "calPixel::calculate() with "
                         << "fHash ->" << fHash << "<-"
                         << endl;
}
