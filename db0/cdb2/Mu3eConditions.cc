#include <iostream>
#include <chrono>
#include <string>
#include <vector>
#include <map>
#include <algorithm>    // std::find
#include <sstream>

#include "Mu3eConditions.hh"
#include "cdbAbs.hh"

#include "Mu3eCalFactory.hh"

#include "calPixelAlignment.hh"
#include "calFibreAlignment.hh"
#include "calMppcAlignment.hh"
#include "calTileAlignment.hh"

using namespace std;

Mu3eConditions* Mu3eConditions::fInstance = 0;


// ----------------------------------------------------------------------
Mu3eConditions* Mu3eConditions::instance(std::string gt, cdbAbs *db) {
  if (0 == fInstance) {
    fInstance = new Mu3eConditions(gt, db);
  }
  return fInstance;
}


// ----------------------------------------------------------------------
Mu3eConditions::Mu3eConditions(std::string gt, cdbAbs *db) : fDB(db), fGT(gt) {
  cout << "Mu3eConditions::Mu3eConditions(" << gt
       << ", " << (fDB? fDB->getName(): "no DB")
       << ")" << endl;
  if (fDB) {
    fGlobalTags = fDB->readGlobalTags(fGT);
    fTags       = fDB->readTags(fGT);
    fIOVs       = fDB->readIOVs(fTags);
  }
}


// ----------------------------------------------------------------------
Mu3eConditions::~Mu3eConditions() {
  cout << "Mu3eConditions::~Mu3eConditions()" << endl;
}


// ----------------------------------------------------------------------
calAbs* Mu3eConditions::createClass(string name) {
  string tag("nada");
  for (auto it : fTags) {
    cout << "Mu3eConditions::createClass> searching " << name << ", looking at " << it << endl; 
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

  return createClassWithDB(name, tag, fDB); 
}


// ----------------------------------------------------------------------
calAbs* Mu3eConditions::createClass(string name, string tag) {
  return createClassWithDB(name, tag, fDB); 
}


// ----------------------------------------------------------------------
calAbs* Mu3eConditions::createClassWithDB(string name, string tag, cdbAbs *db) {
  calAbs* a(0);
  auto tbegin = std::chrono::high_resolution_clock::now();
  
  Mu3eCalFactory *mcf = Mu3eCalFactory::instance(fGT, db);
  a = mcf->createClassWithDB(name, tag, db);
  
  // if (!name.compare("pixelalignment_")) {
  //   a = new calPixelAlignment(db, tag);
  //   if (fVerbose > 0) cout << "Mu3eConditions::createClassWithDB("
  //                          << name << ", " << db->getName()
  //                          << ", " << tag << ")"
  //                          << ", " << db->getName() << ")"
  //                          << endl;
  // } else if (!name.compare("fibrealignment_")) {
  //   a = new calFibreAlignment(db, tag);
  //   if (fVerbose > 0) cout << "Mu3eConditions::createClassWithDB("
  //                          << name << ", " << db->getName()
  //                          << ", " << tag << ")"
  //                          << ", " << db->getName() << ")"
  //                          << endl;
  // } else if (!name.compare("mppcalignment_")) {
  //   a = new calMppcAlignment(db, tag);
  //   if (fVerbose > 0) cout << "Mu3eConditions::createClassWithDB("
  //                          << name << ", " << db->getName()
  //                          << ", " << tag << ")"
  //                          << ", " << db->getName() << ")"
  //                          << endl;
  // } else if (!name.compare("tilealignment_")) {
  //   a = new calTileAlignment(db, tag);
  //   if (fVerbose > 0) cout << "Mu3eConditions::createClassWithDB("
  //                          << name << ", " << db->getName()
  //                          << ", " << tag << ")"
  //                          << ", " << db->getName() << ")"
  //                          << endl;
  // } else {
  //   cout << "ERROR: " << name
  //        << " is an unknown class. Nothing registered in Mu3Conditions"
  //        << endl;
  //   return 0;
  // }
  auto tend = std::chrono::high_resolution_clock::now();
  if (fPrintTiming) cout << chrono::duration_cast<chrono::microseconds>(tend-tbegin).count()
                         << "us ::timing::" << tag << " ctor"
                         << endl;

  if (fPrintTiming) a->setPrintTiming(fPrintTiming);
  tbegin = chrono::high_resolution_clock::now();
  a->setIOVs(getIOVs(tag));
  registerCalibration(tag, a);
  tend = chrono::high_resolution_clock::now();
  if (fPrintTiming) cout << chrono::duration_cast<chrono::microseconds>(tend-tbegin).count()
                         << "us ::timing " << tag << " registerCalibration"
                         << endl;
  return a;
}


// ----------------------------------------------------------------------
void Mu3eConditions::registerCalibration(string tag, calAbs *c) {
  cout << "Mu3eConditions::registerCalibration> name ->" << c->getName()
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

  auto tbegin = chrono::high_resolution_clock::now();
  auto tend = chrono::high_resolution_clock::now();

	if (runnumber != fRunNumber) {
		fRunNumber = runnumber;
    // -- call update for all registered calibrations
    //    each calibration will check with its tag/IOV whether an update is required
    for (auto it: fCalibrations) {
      cout << "Mu3eConditions::setRunNumber> call update runnumber = " << runnumber
           << " tag = " << it.first
           << endl;
      tbegin = chrono::high_resolution_clock::now();
      it.second->update(getHash(runnumber, it.first));
      tend = chrono::high_resolution_clock::now();
      if (fPrintTiming) cout << chrono::duration_cast<chrono::microseconds>(tend-tbegin).count()
                             << "us ::timing::" << it.first << " update"
                             << endl;
    }
	}
}


// ----------------------------------------------------------------------
calAbs* Mu3eConditions::getCalibration(std::string name) {
  for (auto it: fCalibrations) {
    cout << "  Mu3eConditions::getCalibration> looking at " << it.first << endl;
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



// ----------------------------------------------------------------------
int Mu3eConditions::whichIOV(int runnumber, string tag) {
	int iov(-1);
  for (auto it : fIOVs[tag]) {
    if (it > runnumber) {
			return iov;
    } else {
			iov = it;
		}
  }
	return iov; 
}


// ----------------------------------------------------------------------
string Mu3eConditions::getHash(int runnumber, string tag) {
  int iov = whichIOV(runnumber, tag);
  // -- hash is a misnomer here
  std::stringstream ssHash;
  ssHash << "tag_" << tag << "_iov_" << iov;
  if (fVerbose > 4) cout << "calAbs::getHash(" << runnumber << ", " << tag << ") = " << ssHash.str() << endl;
  return ssHash.str();
}
