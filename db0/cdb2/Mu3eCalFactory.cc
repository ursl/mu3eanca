#include "Mu3eCalFactory.hh"

#include "calPixelAlignment.hh"
#include "calFibreAlignment.hh"
#include "calMppcAlignment.hh"
#include "calTileAlignment.hh"
#include "calDetConfV1.hh"
#include "calDetSetupV1.hh"
#include "calEventStuffV1.hh"
#include "calPixelQualityLM.hh"
#include "calPixelTimeCalibration.hh"
#include "calTileQuality.hh"
#include "calPixelEfficiency.hh"
#include "calFibreQuality.hh"
#include "calPixelMask.hh"
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
  if (name.find("pixelalignment_") != string::npos) {
    a = new calPixelAlignment(db, tag);
  } else if (name.find("fibrealignment_") != string::npos) {
    a = new calFibreAlignment(db, tag);
  } else if (name.find("mppcalignment_") != string::npos)  {
    a = new calMppcAlignment(db, tag);
  } else if (name.find("tilealignment_") != string::npos)  {
    a = new calTileAlignment(db, tag);
  } else if (name.find("detconfv1_") != string::npos)  {
    a = new calDetConfV1(db, tag);
  } else if (name.find("detsetupv1_") != string::npos)  {
    a = new calDetSetupV1(db, tag);
  } else if (name.find("eventstuffv1_") != string::npos)  {
    a = new calEventStuffV1(db, tag);
  } else if (name.find("pixelqualitylm_") != string::npos)  {
    a = new calPixelQualityLM(db, tag);
  } else if (name.find("pixeltimecalibration_") != string::npos)  {
    a = new calPixelTimeCalibration(db, tag);
  } else if (name.find("tilequality_") != string::npos)  {
    a = new calTileQuality(db, tag);
  } else if (name.find("fibrequality_") != string::npos)  {
    a = new calFibreQuality(db, tag);
  } else if (name.find("pixelefficiency_") != string::npos)  {
    a = new calPixelEfficiency(db, tag);
  } else if (name.find("pixelmask_") != string::npos)  {
    a = new calPixelMask(db, tag);
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
  if (hash.find("pixelalignment_") != string::npos) {
    a = new calPixelAlignment();
  } else if (hash.find("fibrealignment_") != string::npos) {
    a = new calFibreAlignment();
  } else if (hash.find("mppcalignment_") != string::npos)  {
    a = new calMppcAlignment();
  } else if (hash.find("tilealignment_") != string::npos)  {
    a = new calTileAlignment();
  } else if (hash.find("detconfv1_") != string::npos)  {
    a = new calDetConfV1();
  } else if (hash.find("detsetupv1_") != string::npos)  {
    a = new calDetSetupV1();
  } else if (hash.find("eventstuffv1_") != string::npos)  {
    a = new calEventStuffV1();
  } else if (hash.find("pixelqualitylm_") != string::npos)  {
    a = new calPixelQualityLM();
  } else if (hash.find("pixeltimecalibration_") != string::npos)  {
    a = new calPixelTimeCalibration();
  } else if (hash.find("tilequality_") != string::npos)  {
    a = new calTileQuality();
  } else if (hash.find("fibrequality_") != string::npos)  {
    a = new calFibreQuality();
  } else if (hash.find("pixelefficiency_") != string::npos)  {
    a = new calPixelEfficiency();
  } else if (hash.find("pixelmask_") != string::npos)  {
    a = new calPixelMask();
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
