#include "cdbRest.hh"
#include "cdbUtil.hh"

#include "runRecord.hh"

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string.h>
#include <dirent.h>
#include <sys/time.h>
#include <unistd.h>

// ----------------------------------------------------------------------
// syncJSON — copy CDB from REST into a JSON directory tree:
//   <dir>/globaltags/<gt>
//   <dir>/tags/<tag>
//   <dir>/payloads/<tag>/<block>/tag_<tag>_iov_<n>
//
// Usage (new):
//   --dir <cdb-root> --host <h> --sync all [--name <gt>] [--pat <sub>]   # all GTs (optional --pat filter)
//   --dir <cdb-root> --host <h> --sync gt --name <gt> [--deep]
//   --dir <cdb-root> --host <h> --sync tag --name <tag> [--deep]
//   --dir <cdb-root> --host <h> --sync payload --name <tagDir|tag_*_iov_*>
//
// Legacy (still accepted): -m all|gt|tag|payload with -p / -e for names
//
// Options:  -c / --cdb  CDB only (skip runrecords)
//           -a / --all  all runrecords (not only significant)
//           -f -l -r    run number filters (unchanged)
//           --rdb       RDB only
// ----------------------------------------------------------------------

using namespace std;

// ----------------------------------------------------------------------
static void printSyncJSONUsage(const char* prog) {
  cout <<
    "syncJSON — download CDB from REST (<host>:5050/cdb/) into a JSON directory tree.\n"
    "\n"
    "Output layout under --dir:\n"
    "  globaltags/<gt>     tags/<tag>     payloads/<tag>/<runblock>/tag_<tag>_iov_<n>\n"
    "\n"
    "Options:\n"
    "  --dir <path>     CDB root (required). Subdirs are created as needed.\n"
    "  --host <h>       REST host (default mu3edb0). URL: <host>:5050/cdb/\n"
    "                   Note: -h <h> is the same as --host (not help).\n"
    "  --sync <what>    all | gt | tag | payload\n"
    "  --name <name>    GT name, tag name, tag directory, or full payload hash (see below).\n"
    "  --deep           With gt: GT + all member tags + all payloads.\n"
    "                   With tag: tag file + all IOVs for that tag.\n"
    "                   Without --deep: only the GT or tag JSON file.\n"
    "  --pat <sub>      With --sync all: optional substring filter on global tag names.\n"
    "  -c, --cdb        CDB only (skip runrecords).\n"
    "  -a, --all        When writing runrecords: include all runs, not only significant.\n"
    "  -f <n> -l <n>    First / last run number (runrecord dump).\n"
    "  -r <file>        Comma-separated run list file (runrecord dump).\n"
    "  --rdb            Runrecords only (skip CDB sync).\n"
    "  --help, -?       This message.\n"
    "\n"
    "--sync modes:\n"
    "  all       Every global tag, deep (GT + tags + payloads).\n"
    "            Optional --name <gt> restricts to one GT; optional --pat filters GT names.\n"
    "  gt        --name required. Shallow: globaltags/<gt> only. --deep: +tags +payloads.\n"
    "  tag       --name required. Shallow: tags/<tag> only. --deep: + all payload IOVs.\n"
    "  payload   --name is either a tag directory (all IOVs) or tag_<tag>_iov_<n> (single).\n"
    "\n"
    "Legacy: -m all|gt|tag|payload with -e or -p supplying the name (same as --name).\n"
    "Default -m is all → full deep dump of every global tag.\n"
    "\n"
    "Examples:\n"
    "  " << prog << " --cdb --dir ~/cdb --host localhost --sync all \n"
    "  " << prog << " --cdb --dir ~/cdb --host localhost --sync payload --name pixeltimecalibration_mcidealv6.5 \n"
    "  " << prog << " --cdb --dir ~/cdb --host localhost --sync gt --name datav6.3=2025 --deep \n"
    "\n";
}


// ----------------------------------------------------------------------
static void mkdirParentsForFile(const string& filepath) {
  string::size_type lastSlash = filepath.rfind('/');
  if (lastSlash != string::npos) {
    system(string("mkdir -p " + filepath.substr(0, lastSlash)).c_str());
  }
}

