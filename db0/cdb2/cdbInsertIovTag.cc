#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <dirent.h>
#include <set>

#include "cdbUtil.hh"

using namespace std;

// ----------------------------------------------------------------------
// cdbInsertIovTag
// ===============
//
// Usage:   cdbInsertIovTag -j jsondir -t tag -i 265
//          cdbInsertIovTag -j jsondir -t tag -r 265
//          cdbInsertIovTag -j jsondir -t tag -c
//          cdbInsertIovTag -j jsondir -t tag -init
//          cdbInsertIovTag -j jsondir -t tag -d payloaddir -p pat
//          ./bin/cdbInsertIovTag -t pixelqualitylm_datav6.5=2025V0 -j /Users/ursl/data/mu3e/test-cdb \
//            -d ~/data/mu3e/test-cdb/payloads/pixelqualitylm_datav6.5=2025V0/ -p pixelqualitylm_datav6.5=2025V0
//
// History
//         2023/08/21 first shot
//         2023/08/22 fix bug removing previous entries
//         2025/11/24 Cursor-translated from Perl to C++
// ----------------------------------------------------------------------
// Send all questions, wishes and complaints to the 
//
// Author    Urs Langenegger <urslangenegger@gmail.com>
// ----------------------------------------------------------------------

void printUsage(const char* progname) {
  cerr << "Usage: " << progname << " -j jsondir -t tag [-i run] [-r run] [-c] [-d payloaddir -p pat]" << endl;
  cerr << "  -j jsondir    : JSON directory path" << endl;
  cerr << "  -t tag        : Tag name" << endl;
  cerr << "  -i run        : Insert run number (keeps sorted order)" << endl;
  cerr << "  -r run        : Remove run number" << endl;
  cerr << "  -c            : Clear IOVs to [1]" << endl;
  cerr << "  -d payloaddir : Payload directory (for meta mode: extract runs from payloads)" << endl;
  cerr << "  -p pat        : Pattern to match in payload filenames (for meta mode: extract runs from payloads)" << endl;
}

