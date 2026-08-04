
#include <dirent.h>  /// for directory reading
#include <fstream>
#include <iostream>
#include <algorithm>  // for std::sort
#include <vector>     // for std::vector
#include <string>     // for std::string
#include <string.h>   // for strcmp
#include "cdbUtil.hh"


using namespace std;


// ----------------------------------------------------------------------
// -- downloadDetConfig
// ------------------
//
// -- download (binary) files to a directory, using a "tag" from the detconfigs collection
//
// -- Usage:
//    downloadDetConfig --dir /Users/ursl/Downloads/tdac_files_bu_06_10 --tag tdac_files_bu_06_10
//
// -- Retrieval of all files corresponding to "tag" (zip written as <dir>/<tag>.zip, not PWD)
//    curl -fSL --output /path/tag.zip "http://mu3edb0.psi.ch/detconfigs/downloadTag?tag=..."
//
// -- List all detconfigs tags (one per line):
//    downloadDetConfig --listtags
//    curl -fsS "http://mu3edb0.psi.ch/detconfigs/detconfigTags"
//
// -- Direct to Node (bypass nginx): --host localhost --port 5050
// ----------------------------------------------------------------------



// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {
  
  string dirPath("."), tag("nada"), host("mu3edb0.psi.ch");
  int port(0);
  bool listtags(false);
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "--dir"))     {dirPath = string(argv[++i]);}
    if (!strcmp(argv[i], "-d"))        {dirPath = string(argv[++i]);}
    if (!strcmp(argv[i], "--host"))    {host = string(argv[++i]);}
    if (!strcmp(argv[i], "-h"))        {host = string(argv[++i]);}
    if (!strcmp(argv[i], "--port") && i + 1 < argc) {port = atoi(argv[++i]);}
    if (!strcmp(argv[i], "--listtags")) {listtags = true;}
    if (!strcmp(argv[i], "-l"))        {listtags = true;}
    if (!strcmp(argv[i], "--tag"))     {tag = string(argv[++i]);}
    if (!strcmp(argv[i], "-t"))        {tag = string(argv[++i]);}
  }

  string restBase = detconfigsRestBase(host, port);

  if (listtags) {
    string url = restBase + "/detconfigTags";
    string cmd = string("curl -fSs \"") + url + "\"";
    int st = system(cmd.c_str());
    if (st != 0) {
      cerr << "downloadDetConfig: list tags failed (curl exit " << st << ")" << endl;
      return 1;
    }
    return 0;
  }

  // -- check if tag is specified
  if ("nada" == tag) {
    cout << "Tag is not specified" << endl;
    cout << "Usage: downloadDetConfig [--host mu3edb0.psi.ch] [--port 5050] --dir <path> --tag <tagname>" << endl;
    cout << "       downloadDetConfig [--host mu3edb0.psi.ch] --listtags   (-l)" << endl;
    return 1;
  }

  string url = restBase + "/downloadTag?tag=" + tag;
  cout << "Downloading from ->" << url << "<-" << endl;
  // -O writes to PWD; use --output so the zip lands under --dir (quote paths for spaces).
  string outZip = dirPath + "/" + tag + ".zip";
  string cmd = string("mkdir -p \"") + dirPath + "\" && curl -fSL --output \"" + outZip + "\" \"" + url + "\"";
  cout << "cmd: " << cmd << endl;
  system(cmd.c_str());
  
  string tagdir = dirPath + "/" + tag;
  // -- Check if the file was downloaded
  if (!fileExists(outZip)) {
    cout << "Error: Failed to download the file" << endl;
    return 1;
  } else {
    cout << "File downloaded successfully" << endl;
    cmd = string("mkdir -p \"") + tagdir + "\"";
    cout << "cmd: " << cmd << endl;
    system(cmd.c_str());
      
    // -- Unzip the file
    cmd = string("unzip \"") + outZip + "\" -d \"" + tagdir + "\"";
    cout << "cmd: " << cmd << endl;
    system(cmd.c_str());
  }  
  return 0;
}
