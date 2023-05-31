#include <iostream>
#include <cstdlib>
#include <vector>
#include <string.h>

#include "cdbAscii.hh"
#include "cdbMongo.hh"

#include "calPixel.hh"


using namespace std;

void printStuff(cdb*);
void aFewRuns(cdb*, string globalTag);

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
  string gt("dt23intrun");
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i], "-gt"))  {gt = string(argv[++i]);}
    if (!strcmp(argv[i], "-m"))   {mode = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-r"))   {run = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-v"))   {verbose = atoi(argv[++i]);}
  }
  
  cdb *db1 = new cdbAscii("ascii", "ascii");
  cout << "instantiated cdbAscii with name " << db1->getName() << endl;
    
  string ms("mongodb://127.0.0.1:27017/?directConnection=true&serverSelectionTimeoutMS=2000&appName=mongosh+1.7.1");
  cdb *md1 = new cdbMongo("mongo", ms);
  cout << "instantiated cdbMongo with name " << md1->getName() << endl;
    
  if (verbose > 9) {
    printStuff(db1);
    cout << "----------------------------------------------------------------------" << endl;
    printStuff(md1);
    cout << "----------------------------------------------------------------------" << endl;
  }
  
  if (0 == mode) {
    aFewRuns(db1, gt);  
    cout << "-------------" << endl;
    aFewRuns(md1, gt);  
    return 0;
  } else if (1 == mode) {
    calAbs *cal0 = new calPixel(db1, gt);
    cout << "run = " << run << " payload = " << cal0->getPayload(run) << endl;
    calAbs *cal1 = new calPixel(md1, gt);
    cout << "run = " << run << " payload = " << cal1->getPayload(run) << endl;
  }
}


// ----------------------------------------------------------------------
void printStuff(cdb *db) {
  vector<string> gt = db->getGlobalTags();
  for (auto igt : gt) {
    cout << "GT " << igt << endl;
    vector<string> tags = db->getTags(igt);
    for (auto itt : tags) {
      cout << " tag: " << itt << endl;
      vector<int> iovs = db->getIovs(itt);
      for (auto ittt :  iovs) {
        cout << "   iov " << ittt << endl;
      }
    }
  }
  
  string pl = db->getPayload(12, "pixelir");
  cout << "pixel payload: " << pl << endl;
}


// ----------------------------------------------------------------------
void aFewRuns(cdb *db, string gt) {
  cout << "DB " << db->getName() << endl;
  vector<int> vruns{23,24,25,56,90,156,157,201,202};
  calAbs *cal = new calPixel(db, gt);
  for (auto it: vruns) {
    cout << "run = " << it << " payload = " << cal->getPayload(it) << endl;
  }   
}
    
  
