#include "calPixel.hh"

#include <iostream>

using namespace std;

// ----------------------------------------------------------------------
calPixel::calPixel(cdb *db) : calAbs(db) {
}


// ----------------------------------------------------------------------
calPixel::calPixel(cdb *db, string gt) : calAbs(db, gt) {
	vector<string> ptags = db->getTags(fGlobalTag);
	for (auto it : ptags) {
		if (string::npos != it.find("pixel")) {
			fPixelTag = it;
		}
	}
	cout << "calPixel with global tag ->" << fGlobalTag << "<-" 
			 << " and tag ->" << fPixelTag << "<-" 
			 << endl;
}


// ----------------------------------------------------------------------
calPixel::~calPixel() {
  cout << "this is the end of calPixel with global tag " << fGlobalTag  << endl;
}


// ----------------------------------------------------------------------
string calPixel::getPayload(int irun) {
	if (!fDB) return string("ERROR: no database handle provided");
	return fDB->getPayload(irun, fPixelTag);
}