// ----------------------------------------------------------------------
static void writePayloadOne(cdbAbs* pDB, const string& dirPath, const string& hash) {
  payload pl = pDB->getPayload(hash);
  string subpath = payloadSubPathFromHash(hash);
  string filepath = pathJoin(pathJoin(dirPath, "payloads"), subpath);
  mkdirParentsForFile(filepath);
  ofstream ofs(filepath);
  ofs << pl.json() << endl;
}

// ----------------------------------------------------------------------
static void writePayloadsForTag(cdbAbs* pDB, const string& dirPath, const string& tag) {
  map<string, vector<int>> mIOVs = pDB->readIOVs(vector<string>{tag});
  auto it = mIOVs.find(tag);
  if (it == mIOVs.end()) {
    cerr << "syncJSON: no IOVs for tag \"" << tag << "\" (skip payloads)" << endl;
    return;
  }
  for (int iov : it->second) {
    string h = "tag_" + tag + "_iov_" + to_string(iov);
    writePayloadOne(pDB, dirPath, h);
  }
}

// ----------------------------------------------------------------------
static void writeTagFile(cdbAbs* pDB, const string& dirPath, const string& tag,
                         const vector<int>& iovs) {
  stringstream sstr;
  sstr << "{ \"tag\" : \"" << tag << "\", \"iovs\" : ";
  sstr << jsFormat(iovs);
  string tagComment = pDB->getTagComment(tag);
  if (!tagComment.empty()) {
    sstr << ", \"comment\" : \"" << escapeJsonString(tagComment) << "\"";
  }
  sstr << " }" << endl;
  string tf = pathJoin(pathJoin(dirPath, "tags"), tag);
  mkdirParentsForFile(tf);
  ofstream ofs(tf);
  ofs << sstr.str();
}

// ----------------------------------------------------------------------
static void writeGlobalTagFile(cdbAbs* pDB, const string& dirPath, const string& gt,
                               const vector<string>& vTags) {
  stringstream sstr;
  sstr << "{ \"gt\" : \"" << gt << "\", \"tags\" : ";
  sstr << jsFormat(vTags);
  string gtComment = pDB->getGlobalTagComment(gt);
  if (!gtComment.empty()) {
    sstr << ", \"comment\" : \"" << escapeJsonString(gtComment) << "\"";
  }
  sstr << " }" << endl;
  string gf = pathJoin(pathJoin(dirPath, "globaltags"), gt);
  mkdirParentsForFile(gf);
  ofstream ofs(gf);
  ofs << sstr.str();
}

// ----------------------------------------------------------------------
static void syncGtShallow(cdbAbs* pDB, const string& dirPath, const string& gt) {
  vector<string> vTags = pDB->readTags(gt);
  writeGlobalTagFile(pDB, dirPath, gt, vTags);
}

// ----------------------------------------------------------------------
static void syncGtDeep(cdbAbs* pDB, const string& dirPath, const string& gt) {
  syncGtShallow(pDB, dirPath, gt);
  vector<string> vTags = pDB->readTags(gt);
  for (const string& t : vTags) {
    map<string, vector<int>> mIOVs = pDB->readIOVs(vector<string>{t});
    auto it = mIOVs.find(t);
    if (it == mIOVs.end()) {
      cerr << "syncJSON: skip tag \"" << t << "\" (no tag document / IOVs)" << endl;
      continue;
    }
    writeTagFile(pDB, dirPath, t, it->second);
    for (int iov : it->second) {
      string h = "tag_" + t + "_iov_" + to_string(iov);
      writePayloadOne(pDB, dirPath, h);
    }
  }
}

// ----------------------------------------------------------------------
static void syncTagShallow(cdbAbs* pDB, const string& dirPath, const string& tag) {
  map<string, vector<int>> mIOVs = pDB->readIOVs(vector<string>{tag});
  auto it = mIOVs.find(tag);
  if (it == mIOVs.end()) {
    cerr << "syncJSON: cannot read tag \"" << tag << "\"" << endl;
    return;
  }
  writeTagFile(pDB, dirPath, tag, it->second);
}

