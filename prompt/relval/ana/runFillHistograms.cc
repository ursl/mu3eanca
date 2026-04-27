#include "fillHist.hh"

#include <iostream>
#include <string>

using namespace std;

// ----------------------------------------------------------------------
int main(int argc, char **argv) {
  string outfile = "histograms.root";
  string infile = "";
  string tree = "frames";
  string mode = "relval";
  int nevents = -1;
  for (int i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "--mode")) { mode = argv[++i]; }
    if (!strcmp(argv[i], "-n")) { nevents = atoi(argv[++i]); }
    if (!strcmp(argv[i], "--nevts")) { nevents = atoi(argv[++i]); }
    if (!strcmp(argv[i], "--in")) { infile = argv[++i]; }
    if (!strcmp(argv[i], "--out")) { outfile = argv[++i]; }
    if (!strcmp(argv[i], "--tree")) { tree = argv[++i]; }
  }

  fillHist fh(tree, outfile);
  fh.setupTree(infile);
  fh.bookHist("relval");
  fh.run(nevents);
  
  return 0;
}
