#include <iostream>
#include <cstdlib>
#include <vector>
#include <string.h>

#include "Mu3eConditions.hh"
#include "cdbMongo.hh"
#include "cdbJSON.hh"
#include "cdbRest.hh"
#include "runRecord.hh"

#include "calPixelAlignment.hh"


using namespace std;

void aFewRuns(cdbAbs *, string globalTag, calAbs *);
void printStuff(cdbAbs *, string gt);
void printAll(cdbAbs *);

// ----------------------------------------------------------------------
// testCDB
// -------
//
// Examples:
// ---------
// bin/testCDB -v 1 -gt mcidealv5.1 -db mongo
//
// The following two require
//   pc11740>cd ~/mu3eanca/db1/rest && sudo npm run start
// 
// bin/testCDB -v 1 -gt mcidealv5.0 -db rest
// bin/testCDB -v 1 -gt mcidealv5.1 -db http://pc11740.psi.ch/cdb
//
// bin/testCDB -v 1 -gt mcidealv5.0 -db ~/data/mu3e/json10
// 
// 
// The following dumps the entire CDB contents
// bin/testCDB -v 1 -gt mcidealv5.0 -m 1
//
// The following dumps a config file
// bin/testCDB -v 1 -gt mcidealv5.0 -m 2
// ----------------------------------------------------------------------




// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {
  
  // -- command line arguments
  int mode(0), run(3), verbose(0);
  string db("json"), gt("mcidealv5.0");
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i], "-db"))  {db = string(argv[++i]);}
    if (!strcmp(argv[i], "-gt"))  {gt = string(argv[++i]);}
    if (!strcmp(argv[i], "-m"))   {mode = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-r"))   {run = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-v"))   {verbose = atoi(argv[++i]);}
  }

  cdbAbs *pDB(0);
  if (string::npos != db.find("json")) {
    pDB = new cdbJSON(gt, db, verbose);
  } else if (string::npos != db.find("mongo")) {
    string ms("mongodb://pc11740.psi.ch:27017/?directConnection=true&serverSelectionTimeoutMS=2000&appName=mongosh+1.7.1");
    pDB = new cdbMongo(gt, ms, verbose);
  } else if (string::npos != db.find("rest") || string::npos != db.find("http://")) {
    //    string ms("https://eu-central-1.aws.data.mongodb-api.com/app/data-pauzo/endpoint/data/v1/action/");
    string ms("http://pc11740.psi.ch/cdb");
    pDB = new cdbRest(gt, ms, verbose);
  } else {
    cout << "ERROR: " << db << " not known." << endl;
    return 0;
  }


  Mu3eConditions *pDC = Mu3eConditions::instance(gt, pDB);
  pDC->setVerbosity(verbose);
  
  if (0 == mode) {
    cout << "----------------------------------------------------------------------" << endl;
    
    calAbs *cal0 = pDC->createClass("pixelalignment_");
    if (verbose > 0) cal0->setVerbosity(verbose);
    
    pDC->setRunNumber(run);
    cout << "set run number to " << pDC->getRunNumber() << endl;
    printStuff(pDB, gt);
    cout << "----------------------------------------------------------------------" << endl;
  } else if (1 == mode) {
    printAll(pDB);
  } else if (2 == mode) {
    cout << "Test configuration" << endl;
    Mu3eConditions *pDC = Mu3eConditions::instance();
    string sconfDet = pDC->getConfString("detector");
    cout << "detector configuration" << endl;
    cout << "----------------------------------------------------------------------" << endl;
    cout << sconfDet << endl;
    cout << "----------------------------------------------------------------------" << endl;
  }
  return 0;
}


// ----------------------------------------------------------------------
void printStuff(cdbAbs *db, string gt) {
  Mu3eConditions *pDC = Mu3eConditions::instance();
  vector<string> vgt = pDC->getGlobalTags();
  for (auto igt : vgt) {
    cout << "GT " << igt << endl;
    vector<string> tags = pDC->getTags();
    for (auto itt : tags) {
      cout << " tag: " << itt << endl;
      vector<int> iovs = pDC->getIOVs(itt);
      for (auto ittt :  iovs) {
        cout << "   iov " << ittt << endl;
      }
    }
  }
  
  payload pl = db->getPayload("tag_pixelalignment_" + gt + "_iov_1");
  cout << "printStuff> pixel payload: " << pl.printString(false) << endl;

  runRecord rr = db->getRunRecord(12);
  rr.print();
}


// ----------------------------------------------------------------------
void aFewRuns(cdbAbs *db, string gt, calAbs *cal) {
  Mu3eConditions *pDC = Mu3eConditions::instance();
  cout << "DB " << pDC->getGlobalTag() << endl;
  vector<int> vruns{23,24,157,201,202};
  pDC->printCalibrations();

  calAbs *cl = pDC->getCalibration("pixelalignment_");
  std::cout << "cl = " << cl << std::endl;

  //  calPixelAlignment *al = dynamic_cast<calPixelAlignment*>(cal);
  for (auto it: vruns) {
    pDC->setRunNumber(it);
    cout << "now for run = " << it << " payload hash ->" << cal->getHash() << "<-" << endl;
    // -- not any more. calAbs* classes are filled from BLOBs, not directly (except though makeBLOB)
    // double vx;
    // al->setVxAddr(&vx);
    // al->fillVars(1);
    // cout << "vx[1]  = " << vx << endl;
    // al->fillVars(33);
    // cout << "vx[33] = " << vx << endl;
  }   
}


// ----------------------------------------------------------------------
void printAll(cdbAbs *db) {
  vector<string> vgt = db->readGlobalTags();
  for (auto igt : vgt) {
    cout << "GT " << igt << endl;
    vector<string> tags = db->readTags(igt);
    map<string, vector<int>> iovs = db->readIOVs(tags);
    for (auto itt : iovs) {
      cout << " tag: " << itt.first << endl;
      for (auto ittt :  itt.second) {
        cout << "   iov " << ittt << endl;
        payload pl = db->getPayload("tag_" + itt.first + "_iov_" + to_string(ittt));
        pl.print();
      }
    }
  }

  return;
}
