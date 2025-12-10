#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <dirent.h>
#include <set>

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
  cerr << "  -d payloaddir : Payload directory (for meta mode)" << endl;
  cerr << "  -p pat        : Pattern to match in payload filenames (for meta mode)" << endl;
}

// ----------------------------------------------------------------------
// Read IOVs from a tag file
vector<int> readIOVsFromFile(const string& filepath) {
  vector<int> runs;
  
  ifstream in(filepath);
  if (!in) {
    cerr << "insertIovTag: Cannot open " << filepath << endl;
    return runs;
  }

  // Read entire file content
  string content((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
  in.close();

  // Remove all newlines and spaces (like Perl script)
  content.erase(remove(content.begin(), content.end(), '\n'), content.end());
  content.erase(remove(content.begin(), content.end(), ' '), content.end());

  // Parse the IOVs array
  // Pattern: {"tag":"tagname","iovs":[1,2,3,...]}
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
  
  return runs;
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
  string cmd = "mv " + filepath + " " + oldfilepath;
  if (system(cmd.c_str()) != 0) {
    cerr << "insertIovTag: Failed to mv backup from " << filepath << " to " << oldfilepath << endl;
    return false;
  }
  return true;
}

// ----------------------------------------------------------------------
// Write IOVs to a tag file
bool writeIOVsToFile(const string& filepath, const string& tag, const vector<int>& runs, bool clear) {
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
  out << "]}\n";
  out.close();
  cout << "insertIovTag: wrote " << filepath << " with " << runs.size() << " runs" << endl;
  return true;
}

// ----------------------------------------------------------------------
// Extract run numbers from payload filenames matching pattern
// Filenames are like: tag_pixelqualitylm_mcidealv6.1_iov_265
vector<int> extractRunsFromPayloads(const string& payloaddir, const string& pattern) {
  vector<int> runs;
  set<int> uniqueRuns;  // Use set to avoid duplicates
  
  DIR* dir = opendir(payloaddir.c_str());
  if (!dir) {
    cerr << "insertIovTag: Cannot open directory " << payloaddir << endl;
    return runs;
  }
  
  struct dirent* entry;
  while ((entry = readdir(dir)) != nullptr) {
    string filename = entry->d_name;
    
    // Skip . and ..
    if (filename == "." || filename == "..") continue;
    
    // Check if filename contains pattern
    if (filename.find(pattern) == string::npos) continue;
    
    // Extract run number after _iov_
    size_t iovPos = filename.find("_iov_");
    if (iovPos != string::npos) {
      string runStr = filename.substr(iovPos + 5);  // +5 for "_iov_"
      // Remove any extension
      size_t dotPos = runStr.find('.');
      if (dotPos != string::npos) {
        runStr = runStr.substr(0, dotPos);
      }
      try {
        int run = stoi(runStr);
        if (uniqueRuns.insert(run).second) {  // Insert returns true if new
          runs.push_back(run);
        }
      } catch (...) {
        // Skip invalid run numbers
      }
    }
  }
  
  closedir(dir);
  
  // Sort the runs
  sort(runs.begin(), runs.end());
  
  return runs;
}

// ----------------------------------------------------------------------
// Process a single tag file: insert or remove runs
int processTagFile(const string& jsondir, const string& tag, int insertRun, int removeRun, bool clear) {
  // Construct file path
  string file = jsondir + "/tags/" + tag;

  // Read existing IOVs
  vector<int> runs = readIOVsFromFile(file);
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

  // Write updated file
  if (!writeIOVsToFile(file, tag, runs, clear)) {
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
    ofstream out(jsondir + "/tags/" + tag);
    out << "{\"tag\" : \"" << tag << "\", \"iovs\" : [1]}" << endl;
    out.close();
    return 0;
  }

  // Meta mode: extract runs from payloads
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
    
    // Read existing IOVs once
    string file = jsondir + "/tags/" + tag;
    vector<int> runs = readIOVsFromFile(file);
    
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
    
    // Create backup and write once
    if (!backupFile(file)) {
    }
    if (!writeIOVsToFile(file, tag, runs, false)) {
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
