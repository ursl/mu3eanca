#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>    // std::find

#include "CdbClassFactory.hh"
#include "cdb.hh"

#include "calPixel.hh"
#include "calPixelAlignment.hh"

using namespace std;

CdbClassFactory* CdbClassFactory::fInstance = 0;


// ----------------------------------------------------------------------
CdbClassFactory* CdbClassFactory::instance(cdb *db) {
  if (0 == fInstance) {
    fInstance = new CdbClassFactory(db);
  }
  return fInstance;
}


// ----------------------------------------------------------------------
CdbClassFactory::CdbClassFactory(cdb *db) : fDB(db) {
  cout << "CdbClassFactory::CdbClassFactory()" << endl;
}


// ----------------------------------------------------------------------
CdbClassFactory::~CdbClassFactory() {
  cout << "CdbClassFactory::~CdbClassFactory()" << endl;
}


// ----------------------------------------------------------------------
calAbs* CdbClassFactory::createClass(string name) {
  vector<string> tags = fDB->getTags();

  string tag("nada");
  for (auto it : tags) {
    cout << "searching " << name << ", looking at " << it << endl; 
    if (string::npos != it.find(name)) {
      tag = it;
      cout << "CdbClassFactory::createClass> found " << tag << endl;
      break;
    }
  }

  if (string::npos != tag.find("nada")) {
    cout << "CdbClassFactory::createClass> ERROR did not find tag containing " << name << endl;
    return 0;
  }


  return createClass(name, tag); 
}


// ----------------------------------------------------------------------
calAbs* CdbClassFactory::createClass(string name, string tag) {
  if (!name.compare("pixel_")) {
    if (fVerbose > 0) cout << "CdbClassFactory::createClass("
                           << name << ", " << tag << ")" << endl;
    return new calPixel(fDB, tag);
  }
  else if (!name.compare("pixelalignment_")) {
    if (fVerbose > 0) cout << "CdbClassFactory::createClass("
                           << name << ", " << tag << ")" << endl;
    return new calPixelAlignment(fDB, tag);
  }
  return 0;
}

// ----------------------------------------------------------------------
calAbs* CdbClassFactory::createClassWithDB(string name, string tag,cdb *db) {
  if (!name.compare("calPixel")) {
    if (fVerbose > 0) cout << "CdbClassFactory::createClassWithDB("
                           << name << ", " << db->getName()
                           << ", " << tag << ")"
                           << endl;
    return new calPixel(db, tag);
  }
  return 0;
}
