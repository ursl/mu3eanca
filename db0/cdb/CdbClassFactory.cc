#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "CdbClassFactory.hh"
#include "cdb.hh"

#include "calPixel.hh"

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
calAbs* CdbClassFactory::createClass(string name, string tag) {
  if (!name.compare("calPixel")) {
    return new calPixel(fDB, tag);
  }
  return 0;
}

// ----------------------------------------------------------------------
calAbs* CdbClassFactory::createClassWithDB(string name, cdb *db, string tag) {
  if (!name.compare("calPixel")) {
    return new calPixel(db, tag);
  }
  return 0;
}
