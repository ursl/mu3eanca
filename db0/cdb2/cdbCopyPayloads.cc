#include <iostream>
#include <string>
#include <fstream>
#include <cstring>
#include <dirent.h>

#include "cdbUtil.hh"

using namespace std;

// ----------------------------------------------------------------------
// cdbCopyPayloads
// ---------------
// Copy payloads from a flat production directory into a CDB JSON payload
// directory, organizing them into tag/block layout:
//   payloads/<tag>/<block>/tag_<tagname>_iov_<run>
//
// Usage: ./cdbCopyPayloads -s <srcdir> -j <jsondir> [-p pattern]
//
//   -s, --src     Source directory (flat payloads from production)
//   -j, --jsondir CDB JSON directory (payloads go to jsondir/payloads/)
//   -p, --pat     Optional pattern to match payload filenames
//
// Creates tag directories and 4-digit block subdirs as needed.
// ----------------------------------------------------------------------

void printUsage(const char* progname) {
  cerr << "Usage: " << progname << " -s <srcdir> -j <jsondir> [-p pattern]" << endl;
  cerr << "  -s, --src     Source directory with flat payloads" << endl;
  cerr << "  -j, --jsondir CDB JSON directory (output: jsondir/payloads/<tag>/<block>/)" << endl;
  cerr << "  -p, --pat     Optional pattern to match in payload filenames" << endl;
}

// ----------------------------------------------------------------------
// Check if filename looks like a payload (tag_<tagname>_iov_<run>)
bool isPayloadFilename(const string& name) {
  if (name.size() < 10) return false;
  if (name.compare(0, 4, "tag_") != 0) return false;
  if (name.find("_iov_") == string::npos) return false;
  return true;
}

// ----------------------------------------------------------------------
// Copy file from src to dst
bool copyFile(const string& src, const string& dst) {
  ifstream in(src, ios::binary);
  if (!in) {
    cerr << "cdbCopyPayloads: Cannot open " << src << endl;
    return false;
  }
  ofstream out(dst, ios::binary);
  if (!out) {
    cerr << "cdbCopyPayloads: Cannot create " << dst << endl;
    return false;
  }
  out << in.rdbuf();
  return in.good() && out.good();
}

// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {
  string srcdir;
  string jsondir;
  string pattern;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) srcdir = argv[++i];
    else if (strcmp(argv[i], "--src") == 0 && i + 1 < argc) srcdir = argv[++i];
    else if (strcmp(argv[i], "-j") == 0 && i + 1 < argc) jsondir = argv[++i];
    else if (strcmp(argv[i], "--jsondir") == 0 && i + 1 < argc) jsondir = argv[++i];
    else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) pattern = argv[++i];
    else if (strcmp(argv[i], "--pat") == 0 && i + 1 < argc) pattern = argv[++i];
    else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      printUsage(argv[0]);
      return 0;
    }
  }

  if (srcdir.empty() || jsondir.empty()) {
    cerr << "Error: -s srcdir and -j jsondir are required" << endl;
    printUsage(argv[0]);
    return 1;
  }

  string payloaddir = jsondir + "/payloads";
  cout << "cdbCopyPayloads: copying from " << srcdir << " to " << payloaddir << endl;
  if (!pattern.empty()) cout << "  pattern: " << pattern << endl;

  DIR* dir = opendir(srcdir.c_str());
  if (!dir) {
    cerr << "cdbCopyPayloads: Cannot open directory " << srcdir << endl;
    return 1;
  }

  int copied = 0;
  int skipped = 0;
  struct dirent* entry;

  while ((entry = readdir(dir)) != nullptr) {
    string name = entry->d_name;
    if (name == "." || name == "..") continue;
    if (entry->d_type != 8 && entry->d_type != 0) continue;  // regular file or unknown

    if (!isPayloadFilename(name)) continue;
    if (!pattern.empty() && name.find(pattern) == string::npos) continue;

    string subpath = payloadSubPathFromHash(name);
    if (subpath == name) {
      cout << "cdbCopyPayloads: skipping (not parseable): " << name << endl;
      skipped++;
      continue;
    }

    string destpath = payloaddir + "/" + subpath;
    string destdir = destpath.substr(0, destpath.rfind('/'));
    string mkdir_cmd = "mkdir -p " + destdir;
    if (system(mkdir_cmd.c_str()) != 0) {
      cerr << "cdbCopyPayloads: Failed to create " << destdir << endl;
      skipped++;
      continue;
    }

    string srcpath = srcdir + "/" + name;
    if (copyFile(srcpath, destpath)) {
      cout << "  " << name << " -> " << subpath << endl;
      copied++;
    } else {
      skipped++;
    }
  }

  closedir(dir);
  cout << "cdbCopyPayloads: copied " << copied << " payloads" << (skipped > 0 ? ", skipped " + to_string(skipped) : "") << endl;
  return 0;
}
