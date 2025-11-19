#include "cdbRest.hh"
#include "cdbUtil.hh"

#include "runRecord.hh"

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string.h>
#include <dirent.h>  /// for directory reading
#include <sys/time.h>
#include <unistd.h>

 
// ----------------------------------------------------------------------
// synJSON copy mongoDB CDB contents via REST interface to JSON directory. 
//         Will not delete what is there but will overwrite //
// 
// Usage:     ./bin/syncJSON --dir junk [-a] [-h pc11740]
// -----
// 
// Examples:  ./bin/syncJSON --dir /Users/ursl/data/mu3e/test-cdb --host localhost -m payload --pat tag_pixelalignment_datav6.3=2025V1test_iov_1 -c
//            ./bin/syncJSON --dir /Users/ursl/data/mu3e/test-cdb --host localhost -m tag --pat pixelalignment_datav6.3=2025V1test -c
//            ./bin/syncJSON --dir /Users/ursl/data/mu3e/test-cdb --host localhost -m gt --pat datav6.3=2025V1test -c
//
// Options:   -a, --all        all runs dumped into the runrecords
// -------    -c               CDB only
//            -d, --dir path   provide a location to dump the JSON files
//            -h, --host host  provide a hostname from where to retrieve
//            -f  run          provide a first run number
//            -l  run          provide a last run number
//            -m, --max runs   provide a maximum number of runs to dump
//            -p, --pat pattern provide a pattern to match the global tags
//            -r               RDB only
// ----------------------------------------------------------------------

using namespace std;
// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {

  // -- command line arguments
  string dirName("fixme"), dirPath("fixme"), pattern("unset"), 
         host("pc11740"), mode("all");
  bool all(false);
  bool cdbOnly(false);
  bool rdbOnly(false);
  bool onlyDelete(false); // ONLY delete, do not write new records
  int firstRun(0), lastRun(-1);
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-a"))     {all = true;}
    if (!strcmp(argv[i], "--all"))  {all = true;}
    if (!strcmp(argv[i], "-c"))     {cdbOnly = true;}
    if (!strcmp(argv[i], "--cdb"))  {cdbOnly = true;}
    if (!strcmp(argv[i], "-d"))     {dirPath = string(argv[++i]);}
    if (!strcmp(argv[i], "--dir"))  {dirPath = string(argv[++i]);}
    if (!strcmp(argv[i], "-h"))     {host = string(argv[++i]);}
    if (!strcmp(argv[i], "--host")) {host = string(argv[++i]);}
    if (!strcmp(argv[i], "-f"))     {firstRun = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-l"))     {lastRun = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-m"))     {mode = string(argv[++i]);}
    if (!strcmp(argv[i], "-p"))     {pattern = string(argv[++i]);}
    if (!strcmp(argv[i], "--pat"))  {pattern = string(argv[++i]);}
    if (!strcmp(argv[i], "-r"))     {rdbOnly = true;}
    if (!strcmp(argv[i], "--rdb"))  {rdbOnly = true;}
  }

  cdbRest *pDB(0);

  string urlString = host + ":5050/cdb/";
  cout << "urlString: " << urlString << endl;
  pDB = new cdbRest("mcidealv6.1", urlString, 0);

