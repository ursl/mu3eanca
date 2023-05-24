#include <iostream>
#include <cstdlib>
#include <vector>
#include <string.h>

#include "cdbAscii.hh"
#include "cdbMongo.hh"


using namespace std;

// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {
    
  // -- command line arguments
  int nruns(-1);
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i],"-n"))  {nruns   = atoi(argv[++i]);}
  }

  cdb *db = new cdb();
  cout << "instantiated cdb with name " << db->getName() << endl;

  cdb *db0 = new cdb("abstract", "nowhere");
  cout << "instantiated cdb with name " << db0->getName() << endl;

  cdb *db1 = new cdbAscii("ascii", "ascii");
  cout << "instantiated cdbAscii with name " << db1->getName() << endl;
  vector<string> gt = db1->getGlobalTags();
  cout << "gt.size() = " << gt.size() << endl;
  for (auto it : gt) {
    cout << " " << it << endl;
  }

  string ms("mongodb://127.0.0.1:27017/?directConnection=true&serverSelectionTimeoutMS=2000&appName=mongosh+1.7.1");
  cdb *md1 = new cdbMongo("mongo", ms);
  cout << "instantiated cdbMongo with name " << md1->getName() << endl;
  gt = md1->getGlobalTags();

  return 0;
}
