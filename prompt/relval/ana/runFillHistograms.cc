#include "fillHist.hh"

#include <iostream>
#include <string>

using namespace std;

// ----------------------------------------------------------------------
int main(int argc, char **argv) {
  string outdir = ".";
  string outfile = "nada";
  string infile = "";
  string tree = "frames";
  string mode = "relval";
  string annotation = "";
  for (int i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "--mode")) { mode = argv[++i]; }
    if (!strcmp(argv[i], "--in")) { infile = argv[++i]; }
    if (!strcmp(argv[i], "--out")) { outfile = argv[++i]; }
    if (!strcmp(argv[i], "--outdir")) { outdir = argv[++i]; }
    if (!strcmp(argv[i], "--ann")) { annotation = argv[++i]; }
    if (!strcmp(argv[i], "--tree")) { tree = argv[++i]; }
  }

  if (outfile == "nada") {
    string baseinfilename = infile.substr(infile.find_last_of('/') + 1);
    baseinfilename = baseinfilename.substr(0, baseinfilename.find_last_of('.'));
    outfile = "histograms-" + (annotation != "" ? annotation + "-": "") + baseinfilename + ".root";
    outfile = outdir + "/" + outfile;
    cout << "runFillHistograms() INFO: output file not specified, using default " << outfile << endl;
  }

  fillHist fhFrames(infile, outfile);
  fhFrames.setupTrees();
  fhFrames.bookHist("relval", annotation);
  fhFrames.run();

  return 0;
}
