#include "calPixelAlignment.hh"

#include <iostream>

using namespace std;

// ----------------------------------------------------------------------
calPixelAlignment::calPixelAlignment(cdb *db) : calAbs(db) {
  db->registerCalibration("thisshouldnothappen", this);
}


// ----------------------------------------------------------------------
calPixelAlignment::calPixelAlignment(cdb *db, string tag) : calAbs(db, tag) {
	cout << "calPixelAlignment with tag ->" << fTag << "<-" 
			 << endl;
  db->registerCalibration(fTag, this);
}



// ----------------------------------------------------------------------
calPixelAlignment::~calPixelAlignment() {
  cout << "this is the end of calPixelAlignment with tag ->" << fTag << "<-" << endl;
}


// ----------------------------------------------------------------------
void calPixelAlignment::calculate() {
  cout << "calPixelAlignment::calculate() with "
       << "fHash ->" << fHash << "<-"
       << endl;
}

