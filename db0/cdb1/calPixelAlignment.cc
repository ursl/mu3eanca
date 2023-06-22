#include "calPixelAlignment.hh"

#include <iostream>
#include <sstream>

#include "cdbUtil.hh"

using namespace std;

// ----------------------------------------------------------------------
calPixelAlignment::calPixelAlignment(cdbAbs *db) : calAbs(db) {
  db->registerCalibration("thisshouldnothappen", this);
}


// ----------------------------------------------------------------------
calPixelAlignment::calPixelAlignment(cdbAbs *db, string tag) : calAbs(db, tag) {
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
  fMapConstants.clear();
  string spl = fTagIOVPayloadMap[fHash].fBLOB;
  vector<string> tokens = split(spl, ',');
  for (unsigned int i = 0; i < tokens.size(); i += 16) {
    constants a; 
    a.id        = stoi(tokens[i]);
    a.vx        = stod(tokens[i+1]);
    a.vy        = stod(tokens[i+2]);
    a.vz        = stod(tokens[i+3]);
    a.rowx      = stod(tokens[i+4]);
    a.rowy      = stod(tokens[i+5]);
    a.rowz      = stod(tokens[i+6]);
    a.colx      = stod(tokens[i+7]);
    a.coly      = stod(tokens[i+8]);
    a.colz      = stod(tokens[i+9]);
    a.nrow      = stoi(tokens[i+10]);
    a.ncol      = stoi(tokens[i+11]);
    a.width     = stod(tokens[i+12]);
    a.length    = stod(tokens[i+13]);
    a.thickness = stod(tokens[i+14]);
    a.pixelSize = stod(tokens[i+15]);

    fMapConstants.insert(make_pair(a.id, a));
  }
}

