#include <iostream>
#include <algorithm>    // std::find
#include <chrono>
#include <string>
#include <vector>
#include <map>
#include <sstream>

#include "cdbAbs.hh"

#include "Mu3eConditions.hh"
#include "Mu3eCalFactory.hh"

#include "calAbs.hh"

using namespace std;

Mu3eConditions* Mu3eConditions::fInstance = 0;

// ----------------------------------------------------------------------
Mu3eConditions* Mu3eConditions::instance(std::string gt, cdbAbs *db) {
  if (0 == fInstance) {
    fInstance = new Mu3eConditions(gt, db);
  } 

  if (gt != "unset" && gt != fInstance->fGT) {
    cout << "Mu3eConditions::instance(" << gt<< ", " << (db? db->getName(): "no DB")
         << ") called with global tag different from the initial one."
         << endl;
    cout << "This is a mistake. You will get the one and only Mu3eConditions::instance(" 
         << fInstance->fGT << ", " << fInstance->fDB->getName() << ")"
         << endl;
  }
  if (db && db->getName() != fInstance->fDB->getName()) {
    cout << "Mu3eConditions::instance(" << gt<< ", " << (db? db->getName(): "no DB")
         << ") called with different database."
         << endl;
    cout << "This is a mistake. You will get the one and only Mu3eConditions::instance(" 
         << fInstance->fGT << ", " << fInstance->fDB->getName() << ")"
         << endl;
  }
  return fInstance;
}


// ----------------------------------------------------------------------
Mu3eConditions::Mu3eConditions(std::string gt, cdbAbs *db) : fDB(db), fGT(gt) {
  cout << "Mu3eConditions::Mu3eConditions(" << gt
       << ", " << (fDB? fDB->getName(): "no DB")
       << ")" << endl;

  if (fDB) {
    fGlobalTags = fDB->readGlobalTags();
    fTags       = fDB->readTags(fGT);
    fIOVs       = fDB->readIOVs(fTags);
  } else { 
    // -- safe guard to keep Mu3eConditions::setRunNumber free of DB checks in mu3e code
    return;
  }

  // -- setup basic classes
  int verbose(0); 
  calAbs *cal = createClass("pixelalignment_");
  cal->setVerbosity(verbose);
  cal = createClass("fibrealignment_");
  cal->setVerbosity(verbose);
  cal = createClass("mppcalignment_");
  cal->setVerbosity(verbose);
  cal = createClass("tilealignment_");
  cal->setVerbosity(verbose);
}


// ----------------------------------------------------------------------
Mu3eConditions::~Mu3eConditions() {
  cout << "Mu3eConditions::~Mu3eConditions()" << endl;
}


// ----------------------------------------------------------------------
calAbs* Mu3eConditions::createClass(string name) {
  string tag("nada");
  for (auto it : fTags) {
    // cout << "Mu3eConditions::createClass> searching " << name << ", looking at " << it << endl; 
    if (string::npos != it.find(name)) {
      tag = it;
      // cout << "Mu3eConditions::createClass> found " << tag << endl;
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
  //dbx  auto tbegin = std::chrono::high_resolution_clock::now();
  
  Mu3eCalFactory *mcf = Mu3eCalFactory::instance(fGT, db);
  a = mcf->createClassWithDB(name, tag, db);
  
  //dbx  auto tend = std::chrono::high_resolution_clock::now();
  //dbx  if (fPrintTiming) cout << chrono::duration_cast<chrono::microseconds>(tend-tbegin).count()
  //dbx                         << "us ::timing::" << tag << " ctor"
  //dbx                         << endl;

  if (fPrintTiming) a->setPrintTiming(fPrintTiming);
  //dbx  tbegin = chrono::high_resolution_clock::now();
  a->setIOVs(getIOVs(tag));
  registerCalibration(tag, a);
  //dbx tend = chrono::high_resolution_clock::now();
  //dbx if (fPrintTiming) cout << chrono::duration_cast<chrono::microseconds>(tend-tbegin).count()
  //dbx << "us ::timing " << tag << " registerCalibration"
  //dbx << endl;
  return a;
}


// ----------------------------------------------------------------------
void Mu3eConditions::registerCalibration(string tag, calAbs *c) {
  cout << "Mu3eConditions::registerCalibration> name ->" << c->getName()
       << "<- with tag ->" << tag << "<-";
  fCalibrations.insert(make_pair(tag, c));
  cout << " ...  done" << endl;
}


// ----------------------------------------------------------------------
void Mu3eConditions::setRunNumber(int runnumber) {
  if (fVerbose > 2)   cout << "Mu3eConditions::setRunNumber(" << runnumber << "), old runnumber = " 
                           << fRunNumber
                           << " fCalibrations.size() = " << fCalibrations.size()
                           << endl;

  // -- safe guard to keep Mu3eConditions::setRunNumber free of DB checks in mu3e code
  if (!fDB) return;
  
  //dbx  auto tbegin = chrono::high_resolution_clock::now();
  //dbx auto tend = chrono::high_resolution_clock::now();

	if (runnumber != fRunNumber) {
		fRunNumber = runnumber;
    // -- call update for all registered calibrations
    //    each calibration will check with its tag/IOV whether an update is required
    for (auto it: fCalibrations) {
      if (0) cout << "Mu3eConditions::setRunNumber> call update runnumber = " << runnumber
                  << " tag = " << it.first
                  << endl;
      //dbx tbegin = chrono::high_resolution_clock::now();
      it.second->update(getHash(runnumber, it.first));
      //dbx tend = chrono::high_resolution_clock::now();
      //dbx if (fPrintTiming) cout << chrono::duration_cast<chrono::microseconds>(tend-tbegin).count()
      //dbx                        << "us ::timing::" << it.first << " update"
      //dbx                        << endl;
    }
	}
}


// ----------------------------------------------------------------------
calAbs* Mu3eConditions::getCalibration(std::string name) {
  for (auto it: fCalibrations) {
    if (fVerbose > 2) cout << "  Mu3eConditions::getCalibration> looking at " << it.first << endl;
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
      break;
    } else {
			iov = it;
		}
  }
	return iov; 
}


// ----------------------------------------------------------------------
string Mu3eConditions::getHash(int runnumber, string tag) {
  int iov = whichIOV(runnumber, tag);
  // cout << "getHash: runnumber = " << runnumber << " tag = " << tag << " iov = " << iov << endl;
  // -- hash is a misnomer here
  std::stringstream ssHash;
  ssHash << "tag_" << tag << "_iov_" << iov;
  if (fVerbose > 4) cout << "calAbs::getHash(" << runnumber << ", " << tag << ") = " << ssHash.str() << endl;
  return ssHash.str();
}


// ----------------------------------------------------------------------
runRecord Mu3eConditions::getRunRecord(int irun) {
  return fDB->getRunRecord(irun);
}
