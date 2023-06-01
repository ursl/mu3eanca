#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "CdbClassFactory.hh"

using namespace std;

CdbClassFactory* CdbClassFactory::fInstance = 0;


// ----------------------------------------------------------------------
CdbClassFactory* CdbClassFactory::instance() {
  if (0 == fInstance) {
    fInstance = new CdbClassFactory;
  }
  return CdbClassFactory;
}


// ----------------------------------------------------------------------
CdbClassFactory::CdbClassFactory() {
  cout << "CdbClassFactory::CdbClassFactory()" << endl;
}


// ----------------------------------------------------------------------
CdbClassFactory::~CdbClassFactory() {
  cout << "CdbClassFactory::~CdbClassFactory()" << endl;
}


// ----------------------------------------------------------------------
calAbs* CdbClassFactory::createClass(string name, cdb *db, string tag) {
  if (!name.compare("calPixel")) {
    return new calPixel(db, tag);
  }
  return 0;
}
