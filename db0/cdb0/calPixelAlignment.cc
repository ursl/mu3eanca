#include "calPixelAlignment.hh"

#include <iostream>
#include <sstream>

#include "util/util.hh"

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
  fMapConstants.clear();
  string spl = fTagIOVPayloadMap[fHash].fBLOB;
  vector<string> tokens = split(spl, ',');
  constants a; 
  for (unsigned int i = 0; i < tokens.size(); i += 10) {
    a.id = stoi(tokens[i]);
    a.vx = stod(tokens[i+1]);
    a.vy = stod(tokens[i+2]);
    a.vz = stod(tokens[i+3]);
    a.rowx = stod(tokens[i+4]);
    a.rowy = stod(tokens[i+5]);
    a.rowz = stod(tokens[i+6]);
    a.colx = stod(tokens[i+7]);
    a.coly = stod(tokens[i+8]);
    a.colz = stod(tokens[i+9]);
    fMapConstants.insert(make_pair(a.id, a));
  }
}

