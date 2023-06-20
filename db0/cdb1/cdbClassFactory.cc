#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>    // std::find

#include "cdbClassFactory.hh"
#include "cdbAbs.hh"

#include "calPixelAlignment.hh"

using namespace std;

cdbClassFactory* cdbClassFactory::fInstance = 0;


// ----------------------------------------------------------------------
cdbClassFactory* cdbClassFactory::instance(cdbAbs *db) {
  if (0 == fInstance) {
    fInstance = new cdbClassFactory(db);
  }
  return fInstance;
}


// ----------------------------------------------------------------------
cdbClassFactory::cdbClassFactory(cdbAbs *db) : fDB(db) {
  cout << "cdbClassFactory::cdbClassFactory()" << endl;
}


// ----------------------------------------------------------------------
cdbClassFactory::~cdbClassFactory() {
  cout << "cdbClassFactory::~cdbClassFactory()" << endl;
}


// ----------------------------------------------------------------------
calAbs* cdbClassFactory::createClass(string name) {
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
calAbs* cdbClassFactory::createClass(string name, string tag) {
  if (!name.compare("pixelalignment_")) {
    if (fVerbose > 0) cout << "cdbClassFactory::createClass("
                           << name << ", " << tag << ")" << endl;
    return new calPixelAlignment(fDB, tag);
  }
  return 0;
}

// ----------------------------------------------------------------------
calAbs* cdbClassFactory::createClassWithDB(string name, string tag, cdbAbs *db) {
  if (!name.compare("pixelalignment_")) {
    if (fVerbose > 0) cout << "cdbClassFactory::createClassWithDB("
                           << name << ", " << db->getName()
                           << ", " << tag << ")"
                           << endl;
    return new calPixelAlignment(db, tag);
  }
  return 0;
}
