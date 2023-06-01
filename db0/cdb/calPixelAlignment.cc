#include "calPixelAlignment.hh"

#include <iostream>

using namespace std;

// ----------------------------------------------------------------------
calPixelAlignment::calPixelAlignment(cdb *db) : calAbs(db) {
}


// ----------------------------------------------------------------------
calPixelAlignment::calPixelAlignment(cdb *db, string gt) : calAbs(db, gt) {
	vector<string> ptags = db->getTags(fGlobalTag);
	for (auto it : ptags) {
		if (string::npos != it.find(fPixelAlignmentTag)) {
			fTag = it;
		}
	}
	cout << "calPixelAlignment with global tag ->" << fGlobalTag << "<-" 
			 << " and tag ->" << fTag << "<-" 
			 << endl;
}


// ----------------------------------------------------------------------
calPixelAlignment::calPixelAlignment(cdb *db, string gt, string tag) :
  calAbs(db, gt, tag) {
	cout << "calPixelAlignment with global tag ->" << fGlobalTag << "<-" 
			 << " and tag ->" << fTag << "<-" 
			 << endl;
}


// ----------------------------------------------------------------------
calPixelAlignment::~calPixelAlignment() {
  cout << "this is the end of calPixelAlignment with global tag " << fGlobalTag  << endl;
}

