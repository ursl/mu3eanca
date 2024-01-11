#include "Mu3eCalFactory.hh"

#include <algorithm>
#include <chrono>

#include "calPixelAlignment.hh"
#include "calFibreAlignment.hh"
#include "calMppcAlignment.hh"
#include "calTileAlignment.hh"

using namespace std;

Mu3eCalFactory* Mu3eCalFactory::fInstance = 0;


// ----------------------------------------------------------------------
Mu3eCalFactory* Mu3eCalFactory::instance(std::string gt, cdbAbs *db) {
  if (0 == fInstance) {
    fInstance = new Mu3eCalFactory(gt, db);
  }
  return fInstance;
}


// ----------------------------------------------------------------------
Mu3eCalFactory::Mu3eCalFactory(std::string gt, cdbAbs *db) : fGT(gt), fDB(db) {
  if (fVerbose > 0) cout << "Mu3eCalFactory::Mu3eCalFactory(std::string gt, cdbAbs *db) " << endl;
  if (fDB) {
    fTags       = fDB->readTags(fGT);
  }

}


// ----------------------------------------------------------------------
Mu3eCalFactory::~Mu3eCalFactory() {
  cout << "Mu3eCalFactory::~Mu3eCalFactory()" << endl;
}


// ----------------------------------------------------------------------
calAbs* Mu3eCalFactory::createClass(string name) {
  string tag("nada");
  for (auto it : fTags) {
    if (fVerbose > 0) cout << "Mu3eCalFactory::createClass> searching " << name << ", looking at " << it << endl; 
    if (string::npos != it.find(name)) {
      tag = it;
      if (fVerbose > 0) cout << "Mu3eCalFactory::createClass> found " << tag << endl;
      break;
    }
  }

  if (string::npos != tag.find("nada")) {
    if (fVerbose > 0) cout << "Mu3eCalFactory::createClass> ERROR did not find tag containing " << name << endl;
    return 0;
  }

  return createClassWithDB(name, tag, fDB); 
}


// ----------------------------------------------------------------------
calAbs* Mu3eCalFactory::createClass(string name, string tag) {
  return createClassWithDB(name, tag, fDB); 
}


// ----------------------------------------------------------------------
calAbs* Mu3eCalFactory::createClassWithDB(string name, string tag, cdbAbs *db) {
  calAbs* a(0);
  auto tbegin = std::chrono::high_resolution_clock::now();
  if (!name.compare("pixelalignment_")) {
    a = new calPixelAlignment(db, tag);
    if (fVerbose > 0) cout << "Mu3eCalFactory::createClassWithDB("
                           << name << ", " << db->getName()
                           << ", " << tag << ")"
                           << ", " << db->getName() << ")"
                           << endl;
  } else if (!name.compare("fibrealignment_")) {
    a = new calFibreAlignment(db, tag);
    if (fVerbose > 0) cout << "Mu3eCalFactory::createClassWithDB("
                           << name << ", " << db->getName()
                           << ", " << tag << ")"
                           << ", " << db->getName() << ")"
                           << endl;
  } else if (!name.compare("mppcalignment_")) {
    a = new calMppcAlignment(db, tag);
    if (fVerbose > 0) cout << "Mu3eCalFactory::createClassWithDB("
                           << name << ", " << db->getName()
                           << ", " << tag << ")"
                           << ", " << db->getName() << ")"
                           << endl;
  } else if (!name.compare("tilealignment_")) {
    a = new calTileAlignment(db, tag);
    if (fVerbose > 0) cout << "Mu3eCalFactory::createClassWithDB("
                           << name << ", " << db->getName()
                           << ", " << tag << ")"
                           << ", " << db->getName() << ")"
                           << endl;
  } else {
    cout << "ERROR: " << name
         << " is an unknown class. Nothing registered in Mu3Conditions"
         << endl;
    return 0;
  }

  return a;
}

