#include "cdbRest.hh"
#include "runRecord.hh"

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string.h>
#include <dirent.h>  /// for directory reading
#include <sys/time.h>
#include <unistd.h>



using namespace std;
// ----------------------------------------------------------------------
// -- syncRunDB
// -- ---------
// -- 
// -- produce updates to either DataQuality or RunInfo attributes of run records in RDB
// -- updates a mongoDB server, does not run on the JSON backend server
// --
// -- Usage: bin/syncRunDB -m mode -f firstRun -l lastRun 
// --
// -- History:
// --   2025/05/08: first shot
// ------------------------------------------------------------------------

void rdbMode1(runRecord &);

int main(int argc, char* argv[]) {

  // -- command line arguments
  string urlString("localhost:5050/cdb/");
  bool debug(false);
  int firstRun(0), lastRun(-1), mode(0);
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-d"))   {debug = true;}
    if (!strcmp(argv[i], "-f"))   {firstRun = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-l"))   {lastRun = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-m"))    {mode    = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-u"))    {urlString = string(argv[++i]);}
  }

  cdbRest *pDB(0);

  pDB = new cdbRest("mcidealv6.1", urlString, 0);

  vector<string> vRunNumbers = pDB->getAllRunNumbers();
  for (int it = 0; it < vRunNumbers.size(); ++it) {
    int irun = stoi(vRunNumbers[it]);
    if (irun < firstRun) continue;
    if ((lastRun > 0) && (irun > lastRun)) continue;
    

    runRecord rr = pDB->getRunRecord(irun);
    if (1 == mode) {
      rdbMode1(rr);
    } 
    if (!debug) {
      // call curl
    }
  }
  delete pDB; 
} 


void rdbMode1(runRecord &rr) {
  cout << "hallo: " << rr.printString() << endl;
}