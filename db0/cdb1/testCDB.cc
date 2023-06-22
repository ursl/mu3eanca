#include <iostream>
#include <cstdlib>
#include <vector>
#include <string.h>

#include "cdbMongo.hh"
#include "cdbJSON.hh"
#include "cdbRest.hh"
#include "cdbClassFactory.hh"

#include "calPixelAlignment.hh"


using namespace std;

void printStuff(cdbAbs *);
void aFewRuns(cdbAbs *, string globalTag, calAbs *);

// ----------------------------------------------------------------------
// testCDB
// -------
//
// Examples:
// bin/testCDB -gt dt23prompt -m 1 -r 1000
// bin/testCDB -gt dt23intrun
// bin/testCDB -gt dt23intrun -v 10
// 
// ----------------------------------------------------------------------




// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {
  
  // -- command line arguments
  int mode(0), run(0), verbose(0);
  string db("json"), gt("dt23intrun");
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i], "-db"))  {db = string(argv[++i]);}
    if (!strcmp(argv[i], "-gt"))  {gt = string(argv[++i]);}
    if (!strcmp(argv[i], "-m"))   {mode = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-r"))   {run = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-v"))   {verbose = atoi(argv[++i]);}
  }

  cdbAbs *pDB(0);
  if (string::npos != db.find("json")) {
    pDB = new cdbJSON(gt, "json", verbose);
  } else if (string::npos != db.find("mongo")) {
    string ms("mongodb://127.0.0.1:27017/?directConnection=true&serverSelectionTimeoutMS=2000&appName=mongosh+1.7.1");
    pDB = new cdbMongo(gt, ms, verbose);
  } else if (string::npos != db.find("rest")) {
    // string ms("https://eu-central-1.aws.data.mongodb-api.com/app/data-pauzo/endpoint/data/v1/action/findOne");
    string ms("https://eu-central-1.aws.data.mongodb-api.com/app/data-pauzo/endpoint/data/v1/action/");
    pDB = new cdbRest(gt, ms, verbose);
  } else {
    cout << "ERROR: " << db << " not known." << endl;
    return 0;
  }
  
  // -- calibration classes instantiation and registration must happen before setting the run number in the CBD
  cdbClassFactory *cdbcf = cdbClassFactory::instance(pDB);
  if (verbose > 0) cdbcf->setVerbosity(verbose);

  calAbs *cal0 = cdbcf->createClass("pixelalignment_");
  if (verbose > 0) cal0->setVerbosity(verbose);
    
  pDB->setRunNumber(3);
  cout << "set run number to " << pDB->getRunNumber() << endl;

  
  if (0 == mode) {
    cout << "----------------------------------------------------------------------" << endl;
    printStuff(pDB);
    cout << "----------------------------------------------------------------------" << endl;
  } else if (1 == mode) {
    cout << "run = " << pDB->getRunNumber() << " payload hash -> " << cal0->getHash() << "<-" << endl;
  } else if (2 == mode) {
    aFewRuns(pDB, gt, cal0);  
  }
  return 0;
}


// ----------------------------------------------------------------------
void printStuff(cdbAbs *db) {
  vector<string> gt = db->getGlobalTags();
  for (auto igt : gt) {
    cout << "GT " << igt << endl;
    db->setGlobalTag(igt);
    vector<string> tags = db->getTags();
    for (auto itt : tags) {
      cout << " tag: " << itt << endl;
      vector<int> iovs = db->getIOVs(itt);
      for (auto ittt :  iovs) {
        cout << "   iov " << ittt << endl;
      }
    }
  }
  
  payload pl = db->getPayload(12, "pixelalignment_dt23intrun");
  cout << "printStuff> pixel payload: " << pl.printString() << endl;
}


// ----------------------------------------------------------------------
void aFewRuns(cdbAbs *db, string gt, calAbs *cal) {
  cout << "DB " << db->getGlobalTag() << endl;
  vector<int> vruns{23,24,157,201,202};
  calPixelAlignment *al = dynamic_cast<calPixelAlignment*>(cal);
  for (auto it: vruns) {
    db->setRunNumber(it);
    cout << "now for run = " << it << " payload hash ->" << cal->getHash() << "<-" << endl;
    double vx;
    al->setVxAddr(&vx);
    al->fillVars(1);
    cout << "vx[1]  = " << vx << endl;
    al->fillVars(33);
    cout << "vx[33] = " << vx << endl;
  }   
}
    
  
