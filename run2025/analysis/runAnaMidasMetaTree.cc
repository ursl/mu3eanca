#include <TFile.h>
#include <TTree.h>
#include <iostream>
#include <string>

#include "anaMidasMetaTree.hh"

// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <input.root> [maxEntries]" << std::endl;
    return 1;
  }

  std::string inputPath = argv[1];
  Long64_t maxEntries = -1;
  if (argc >= 3) {
    try { maxEntries = std::stoll(argv[2]); }
    catch (...) { maxEntries = -1; }
  }

  TFile* f = TFile::Open(inputPath.c_str(), "READ");
  if (!f || f->IsZombie()) {
    std::cerr << "Error: cannot open input file: " << inputPath << std::endl;
    return 2;
  }

  TTree* t = static_cast<TTree*>(f->Get("midasMetaTree"));
  if (!t) {
    std::cerr << "Error: TTree 'midasMetaTree' not found in " << inputPath << std::endl;
    f->Close();
    return 3;
  }

  anaMidasMetaTree ana(t);
  ana.bookHistograms();
  ana.loop(maxEntries);
  ana.endAnalysis();
  f->Close();
  return 0;
}