// -- check whether directories for JSONs already exist
  vector<string> testdirs{dirPath,
                          dirPath + "/globaltags",
                          dirPath + "/tags",
                          dirPath + "/payloads",
                          dirPath + "/runrecords",
                          dirPath + "/configs",
                         };
  for (auto it: testdirs) {
    DIR *folder = opendir(it.c_str());
    if (folder == NULL) {
      cout << "creating " << it << endl;
      system(string("mkdir -p " + it).c_str());
    } else {
      closedir(folder);
    }
  }

  vector<string> vGlobalTags = pDB->readGlobalTags();
  
  if (!rdbOnly) {
    if (mode == "all") {
      for (auto it: vGlobalTags) {
        if (pattern != "unset") {
          if (string::npos == it.find(pattern)) {
            cout << "pattern ->" << pattern << "<- not matched to ->" << it << "<- ... skipping" << endl;
            continue;
          }
        }
        // -- write global tag to file
        vector<string> vTags = pDB->readTags(it);
        cout << "global tag: " << it << endl;
        stringstream sstr;
        sstr << "{ \"gt\" : \"" << it << "\", \"tags\" : ";
        sstr << jsFormat(vTags);
        sstr << " }" << endl;
        ofstream ofs(dirPath + "/globaltags/" + it);
        ofs << sstr.str();
        ofs.close();
      
        map<string, vector<int>> mIOVs = pDB->readIOVs(vTags);
        for (auto ittt: mIOVs) {
          // -- write tag to file
          stringstream sstr;
          sstr << "  { \"tag\" : \"" << ittt.first << "\", \"iovs\" : ";
          sstr << jsFormat(ittt.second);
          sstr << " }" << endl;
          
          // -- JSON
          ofstream JS;
          JS.open(dirPath + "/tags/" + ittt.first);
          if (JS.fail()) {
            cout << "Error failed to open " << dirPath << "/tags/" << ittt.first << endl;
          }
          JS << sstr.str();
          JS.close();

          cout << "    tag: " << ittt.first << " iovs: ";
          for (auto itttt: ittt.second) {
            cout << itttt << " ";
            string payloadName = "tag_" + ittt.first + "_iov_" + to_string(itttt);
            payload pl = pDB->getPayload(payloadName);
            ofstream ofs(dirPath + "/payloads/" + payloadName);
            ofs << pl.json() << endl;
            ofs.close();
          }
          cout << endl;
        }
      }
    } else if (mode == "gt") {
      vector<string> vTags = pDB->readGlobalTags();
      for (auto it: vTags) {
        if (pattern != "unset") {
          if (string::npos == it.find(pattern)) {
            cout << "pattern ->" << pattern << "<- not matched to ->" << it << "<- ... skipping" << endl;
            continue;
          }
        }
        vector<string> vTags = pDB->readTags(it);
        cout << "global tag: " << it << endl;
        stringstream sstr;
        sstr << "{ \"gt\" : \"" << it << "\", \"tags\" : ";
        sstr << jsFormat(vTags);
        sstr << " }" << endl;
        ofstream ofs(dirPath + "/globaltags/" + it);
        ofs << sstr.str();
        ofs.close();
      }
    } else if (mode == "tag") {
      vector<string> vGlobalTags = pDB->readGlobalTags();
      for (auto it: vGlobalTags) {
        vector<string> vTags = pDB->readTags(it);
        for (auto ittt: vTags) {
          if ((pattern != "unset") && (string::npos != ittt.find(pattern))) {
            cout << "    tag: " << ittt << endl;
            map<string, vector<int>> mIOVs = pDB->readIOVs(vTags);
            for (auto ittt: mIOVs) {
              if ((pattern != "unset") && (string::npos != ittt.first.find(pattern))) {
                // -- write tag to file
                stringstream sstr;
                sstr << "  { \"tag\" : \"" << ittt.first << "\", \"iovs\" : ";
                sstr << jsFormat(ittt.second);
                sstr << " }" << endl;
                ofstream ofs(dirPath + "/tags/" + ittt.first);
                ofs << sstr.str();
                ofs.close();
              }
            }
          }
        }
      }
    } else if (mode == "payload") {
      vector<string> vGlobalTags = pDB->readGlobalTags();
      // cout << "  dbx vGlobalTags: " << vGlobalTags.size() << endl;
      for (auto it: vGlobalTags) {
        vector<string> vTags = pDB->readTags(it);
        //cout << "  dbx vTags: " << vTags.size() << endl;
        for (auto ittt: vTags) {
          //cout << "  dbx ittt: " << ittt << endl;
            map<string, vector<int>> mIOVs = pDB->readIOVs(vTags);
            string ppattern = pattern;
            if (string::npos != ppattern.find("tag_")) {
              replaceAll(ppattern, "tag_", "");
            }
            if (string::npos != ppattern.find("_iov_")) {
              ppattern = ppattern.substr(0, ppattern.rfind('_iov_')-5);
            }
            //cout << "  dbx ppattern: " << ppattern << endl;
            if ((pattern != "unset") && (string::npos == ittt.find(ppattern))) {
              continue;
            }
            for (auto ittti: mIOVs) {
              for (auto itttt: ittti.second) {
                string spl = "tag_" + ittti.first + "_iov_" + to_string(itttt); 
                if ((pattern != "unset") && (string::npos != spl.find(pattern))) {
                  cout << "    tag: " << ittti.first << " iov: " << itttt 
                  << " pattern: " << pattern << " spl: " << spl
                  << endl;
                  payload pl = pDB->getPayload(spl);
                  ofstream ofs(dirPath + "/payloads/" + spl);
                  ofs << pl.json() << endl;
                  ofs.close();
                }
              }
            }
        }
      }
    }
  }
  return 0;

  if (!cdbOnly) {
    // -- dump all significant runs
    vector<string> vRunNumbers = pDB->getAllRunNumbers();
    cout << "total number of runs: " << vRunNumbers.size() << endl;
    int cnt(0);
    cout << "all = " << all << endl;
    for (int it = 0; it < vRunNumbers.size(); ++it) {
      int irun = stoi(vRunNumbers[it]);
      if (irun < firstRun) continue;
      if (lastRun > -1 && irun > lastRun) continue;
      runRecord rr = pDB->getRunRecord(irun);
      if (all || rr.isSignificant()) {
        cout << rr.printSummary() << endl;
        string filename = dirPath + "/runrecords/" + "runRecord_" + to_string(irun) + ".json";
        ofstream ofs(filename);
        ofs << rr.json() << endl;
        ofs.close();
        ++cnt;  
      }
    }
  }
  delete pDB; 
} 