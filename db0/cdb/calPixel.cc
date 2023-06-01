#include "calPixel.hh"

#include <iostream>

using namespace std;

// ----------------------------------------------------------------------
calPixel::calPixel(cdb *db) : calAbs(db) {
}


// ----------------------------------------------------------------------
calPixel::calPixel(cdb *db, string gt) : calAbs(db, gt) {
	vector<string> ptags = db->getTags(fGlobalTag);
  cout << "calPixel::calPixel(db, gt): fPixelTag = " << fPixelTag << " : ";
  db->print(ptags);
	for (auto it : ptags) {
		if (string::npos != it.find(fPixelTag)) {
			fTag = it;
		}
	}
	cout << "calPixel with global tag ->" << fGlobalTag << "<-" 
			 << " and tag ->" << fTag << "<-" 
			 << endl;
}


// ----------------------------------------------------------------------
calPixel::calPixel(cdb *db, string gt, string tag) :
  calAbs(db, gt, tag) {
	cout << "calPixel with global tag ->" << fGlobalTag << "<-" 
			 << " and tag ->" << fTag << "<-" 
			 << endl;
}


// ----------------------------------------------------------------------
calPixel::~calPixel() {
  cout << "this is the end of calPixel with global tag " << fGlobalTag  << endl;
}

