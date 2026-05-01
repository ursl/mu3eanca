#include "compareHist.hh"

#include <iostream>
#include <string>

using namespace std;

// ----------------------------------------------------------------------
int main(int argc, char **argv) {
  string dirname = ".";
  string infile1 = "";
  string infile2 = "";
  string mode = "relval";

  for (int i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "--mode")) { mode = argv[++i]; }
    if (!strcmp(argv[i], "--in1")) { infile1 = argv[++i]; }
    if (!strcmp(argv[i], "--in2")) { infile2 = argv[++i]; }
    if (!strcmp(argv[i], "--dir")) { dirname = argv[++i]; }
  }

  if (infile1.empty() || infile2.empty()) {
    cout << "runCompareHistograms() ERROR: please provide --in1 and --in2" << endl;
    return 1;
  }

  compareHist cmp(infile1, infile2);
  cmp.setupHists(mode);
  cmp.run(dirname);
  cmp.makeSummaryPDF(dirname);

  return 0;
}
