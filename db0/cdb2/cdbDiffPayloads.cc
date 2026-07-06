#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdio>

#include "calAbs.hh"
#include "calPixelQualityLM.hh"
#include "Mu3eCalFactory.hh"

using namespace std;

// ----------------------------------------------------------------------
static string printPayloadCommon(calAbs *c, const string& hash, int verbose) {
  const payload& pl = c->getPayload(hash);
  string ret = "hash:    " + pl.fHash + "\n";
  ret += "comment: " + pl.fComment + "\n";
  ret += "schema:  " + pl.fSchema + "\n";
  ret += "date:    " + pl.fDate + "\n";
  ret += c->printBLOBString(pl.fBLOB, verbose);
  return ret;
}


// ----------------------------------------------------------------------
// cdbDiffPayloads /path/to/payload1 /path/to/payload2
// ---------------
//
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
int main(int argc, const char* argv[]) {
  string filename1(""), filename2(""), pdir1(""), pdir2(""), hash1(""), hash2("");
  int verbose(10000);
  
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-v")) verbose = atoi(argv[++i]);
  }
  
  if (argc < 3) {
    cout << "provide two payload files" << endl;
    return 0;
  }
  filename1 = argv[1];
  filename2 = argv[2];
  
  pdir1 = filename1.substr(0, filename1.find_last_of("/")+1);
  if (pdir1.empty()) pdir1 = "./";
  hash1 = filename1.substr(filename1.find_last_of("/")+1);
  pdir2 = filename2.substr(0, filename2.find_last_of("/")+1);
  if (pdir2.empty()) pdir2 = "./";
  hash2 = filename2.substr(filename2.find_last_of("/")+1);
  cout << "payload1 ->" << filename1 << "<-" << endl
  << "dir1 ->" << pdir1 << "<-" << endl
  << "hash1 ->" << hash1 << "<-" << endl;
  cout << "payload2 ->" << filename2 << "<-" << endl
  << "dir2 ->" << pdir2 << "<-" << endl
  << "hash2 ->" << hash2 << "<-" << endl;
  
  calAbs *c1 = Mu3eCalFactory::instance()->createClassFromFile(hash1, pdir1);
  if (!c1) {
    cout << "unknown payload type or file not found" << endl;
    return 0;
  }
  if (string::npos != c1->getError().find("Error: file not found")) {
    cout << "file not found" << endl;
    delete c1;
    return 0;
  }
  
  string pl1 = printPayloadCommon(c1, hash1, verbose);
  
  calAbs *c2 = Mu3eCalFactory::instance()->createClassFromFile(hash2, pdir2);
  string pl2 = printPayloadCommon(c2, hash2, verbose);
  
  
  string path1 = "/tmp/cdbDiffPayloads1-" + hash1;
  string path2 = "/tmp/cdbDiffPayloads2-" + hash2;
  
  ofstream out1(path1);
  ofstream out2(path2);
  if (!out1 || !out2) {
    cout << "ERROR: cannot write temp files\n";
    return 2;
  }
  out1 << pl1;
  out2 << pl2;
  out1.close();
  out2.close();
  
  string cmd = "diff -u --label '" + hash1 + "' --label '" + hash2 + "' " + path1 + " " + path2;
  int rc = system(cmd.c_str());   // 0=same, 1=different, 2=error
  
  remove(path1.c_str());
  remove(path2.c_str());
  return rc;
}
