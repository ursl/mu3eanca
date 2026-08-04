#include <cstdlib>
#include <iostream>
#include <string>
#include <string.h>
#include "cdbUtil.hh"

using namespace std;

// ----------------------------------------------------------------------
// deleteDetConfig — remove all documents in MongoDB detconfigs for a tag (via REST).
//
// Usage:
//   deleteDetConfig --tag <tagname>
//   deleteDetConfig --listtags    (-l)
//
// REST (via nginx on port 80):
//   curl -fS -X DELETE "http://mu3edb0.psi.ch/detconfigs/deleteDetconfigTag?tag=..."
//   curl -fsS "http://mu3edb0.psi.ch/detconfigs/detconfigTags"
//
// Direct to Node: --host localhost --port 5050
// ----------------------------------------------------------------------

int main(int argc, char* argv[]) {
  string tag("nada"), host("mu3edb0.psi.ch");
  int port(0);
  bool listtags(false);

  for (int i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "--host") && i + 1 < argc) {
      host = string(argv[++i]);
    } else if (!strcmp(argv[i], "-h") && i + 1 < argc) {
      host = string(argv[++i]);
    } else if (!strcmp(argv[i], "--port") && i + 1 < argc) {
      port = atoi(argv[++i]);
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

  string restBase = detconfigsRestBase(host, port);

  if (listtags) {
    string url = restBase + "/detconfigTags";
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
    cerr << "Usage: deleteDetConfig [--host mu3edb0.psi.ch] [--port 5050] --tag <tagname>" << endl;
    cerr << "       deleteDetConfig [--host mu3edb0.psi.ch] --listtags  (-l)" << endl;
    return 1;
  }

  string url = restBase + "/deleteDetconfigTag?tag=" + tag;
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
