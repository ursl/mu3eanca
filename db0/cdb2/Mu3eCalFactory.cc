#include "Mu3eCalFactory.hh"

#include "calPixelAlignment.hh"
#include "calFibreAlignment.hh"
#include "calMppcAlignment.hh"
#include "calTileAlignment.hh"
#include "calDetConfV1.hh"
#include "calDetSetupV1.hh"
#include "calPixelQualityLM.hh"
#include "calPixelTimeCalibration.hh"
#include "calTileQuality.hh"
#include "calPixelEfficiency.hh"
//#include "calFibreQuality.hh"
#include <algorithm>
#include <chrono>

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
  fVerbose = 10;
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
  } else if (!name.compare("fibrealignment_")) {
    a = new calFibreAlignment(db, tag);
  } else if (!name.compare("mppcalignment_"))  {
    a = new calMppcAlignment(db, tag);
  } else if (!name.compare("tilealignment_"))  {
    a = new calTileAlignment(db, tag);
  } else if (!name.compare("detconfv1_"))  {
    a = new calDetConfV1(db, tag);
  } else if (!name.compare("detsetupv1_"))  {
    a = new calDetSetupV1(db, tag);
  } else if (!name.compare("pixelqualitylm_"))  {
    a = new calPixelQualityLM(db, tag);
  } else if (!name.compare("pixeltimecalibration_"))  {
    a = new calPixelTimeCalibration(db, tag);
  } else if (!name.compare("tilequality_"))  {
    a = new calTileQuality(db, tag);
//  } else if (!name.compare("fibrequality_"))  {
//    a = new calFibreQuality(db, tag);
  } else if (!name.compare("pixelefficiency_"))  {
    a = new calPixelEfficiency(db, tag);
  } else {
    cout << "ERROR: " << name
         << " is an unknown class. Nothing registered in Mu3Conditions"
         << endl;
    return 0;
  }
  
  if (fVerbose > 0) cout << "Mu3eCalFactory::createClassWithDB("
                           << name << ", " << db->getName()
                           << ", " << tag << ")"
                           << ", " << db->getName() << ") " << (a? " success" : "failure")
                           << endl;
                           
  return a;
}


// ----------------------------------------------------------------------
calAbs* Mu3eCalFactory::createClassFromFile(string hash, string dir) {
  calAbs* a(0);
  if (!hash.compare("pixelalignment_")) {
    a = new calPixelAlignment();
  } else if (!hash.compare("fibrealignment_")) {
    a = new calFibreAlignment();
  } else if (!hash.compare("mppcalignment_"))  {
    a = new calMppcAlignment();
  } else if (!hash.compare("tilealignment_"))  {
    a = new calTileAlignment();
  } else if (!hash.compare("detconfv1_"))  {
    a = new calDetConfV1();
  } else if (!hash.compare("detsetupv1_"))  {
    a = new calDetSetupV1();
  } else if (!hash.compare("pixelqualitylm_"))  {
    a = new calPixelQualityLM();
  } else if (!hash.compare("pixeltimecalibration_"))  {
    a = new calPixelTimeCalibration();
  } else if (!hash.compare("tilequality_"))  {
    a = new calTileQuality();
//  } else if (!hash.compare("fibrequality_"))  {
//    a = new calFibreQuality();
  } else if (!hash.compare("pixelefficiency_"))  {
    a = new calPixelEfficiency();
  } else {
    cout << "ERROR: " << hash
         << " indicates an unknown class. Nothing known in Mu3CalFactory"
         << endl;
    return 0;
  }
  
  a->readPayloadFromFile(hash, dir);
  if (a->getError() == "Error: file not found") {
    cout << "file ->" << dir << "/" << hash << "<- not found" << endl;
    return 0;
  }
  a->calculate(hash);
  
  if (fVerbose > 0) cout << "Mu3eCalFactory::createClassFromFile("
                           << hash  << ", " << dir << ")"
                           << ") " << (a? " success" : "failure")
                           << endl;
                           
  return a;
}
