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
int main(int argc, char* argv[]) {

  // -- command line arguments
  string dirName("fixme"), dirPath("fixme"), pattern("unset"), urlString("localhost:5050/cdb/");
  bool all(false);
  bool onlyDelete(false); // ONLY delete, do not write new records
  int maxRuns(10000), firstRun(0), lastRun(-1);
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "--all")) {all = true;}
    if (!strcmp(argv[i], "--dir")) {dirPath = string(argv[++i]);}
    if (!strcmp(argv[i], "-dir"))  {dirPath = string(argv[++i]);}
    if (!strcmp(argv[i], "-f"))    {firstRun = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-l"))    {lastRun = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-m"))    {maxRuns = atoi(argv[++i]);}

    if (!strcmp(argv[i], "--url")) {urlString = string(argv[++i]);}
    if (!strcmp(argv[i], "-u"))    {urlString = string(argv[++i]);}
  }

  cdbRest *pDB(0);

  pDB = new cdbRest("mcidealv6.1", urlString, 0);
  vector<string> vGlobalTags = pDB->readGlobalTags();
  
  for (auto it: vGlobalTags) {
    vector<string> vTags = pDB->readTags(it);
    cout << "global tag: " << it << endl;
    map<string, vector<int>> mIOVs = pDB->readIOVs(vTags);
    for (auto ittt: mIOVs) {
      cout << "    tag: " << ittt.first << " iovs: ";
      for (auto itttt: ittt.second) {
        cout << itttt << " ";
        string payloadName = "tag_" + ittt.first + "_iov_" + to_string(itttt);
        payload pl = pDB->getPayload(payloadName);
        ofstream ofs("payloads/" + payloadName);
        ofs << pl.json() << endl;
        ofs.close();
      }
      cout << endl;
    }
  }

  // for (int irun = 1; irun < maxRuns; ++irun) {
  //   runRecord rr = pDB->getRunRecord(irun);
  //   cout << rr.printString() << endl;
  // }


  vector<string> vRunNumbers = pDB->getAllRunNumbers();
  int cnt(0);
  //  for (int it = startIdx; it < startIdx + maxRuns; ++it) {
  for (int it = 0; it < vRunNumbers.size(); ++it) {
    int irun = stoi(vRunNumbers[it]);
    if (irun < firstRun) continue;
    if (irun > lastRun) continue;
    
    runRecord rr = pDB->getRunRecord(irun);
    cout << rr.printString() << endl;
    //usleep(1000);
    //sleep(1);
    //if (cnt > 100) break;
    ++cnt;
  }

  delete pDB; 
} 