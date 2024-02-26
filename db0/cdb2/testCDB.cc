#include <iostream>
#include <cstdlib>
#include <vector>
#include <string.h>

#include "Mu3eConditions.hh"
#include "cdbUtil.hh"
#include "cdbMongo.hh"
#include "cdbJSON.hh"
#include "cdbRest.hh"
#include "runRecord.hh"

#include "calPixelAlignment.hh"
#include "calMppcAlignment.hh"


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
//
// The following dumps a runRecord
// bin/testCDB -v 1 -gt mcidealv5.0 -m 3
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {
  
  // -- command line arguments
  int mode(0), run(3), verbose(0);
  string db("json"), gt("mcidealv5.0");
  string scals, sconfigs;
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i], "-cal")) {scals = string(argv[++i]);}
    if (!strcmp(argv[i], "-cfg")) {sconfigs = string(argv[++i]);}
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
  pDC->setRunNumber(run);
  
  if (0 == mode) {
    cout << "----------------------------------------------------------------------" << endl;
    
    calAbs *cal0 = pDC->createClass("pixelalignment_");
    if (verbose > 0) cal0->setVerbosity(verbose);
    
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
    string sconfTrirec = pDC->getConfString("trirec");
    cout << "trirec configuration" << endl;
    cout << "----------------------------------------------------------------------" << endl;
    cout << sconfTrirec << endl;
    cout << "----------------------------------------------------------------------" << endl;
  } else if (3 == mode) {
    cout << "Test run record" << endl;
    Mu3eConditions *pDC = Mu3eConditions::instance();
    runRecord rr = pDC->getRunRecord(12);
    cout << "runRecord:" << endl;
    cout << "----------------------------------------------------------------------" << endl;
    cout << rr.json() << endl;
    cout << "----------------------------------------------------------------------" << endl;
  } else if (4 == mode) {
    cout << "Test payloads from different origins" << endl;
    Mu3eConditions *pDC = Mu3eConditions::instance();
    calAbs *cal = pDC->getCalibration("pixelalignment_");
    calPixelAlignment* cpa = dynamic_cast<calPixelAlignment*>(cal);
    cout << "==> default calPixelAlignment: " << endl;
    cpa->printBLOB(cal->makeBLOB(), 4);

    // -- create special pixelalignment for test purposes
    cout << "==> now setup new calPixelAlignment: " << endl;
    calPixelAlignment *cpa2 = new calPixelAlignment();
    cpa2->setVerbosity(10);
    string filename = "../ascii/sensors-mcidealnew.csv";
    string result = cpa2->readCsv(filename);
    if (string::npos == result.find("Error")) {
      string spl = cpa2->makeBLOB();
      string hash = string("tag_pixelalignment_") + string("new") + string("_iov_1");
      payload pl; 
      pl.fHash = hash; 
      pl.fComment = " pixel new";
      pl.fBLOB = spl;
      if (verbose) cpa2->printBLOB(spl); 
      cpa2->writePayloadToFile(hash, ".", pl); 
    } else {
      cout << "cdbInitDB> Error, file " << filename << " not found" << endl;
      return 0;
    }
    // -- create special mppcalignment for test purposes
    cout << "==> now setup new calMppcAlignment: " << endl;
    calMppcAlignment *cma2 = new calMppcAlignment();
    cma2->setVerbosity(10);
    string filename2 = "../ascii/mppcs-mcidealnew.csv";
    string result2 = cma2->readCsv(filename2);
    if (string::npos == result2.find("Error")) {
      string spl = cma2->makeBLOB();
      string hash = string("tag_mppcalignment_") + string("new") + string("_iov_1");
      payload pl; 
      pl.fHash = hash; 
      pl.fComment = " MPPC new";
      pl.fBLOB = spl;
      if (verbose) cma2->printBLOB(spl); 
      cma2->writePayloadToFile(hash, ".", pl); 
    } else {
      cout << "cdbInitDB> Error, file " << filename2 << " not found" << endl;
      return 0;
    }
    // -- read from file and register it as a new one  with pDC
    calPixelAlignment *cpa3 = new calPixelAlignment();
    cpa3->setVerbosity(10);
    cpa3->readPayloadFromFile("tag_pixelalignment_new_iov_1", ".");
    cpa3->calculate("tag_pixelalignment_new_iov_1");
    pDC->registerCalibration("pixelalignment_new", cpa3);
    // -- read this special one from Mu3Conditions
    calAbs *calNew = pDC->getCalibration("pixelalignment_new");
    calPixelAlignment* cpaNew = dynamic_cast<calPixelAlignment*>(calNew);
    cout << " new calPixelAlignment: " << endl;
    cpaNew->printBLOB(calNew->makeBLOB(), 4);

    // -- register it as the OLD one with pDC
    pDC->registerCalibration("pixelalignment_mcidealv5.0", cpa3);
    calAbs *calNewAsOld = pDC->getCalibration("pixelalignment_");
    calPixelAlignment* cpaNewAsOld = dynamic_cast<calPixelAlignment*>(calNewAsOld);
    cout << " newAsOld calPixelAlignment: " << endl;
    cpaNewAsOld->printBLOB(cpaNewAsOld->makeBLOB(), 4);
  } else if (5 == mode) {
    cout << "Test calibration payloads from different origins with command line options passed into vector" << endl;
    Mu3eConditions *pDC = Mu3eConditions::instance();

    // -- command line parsing
    vector<string> cals;
    split(scals, ':', cals);
    for (auto it: cals) {
      string dir  = it.substr(0, it.rfind("/"));
      string hash = it.substr(it.rfind("/")+1);
      cout << "dir ->" << dir << "<-,  hash ->"<< hash << "<-" << endl;
      
      if (string::npos != hash.find("pixelalignment")) {
        calPixelAlignment *cpa = new calPixelAlignment();
        cpa->setVerbosity(10);
        cpa->readPayloadFromFile(hash, dir);
        cpa->calculate(hash);
        vector<string> tags = pDC->getTags("pixelalignment");
        pDC->registerCalibration(tags[0], cpa);
        cpa->printBLOB(cpa->makeBLOB(), 4);
      } else if (string::npos != hash.find("mppcalignment")) {
        calMppcAlignment *cpa = new calMppcAlignment();
        cpa->setVerbosity(10);
        cpa->readPayloadFromFile(hash, dir);
        cpa->calculate(hash);
        vector<string> tags = pDC->getTags("mppcalignment");
        pDC->registerCalibration(tags[0], cpa);
        cpa->printBLOB(cpa->makeBLOB(), 4);
      }
    }    
    // -- printout
    calPixelAlignment* cpa = dynamic_cast<calPixelAlignment*>(pDC->getCalibration("pixelalignment_"));
    cpa->printBLOB(cpa->makeBLOB(), 4);
    calMppcAlignment* cma = dynamic_cast<calMppcAlignment*>(pDC->getCalibration("mppcalignment_"));
    cma->printBLOB(cma->makeBLOB(), 4);
    
  } else if (6 == mode) {
    cout << "Test config payloads from different origins with command line options passed into vector" << endl;
    Mu3eConditions *pDC = Mu3eConditions::instance();
    
    vector<string> configs;
    split(sconfigs, ':', configs);
    for (auto it: configs) {
      string dir  = it.substr(0, it.rfind("/"));
      string hash = it.substr(it.rfind("/")+1);
      cout << "dir ->" << dir << "<-,  hash ->"<< hash << "<-" << endl;
      
      cfgPayload cfg;
      cfg.readFromFile(hash, dir);
      pDC->registerConf(gt, cfg);
    }    
    cout << "conf trirec: " << endl;
    cout << pDC->getConfString("trirec") << endl;
    cout << pDC->getConfString("vertex") << endl;
    
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
        cout << "   iov " << ittt << " get payload " << ("tag_" + itt.first + "_iov_" + to_string(ittt)) << endl;
        payload pl = db->getPayload("tag_" + itt.first + "_iov_" + to_string(ittt));
        pl.print();
      }
    }
  }

  cout << "Test run record" << endl;
  Mu3eConditions *pDC = Mu3eConditions::instance();
  runRecord rr = pDC->getRunRecord(12);
  cout << "runRecord:" << endl;
  cout << "----------------------------------------------------------------------" << endl;
  cout << rr.json() << endl;
  cout << "----------------------------------------------------------------------" << endl;
  
  return;
}
