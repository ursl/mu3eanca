#include <cstdlib>
#include <iostream>
#include <string>
#include <string.h>

using namespace std;

// ----------------------------------------------------------------------
// deleteDetConfig — remove all documents in MongoDB detconfigs for a tag (via REST).
//
// Usage:
//   deleteDetConfig --host mu3edb0 --tag <tagname>
//   deleteDetConfig --host mu3edb0 --listtags    (-l)
//
// REST:
//   curl -fS -X DELETE "http://host:5050/cdb/deleteDetconfigTag?tag=..."
//   curl -fsS "http://host:5050/cdb/detconfigTags"
// ----------------------------------------------------------------------

int main(int argc, char* argv[]) {
  string tag("nada"), host("mu3edb0");
  bool listtags(false);

  for (int i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "--host") && i + 1 < argc) {
      host = string(argv[++i]);
    } else if (!strcmp(argv[i], "-h") && i + 1 < argc) {
      host = string(argv[++i]);
    } else if (!strcmp(argv[i], "--listtags")) {
      listtags = true;
    } else if (!strcmp(argv[i], "-l")) {
      listtags = true;
    } else if (!strcmp(argv[i], "--tag") && i + 1 < argc) {
      tag = string(argv[++i]);
    } else if (!strcmp(argv[i], "-t") && i + 1 < argc) {
      tag = string(argv[++i]);
    }
  }

  if (listtags) {
    string url = "http://" + host + ":5050/cdb/detconfigTags";
    string cmd = string("curl -fSs \"") + url + "\"";
    int st = system(cmd.c_str());
    if (st != 0) {
      cerr << "deleteDetConfig: list tags failed (curl exit " << st << ")" << endl;
      return 1;
    }
    return 0;
  }

  if (tag == "nada") {
    cerr << "deleteDetConfig: --tag <name> is required" << endl;
    cerr << "Usage: deleteDetConfig --host <host> --tag <tagname>" << endl;
    cerr << "       deleteDetConfig --host <host> --listtags  (-l)" << endl;
    return 1;
  }

  string url = "http://" + host + ":5050/cdb/deleteDetconfigTag?tag=" + tag;
  string cmd = string("curl -fS -X DELETE \"") + url + "\"";
  cout << "Deleting detconfigs tag ->" << tag << "<- via REST" << endl;
  cout << "cmd: " << cmd << endl;
  int st = system(cmd.c_str());
  cout << endl;
  if (st != 0) {
    cerr << "deleteDetConfig: delete failed (curl exit " << st << "; tag missing or HTTP error?)" << endl;
    return 1;
  }
  return 0;
}