// ----------------------------------------------------------------------
// Read IOVs and optional comment from a tag file
// Returns pair of (runs, comment). Comment is empty if not present.
pair<vector<int>, string> readIOVsAndCommentFromFile(const string& filepath) {
  vector<int> runs;
  string comment;

  ifstream in(filepath);
  if (!in) {
    cerr << "insertIovTag: Cannot open " << filepath << endl;
    return make_pair(runs, comment);
  }

  // Read entire file content
  string content((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
  in.close();

  // Extract optional comment before normalizing
  string commentVal = jsonGetString(content, "comment");
  if (!commentVal.empty() && commentVal.find("parseError") == string::npos) {
    comment = commentVal;
  }

  // Remove all newlines and spaces (like Perl script)
  content.erase(remove(content.begin(), content.end(), '\n'), content.end());
  content.erase(remove(content.begin(), content.end(), ' '), content.end());

  // Parse the IOVs array
  // Pattern: {"tag":"tagname","iovs":[1,2,3,...],"comment":"..."}
  size_t iovsPos = content.find("\"iovs\":[");
  if (iovsPos != string::npos) {
    size_t lbr = content.find("[", iovsPos);
    size_t rbr = content.find("]", lbr);
    if (lbr != string::npos && rbr != string::npos && rbr > lbr) {
      string arr = content.substr(lbr + 1, rbr - lbr - 1);
      if (!arr.empty()) {
        stringstream ss(arr);
        string tok;
        while (getline(ss, tok, ',')) {
          if (!tok.empty()) {
            try {
              runs.push_back(stoi(tok));
            } catch (...) {
              // Skip invalid tokens
            }
          }
        }
      }
    }
  }
  
  return make_pair(runs, comment);
}

// ----------------------------------------------------------------------
// Insert a run number into the vector, maintaining sorted order
void insertRunIntoVector(vector<int>& runs, int runNumber) {
  // Check if already exists
  for (size_t i = 0; i < runs.size(); i++) {
    if (runs[i] == runNumber) {
      cout << "insertIovTag: same, not updating" << endl;
      return;
    }
    if (runNumber < runs[i]) {
      runs.insert(runs.begin() + i, runNumber);
      return;
    }
  }
  // If not found, append
  runs.push_back(runNumber);
}

// ----------------------------------------------------------------------
// Remove a run number from the vector
void removeRunFromVector(vector<int>& runs, int runNumber) {
  vector<int> filtered;
  for (int r : runs) {
    if (r != runNumber) {
      filtered.push_back(r);
    }
  }
  runs = filtered;
}

// ----------------------------------------------------------------------
// Create backup of a file
bool backupFile(const string& filepath) {
  string oldfilepath = filepath.substr(0, filepath.find_last_of('/')) + "/../old/";
  if (!fileExists(oldfilepath)) {
    system(string("mkdir -p " + oldfilepath).c_str());
  }

  string cmd = "mv " + filepath + " " + oldfilepath;
  if (system(cmd.c_str()) != 0) {
    cerr << "insertIovTag: Failed to mv backup from " << filepath << " to " << oldfilepath << endl;
    return false;
  }
  return true;
}

// ----------------------------------------------------------------------
// Write IOVs and optional comment to a tag file
bool writeIOVsToFile(const string& filepath, const string& tag, const vector<int>& runs, bool clear, const string& comment = "") {
  ofstream out(filepath);
  if (!out) {
    cout << "insertIovTag: Cannot open " << filepath << " for output" << endl;
    return false;
  }

  out << "{\"tag\" : \"" << tag << "\", \"iovs\" : [";
  if (clear) {
    out << "1";
  } else {
    for (size_t i = 0; i < runs.size(); i++) {
      out << runs[i];
      if (i + 1 < runs.size()) {
        out << ", ";
      }
    }
  }
  out << "]";
  if (!comment.empty()) {
    out << ", \"comment\" : \"" << escapeJsonString(comment) << "\"";
  }
  out << "}\n";
  out.close();
  cout << "insertIovTag: wrote " << filepath << " with " << runs.size() << " runs" << endl;
  return true;
}

// ----------------------------------------------------------------------
// Extract run number from payload filename (tag_<tagname>_iov_<run>)
// Returns -1 if not parseable.
static int extractRunFromPayloadFilename(const string& filename) {
  size_t iovPos = filename.find("_iov_");
  if (iovPos == string::npos) return -1;
  string runStr = filename.substr(iovPos + 5);
  size_t dotPos = runStr.find('.');
  if (dotPos != string::npos) runStr = runStr.substr(0, dotPos);
  try {
    return stoi(runStr);
  } catch (...) {
    return -1;
  }
}

// ----------------------------------------------------------------------
// Collect run numbers from payload files in a directory (flat or block subdir).
// Adds to uniqueRuns and runs. Block subdirs are 4-digit (0000, 0001, ...).
static void collectRunsFromDir(const string& dirpath, const string& pattern,
                               set<int>& uniqueRuns, vector<int>& runs) {
  DIR* d = opendir(dirpath.c_str());
  if (!d) return;
  auto is4DigitBlock = [](const string& name) {
    if (name.size() != 4) return false;
    for (char c : name) if (c < '0' || c > '9') return false;
    return true;
  };
  struct dirent* e;
  while ((e = readdir(d)) != nullptr) {
    string name(e->d_name);
    if (name == "." || name == "..") continue;
    string full = dirpath + "/" + name;
    bool isDir = (e->d_type == 4);
    if (e->d_type == 0) isDir = false;  // unknown: treat as file
    if (isDir && is4DigitBlock(name)) {
      collectRunsFromDir(full, pattern, uniqueRuns, runs);
    } else if (e->d_type == 8 || e->d_type == 0) {
      if (name.find(pattern) != string::npos) {
        int run = extractRunFromPayloadFilename(name);
        if (run >= 0 && uniqueRuns.insert(run).second) runs.push_back(run);
      }
    }
  }
  closedir(d);
}

// ----------------------------------------------------------------------
// Extract run numbers from payload filenames matching pattern.
// Traverses entire directory structure: payloaddir, tag subdirs, and block subdirs.
// Layout: payloads/<tag>/<block>/tag_<tagname>_iov_<run>
// Also supports legacy flat layout: payloads/tag_<tagname>_iov_<run>
vector<int> extractRunsFromPayloads(const string& payloaddir, const string& pattern) {
  vector<int> runs;
  set<int> uniqueRuns;

  DIR* dir = opendir(payloaddir.c_str());
  if (!dir) {
    cerr << "insertIovTag: Cannot open directory " << payloaddir << endl;
    return runs;
  }

  struct dirent* entry;
  while ((entry = readdir(dir)) != nullptr) {
    string name = entry->d_name;
    if (name == "." || name == "..") continue;

    string fullpath = payloaddir + "/" + name;
    bool isDir = (entry->d_type == 4);
    if (entry->d_type == 0) isDir = false;

    if (isDir) {
      if (name.find(pattern) != string::npos) {
        // Matching tag directory: traverse into block subdirs
        collectRunsFromDir(fullpath, pattern, uniqueRuns, runs);
      } else {
        // May be block subdir (0000, 0001) if payloaddir is tag dir
        auto is4Digit = [](const string& s) {
          if (s.size() != 4) return false;
          for (char c : s) if (c < '0' || c > '9') return false;
          return true;
        };
        if (is4Digit(name)) collectRunsFromDir(fullpath, pattern, uniqueRuns, runs);
      }
    } else if (entry->d_type == 8 || entry->d_type == 0) {
      // Flat file (legacy layout)
      if (name.find(pattern) != string::npos) {
        int run = extractRunFromPayloadFilename(name);
        if (run >= 0 && uniqueRuns.insert(run).second) runs.push_back(run);
      }
    }
  }

  closedir(dir);
  sort(runs.begin(), runs.end());
  return runs;
}

// ----------------------------------------------------------------------
// Process a single tag file: insert or remove runs
int processTagFile(const string& jsondir, const string& tag, int insertRun, int removeRun, bool clear) {
  // Construct file path
  string file = jsondir + "/tags/" + tag;

  // Read existing IOVs and comment
  auto pr = readIOVsAndCommentFromFile(file);
  vector<int> runs = pr.first;
  string comment = pr.second;
  if (runs.empty() && !clear) {
    // If file doesn't exist or is empty, we might need to create it
    // But for now, we'll just proceed
  }

  cout << "insertIovTag: oldRuns = ";
  for (size_t i = 0; i < runs.size(); i++) {
    cout << runs[i];
    if (i + 1 < runs.size()) cout << " ";
  }
  cout << endl;

  // Modify the runs list
  if (removeRun > 0) {
    removeRunFromVector(runs, removeRun);
    cout << "insertIovTag: removed run " << removeRun << ", new list = ";
    for (size_t i = 0; i < runs.size(); i++) {
      cout << runs[i];
      if (i + 1 < runs.size()) cout << " ";
    }
    cout << endl;
  } else if (insertRun > 0) {
    insertRunIntoVector(runs, insertRun);
  }

  cout << "insertIovTag: " << tag << ": ";
  for (size_t i = 0; i < runs.size(); i++) {
    cout << runs[i];
    if (i + 1 < runs.size()) cout << " ";
  }
  cout << ", last index = " << (runs.empty() ? -1 : (int)runs.size() - 1) << endl;

  // Create backup
  if (!backupFile(file)) {

  }

  // Write updated file (preserve comment)
  if (!writeIOVsToFile(file, tag, runs, clear, comment)) {
    return 1;
  }

  return 0;
}

// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {
  string jsondir;
  string tag;
  string payloaddir;
  string pattern;
  int insertRun = 0;
  int removeRun = 0;
  bool clear = false;
  bool init = false;

  // -- Parse command line arguments
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-init") == 0) init = true;
    if (strcmp(argv[i], "-j") == 0 && i + 1 < argc) jsondir = argv[++i];
    if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) tag = argv[++i];
    if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) insertRun = stoi(argv[++i]);
    if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) removeRun = stoi(argv[++i]);
    if (strcmp(argv[i], "-c") == 0)                 clear = true;
    if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) payloaddir = argv[++i];
    if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) pattern = argv[++i];
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      printUsage(argv[0]);
      return 0;
    }
  }

  // -- Init mode: create a new tag file with IOV 1
  if (init) {
    if (jsondir.empty() || tag.empty()) {
      cerr << "Error: -j jsondir and -t tag are required for init mode" << endl;
      printUsage(argv[0]);
      return 1;
    }
    if (!writeIOVsToFile(jsondir + "/tags/" + tag, tag, {1}, false, "")) {
      return 1;
    }
    return 0;
  }

  // -- Meta mode: extract runs from payloads
  if (!payloaddir.empty() && !pattern.empty()) {
    if (jsondir.empty() || tag.empty()) {
      cerr << "Error: -j jsondir and -t tag are required for meta mode" << endl;
      printUsage(argv[0]);
      return 1;
    }
    
    cout << "insertIovTag: meta mode - scanning " << payloaddir << " for pattern " << pattern << endl;
    vector<int> payloadRuns = extractRunsFromPayloads(payloaddir, pattern);
    
    cout << "insertIovTag: found " << payloadRuns.size() << " runs: ";
    for (size_t i = 0; i < payloadRuns.size(); i++) {
      cout << payloadRuns[i];
      if (i + 1 < payloadRuns.size()) cout << " ";
    }
    cout << endl;
    
    // Read existing IOVs and comment once
    string file = jsondir + "/tags/" + tag;
    auto pr = readIOVsAndCommentFromFile(file);
    vector<int> runs = pr.first;
    string comment = pr.second;
    
    cout << "insertIovTag: oldRuns = ";
    for (size_t i = 0; i < runs.size(); i++) {
      cout << runs[i];
      if (i + 1 < runs.size()) cout << " ";
    }
    cout << endl;
    
    // Insert all found runs
    for (int run : payloadRuns) {
      insertRunIntoVector(runs, run);
    }
    
    cout << "insertIovTag: " << tag << ": ";
    for (size_t i = 0; i < runs.size(); i++) {
      cout << runs[i];
      if (i + 1 < runs.size()) cout << " ";
    }
    cout << ", last index = " << (runs.empty() ? -1 : (int)runs.size() - 1) << endl;
    
    // Create backup and write once (preserve comment)
    if (!backupFile(file)) {
    }
    if (!writeIOVsToFile(file, tag, runs, false, comment)) {
      return 1;
    }
    
    return 0;
  }

  // Normal mode: validate required arguments
  if (jsondir.empty() || tag.empty()) {
    cerr << "Error: -j jsondir and -t tag are required" << endl;
    printUsage(argv[0]);
    return 1;
  }

  if (insertRun > 0 && removeRun > 0) {
    cerr << "Error: Cannot specify both -i and -r" << endl;
    return 1;
  }

  if (!insertRun && !removeRun && !clear) {
    cerr << "Error: Must specify one of -i, -r, or -c" << endl;
    return 1;
  }

  // Process the tag file
  return processTagFile(jsondir, tag, insertRun, removeRun, clear);
}
