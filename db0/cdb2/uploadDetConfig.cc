
#include <dirent.h>  /// for directory reading
#include <fstream>
#include <iostream>
#include <algorithm>  // for std::sort
#include <vector>     // for std::vector
#include <string.h>
#include <string>     // for std::string

using namespace std;


// ----------------------------------------------------------------------
// -- uploadDetConfig
// ------------------
//
// -- upload (binary) files from a directory with a "tag" to the CDB (detconfigs collection)
//
// -- Usage:
//    uploadDetConfig --dir /Users/ursl/Downloads/tdac_files_bu_06_10 -p mask_chip_ -t tdac_files_bu_06_10
//
// -- Retrieval of all files corresponding to "tag" (result will be in a zip file with name "tag".zip)
//    curl -O -J "http://pc11740:5050/cdb/downloadTag?tag=tdac_files_bu_06_10"
// ----------------------------------------------------------------------



// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {
  
  string dirPath("."), tag("nada"), host("pc11740"), pattern("mask_chip_");
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "--dir"))     {dirPath = string(argv[++i]);}
    if (!strcmp(argv[i], "-d"))        {dirPath = string(argv[++i]);}
    if (!strcmp(argv[i], "--host"))    {host = string(argv[++i]);}
    if (!strcmp(argv[i], "-h"))        {host = string(argv[++i]);}
    if (!strcmp(argv[i], "-t"))        {tag = string(argv[++i]);}
    if (!strcmp(argv[i], "--tag"))     {tag = string(argv[++i]);}
    if (!strcmp(argv[i], "--pattern")) {pattern = string(argv[++i]);}
    if (!strcmp(argv[i], "-p"))        {pattern = string(argv[++i]);}
  }

    // -- check if tag is specified
  if ("nada" == tag) {
    cout << "Tag is not specified" << endl;
    cout << "Usage: uploadDetConfig --dir /path/to/directory -p mask_chip_ -t tag" << endl;
    return 1;
  }


  // -- Access the database and collection
  // string url = "http://" + host + ":5050/cdb/uploadMany";
  string url = "http://" + host + ":5050/cdb/upload";
  cout << "Uploading to ->" << url << "<-" << endl;
  string cmd = "curl -v -F ... -F \"tag=4\" " + url;
  
  vector<string> vfiles;
  DIR *folder;
  struct dirent *entry;
  
  folder = opendir(dirPath.c_str());
  if (folder == NULL) {
    cout << "Unable to read directory ->" << dirPath << "<-" << endl;
    cout << "Usage: uploadDetConfig --dir /path/to/directory -p mask_chip_ -t tag" << endl;
    return 1;
  } else {
  
    while ((entry=readdir(folder))) {
      if (8 == entry->d_type) {
        if (string(entry->d_name).find(pattern) != string::npos) {  
          vfiles.push_back(entry->d_name);
        }
      }
    }
    closedir(folder);
    sort(vfiles.begin(), vfiles.end());
  }
  
  // -- Iterate over the files and store them in MongoDB
  for (const auto& filepath : vfiles) {
    string cmd = "curl -v -F \"file=@" + dirPath + "/" + filepath + "\" -F \"tag=" + tag + "\" " + url;
    cout << "cmd: " << cmd << endl;
    system(cmd.c_str());
  }
  
  return 0;
}