// ----------------------------------------------------------------------
static void syncTagDeep(cdbAbs* pDB, const string& dirPath, const string& tag) {
  syncTagShallow(pDB, dirPath, tag);
  writePayloadsForTag(pDB, dirPath, tag);
}

// ----------------------------------------------------------------------
static void syncPayloadSelection(cdbAbs* pDB, const string& dirPath, const string& name) {
  if (isPayloadHashBasename(name)) {
    writePayloadOne(pDB, dirPath, name);
    return;
  }
  writePayloadsForTag(pDB, dirPath, name);
}

// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {
  for (int i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-?")) {
      printSyncJSONUsage(argc > 0 ? argv[0] : "syncJSON");
      return 0;
    }
  }

  string dirPath("fixme"), pattern("unset"), exactPattern("unset"), host("mu3edb0"),
      mode("all"), runfile("unset");
  string syncKind;
  string syncName;
  bool deep(false);
  bool all(false);
  bool cdbOnly(false);
  bool rdbOnly(false);
  int firstRun(0), lastRun(-1);

  for (int i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "-a")) {
      all = true;
    } else if (!strcmp(argv[i], "--all")) {
      all = true;
    } else if (!strcmp(argv[i], "-c")) {
      cdbOnly = true;
    } else if (!strcmp(argv[i], "--cdb")) {
      cdbOnly = true;
    } else if (!strcmp(argv[i], "-d") && i + 1 < argc) {
      dirPath = string(argv[++i]);
    } else if (!strcmp(argv[i], "--dir") && i + 1 < argc) {
      dirPath = string(argv[++i]);
    } else if (!strcmp(argv[i], "-e") && i + 1 < argc) {
      exactPattern = string(argv[++i]);
    } else if (!strcmp(argv[i], "-h") && i + 1 < argc) {
      host = string(argv[++i]);
    } else if (!strcmp(argv[i], "--host") && i + 1 < argc) {
      host = string(argv[++i]);
    } else if (!strcmp(argv[i], "-f") && i + 1 < argc) {
      firstRun = atoi(argv[++i]);
    } else if (!strcmp(argv[i], "-l") && i + 1 < argc) {
      lastRun = atoi(argv[++i]);
    } else if (!strcmp(argv[i], "-m") && i + 1 < argc) {
      mode = string(argv[++i]);
    } else if (!strcmp(argv[i], "-p") && i + 1 < argc) {
      pattern = string(argv[++i]);
    } else if (!strcmp(argv[i], "--pat") && i + 1 < argc) {
      pattern = string(argv[++i]);
    } else if (!strcmp(argv[i], "-r") && i + 1 < argc) {
      runfile = string(argv[++i]);
    } else if (!strcmp(argv[i], "--rdb")) {
      rdbOnly = true;
    } else if (!strcmp(argv[i], "--sync") && i + 1 < argc) {
      syncKind = string(argv[++i]);
    } else if (!strcmp(argv[i], "--name") && i + 1 < argc) {
      syncName = string(argv[++i]);
    } else if (!strcmp(argv[i], "--deep")) {
      deep = true;
    }
  }

  // Map legacy -m / -p / -e when --sync not given
  if (syncKind.empty()) {
    if (mode == "all") {
      syncKind = "all";
    } else if (mode == "gt") {
      syncKind = "gt";
      syncName = (exactPattern != "unset") ? exactPattern : pattern;
    } else if (mode == "tag") {
      syncKind = "tag";
      syncName = (exactPattern != "unset") ? exactPattern : pattern;
    } else if (mode == "payload") {
      syncKind = "payload";
      syncName = (exactPattern != "unset") ? exactPattern : pattern;
    }
  }

  if (dirPath == "fixme" || dirPath.empty()) {
    cerr << "syncJSON: --dir <cdb-root> required" << endl;
    return 1;
  }

  string urlString = host + ":5050/cdb/";
  cout << "urlString: " << urlString << endl;
  cdbRest* pDB = new cdbRest(urlString, 0);

  vector<string> testdirs{dirPath,
                          pathJoin(dirPath, "globaltags"),
                          pathJoin(dirPath, "tags"),
                          pathJoin(dirPath, "payloads"),
                          pathJoin(dirPath, "runrecords"),
                          pathJoin(dirPath, "configs")};
  for (const string& it : testdirs) {
    DIR* folder = opendir(it.c_str());
    if (folder == NULL) {
      cout << "creating " << it << endl;
      system(string("mkdir -p " + it).c_str());
    } else {
      closedir(folder);
    }
  }

  vector<string> vGlobalTags = pDB->readGlobalTags();

  if (!rdbOnly) {
    if (syncKind == "all") {
      bool filterOneGt = !syncName.empty();
      for (const string& gt : vGlobalTags) {
        if (pattern != "unset" && string::npos == gt.find(pattern)) {
          continue;
        }
        if (filterOneGt && gt != syncName) {
          continue;
        }
        cout << "global tag: " << gt << " (deep)" << endl;
        syncGtDeep(pDB, dirPath, gt);
      }
    } else if (syncKind == "gt") {
      if (syncName.empty() || syncName == "unset") {
        cerr << "syncJSON: --sync gt requires --name <gt> (or legacy -e/-p)" << endl;
        delete pDB;
        return 1;
      }
      cout << "global tag: " << syncName << (deep ? " (deep)" : " (shallow)") << endl;
      if (deep) {
        syncGtDeep(pDB, dirPath, syncName);
      } else {
        syncGtShallow(pDB, dirPath, syncName);
      }
    } else if (syncKind == "tag") {
      if (syncName.empty() || syncName == "unset") {
        cerr << "syncJSON: --sync tag requires --name <tag>" << endl;
        delete pDB;
        return 1;
      }
      cout << "tag: " << syncName << (deep ? " (deep)" : " (shallow)") << endl;
      if (deep) {
        syncTagDeep(pDB, dirPath, syncName);
      } else {
        syncTagShallow(pDB, dirPath, syncName);
      }
    } else if (syncKind == "payload") {
      if (syncName.empty() || syncName == "unset") {
        cerr << "syncJSON: --sync payload requires --name <tagDir|hash>" << endl;
        delete pDB;
        return 1;
      }
      cout << "payload: " << syncName << endl;
      syncPayloadSelection(pDB, dirPath, syncName);
    } else if (!syncKind.empty()) {
      cerr << "syncJSON: unknown --sync " << syncKind << endl;
      delete pDB;
      return 1;
    } else {
      cerr << "syncJSON: specify --sync all|gt|tag|payload (or legacy -m)" << endl;
      delete pDB;
      return 1;
    }
  }

  if (!cdbOnly) {
    vector<string> vRunNumbers;
    if (runfile == "unset") {
      vRunNumbers = pDB->getAllRunNumbers();
    } else {
      ifstream file(runfile);
      string line;
      string fileContent;
      while (getline(file, line)) {
        fileContent += line + "\n";
      }
      file.close();
      replaceAll(fileContent, "\n", "");
      replaceAll(fileContent, " ", "");
      replaceAll(fileContent, "{", "");
      replaceAll(fileContent, "}", "");
      vRunNumbers = split(fileContent, ',');
    }
    cout << "total number of runs: " << vRunNumbers.size() << endl;
    cout << "all = " << all << endl;
    for (unsigned int it = 0; it < vRunNumbers.size(); ++it) {
      int irun = stoi(vRunNumbers[it]);
      if (irun < firstRun) continue;
      if (lastRun > -1 && irun > lastRun) continue;
      runRecord rr = pDB->getRunRecord(irun);
      if (all || rr.isSignificant()) {
        cout << rr.printSummary() << endl;
        string subpath = runRecordSubPathFromRun(irun);
        string filepath = pathJoin(pathJoin(dirPath, "runrecords"), subpath);
        mkdirParentsForFile(filepath);
        ofstream ofs(filepath);
        ofs << rr.json() << endl;
        ofs.close();
      }
    }
  }
  delete pDB;
  return 0;
}
