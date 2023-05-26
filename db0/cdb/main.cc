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
int main(int argc, char* argv[]) {
  
  // -- command line arguments
	int verbose(0);
	string gt("dt23intrun");
  for (int i = 0; i < argc; i++){
		if (!strcmp(argv[i], "-gt"))  {gt = string(argv[++i]);}
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
	
	aFewRuns(db1, gt);  
  cout << "-------------" << endl;
	aFewRuns(md1, gt);  
  return 0;
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
    
  
