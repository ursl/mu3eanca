#include <iostream>
#include <cstdlib>
#include <vector>
#include <string.h>

#include "cdbAscii.hh"
#include "cdbMongo.hh"


using namespace std;

void printStuff(cdb*);


// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {
    
  // -- command line arguments
  //  int nruns(-1);
  for (int i = 0; i < argc; i++){
    //    if (!strcmp(argv[i],"-n"))  {nruns   = atoi(argv[++i]);}
  }

  cdb *db1 = new cdbAscii("ascii", "ascii");
  cout << "instantiated cdbAscii with name " << db1->getName() << endl;

  string ms("mongodb://127.0.0.1:27017/?directConnection=true&serverSelectionTimeoutMS=2000&appName=mongosh+1.7.1");
  cdb *md1 = new cdbMongo("mongo", ms);
  cout << "instantiated cdbMongo with name " << md1->getName() << endl;

  printStuff(db1);
  cout << "----------------------------------------------------------------------" << endl;
  printStuff(md1);
  
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
