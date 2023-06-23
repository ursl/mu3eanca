#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>    // std::find

#include "Mu3eConditions.hh"
#include "cdbAbs.hh"

#include "calPixelAlignment.hh"

using namespace std;

Mu3eConditions* Mu3eConditions::fInstance = 0;


// ----------------------------------------------------------------------
Mu3eConditions* Mu3eConditions::instance(cdbAbs *db) {
  if (0 == fInstance) {
    fInstance = new Mu3eConditions(db);
  }
  return fInstance;
}


// ----------------------------------------------------------------------
Mu3eConditions::Mu3eConditions(cdbAbs *db) : fDB(db) {
  cout << "Mu3eConditions::Mu3eConditions()" << endl;
}


// ----------------------------------------------------------------------
Mu3eConditions::~Mu3eConditions() {
  cout << "Mu3eConditions::~Mu3eConditions()" << endl;
}


// ----------------------------------------------------------------------
calAbs* Mu3eConditions::createClass(string name) {
  vector<string> tags = fDB->getTags();

  string tag("nada");
  for (auto it : tags) {
    cout << "searching " << name << ", looking at " << it << endl; 
    if (string::npos != it.find(name)) {
      tag = it;
      cout << "Mu3eConditions::createClass> found " << tag << endl;
      break;
    }
  }

  if (string::npos != tag.find("nada")) {
    cout << "Mu3eConditions::createClass> ERROR did not find tag containing " << name << endl;
    return 0;
  }

  return createClass(name, tag); 
}


// ----------------------------------------------------------------------
calAbs* Mu3eConditions::createClass(string name, string tag) {
  if (!name.compare("pixelalignment_")) {
    if (fVerbose > 0) cout << "Mu3eConditions::createClass("
                           << name << ", " << tag << "), fDB = "
                           << fDB->getName() 
                           << endl;
    calAbs* a = new calPixelAlignment(fDB, tag);
    registerCalibration(tag, a);
    return a;
  }
  return 0;
}


// ----------------------------------------------------------------------
calAbs* Mu3eConditions::createClassWithDB(string name, string tag, cdbAbs *db) {
  if (!name.compare("pixelalignment_")) {
    if (fVerbose > 0) cout << "Mu3eConditions::createClassWithDB("
                           << name << ", " << db->getName()
                           << ", " << tag << ")"
                           << ", " << db->getName() << ")"
                           << endl;
    calAbs* a = new calPixelAlignment(db, tag);
    registerCalibration(tag, a);
    return a;
  }
  return 0;
}


// ----------------------------------------------------------------------
void Mu3eConditions::registerCalibration(string tag, calAbs *c) {
  cout << "Mu3eConditions::registerCalibration name ->" << c->getName()
       << "<- with tag ->" << tag << "<-"
       << endl;
  fCalibrations.insert(make_pair(tag, c));
  cout << "   done" << endl;
}


// ----------------------------------------------------------------------
void Mu3eConditions::setRunNumber(int runnumber) {
  if (fVerbose > 0)   cout << "Mu3eConditions::setRunNumber(" << runnumber << "), old runnumber = " 
                           << fRunNumber
                           << " fCalibrations.size() = " << fCalibrations.size()
                           << endl;
  
	if (runnumber != fRunNumber) {
		fRunNumber = runnumber;
    // -- call update for all registered calibrations
    //    each calibration will check with its tag/IOV whether an update is required
    for (auto it: fCalibrations) {
      it.second->update(runnumber);
    }
	}
}


// ----------------------------------------------------------------------
calAbs* Mu3eConditions::getCalibration(std::string name) {
  for (auto it: fCalibrations) {
    cout << "  looking at " << it.first << endl;
    if (string::npos != it.first.find(name)) return it.second;
  }  
  return 0;
}


// ----------------------------------------------------------------------
void Mu3eConditions::printCalibrations() {
  for (auto it: fCalibrations) {
    cout << it.second->getName() << endl;
  }
}

