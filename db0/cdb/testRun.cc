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
// testRun
// -------
//
// Examples:
// bin/testRun -rmin 123 -rmax 200 
// 
// ----------------------------------------------------------------------




// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {
  
  // -- command line arguments
  int rmin(100), rmax(101), verbose(0);
  string gt("dt23intrun"), mode("ascii");
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i], "-gt"))   {gt      = string(argv[++i]);}
    if (!strcmp(argv[i], "-m"))    {mode    = string(argv[++i]);}
    if (!strcmp(argv[i], "-rmin")) {rmin    = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-rmax")) {rmax    = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-v"))    {verbose = atoi(argv[++i]);}
  }
  
  cdb *db;
  if (string::npos != mode.find("ascii")) {
    db = new cdbAscii("ascii", "ascii");
  } else if (string::npos != mode.find("mongo")) {
    string ms("mongodb://127.0.0.1:27017/?directConnection=true&serverSelectionTimeoutMS=2000&appName=mongosh+1.7.1");
    db = new cdbMongo("mongo", ms);
  } else {
		cout << "no DB instantiated, aborting" << endl;
		return 0;
	}
  cout << "instantiated cdb with name " << db->getName() << endl;
  
	return 0;
}
