#include <TFile.h>
#include <TTree.h>
#include <iostream>
#include <string>

#include "anaMidasMetaTree.hh"

// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <input.root> [-n maxEntries] [-r run] [-c chipID]" << std::endl;
    return 1;
  }

  std::vector<std::string> files;
  Long64_t maxEntries = -1;
  int run = -1;
  int chipID = -1;
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-r")) { run = atoi(argv[++i]); continue;}
    if (!strcmp(argv[i], "-c")) { chipID = atoi(argv[++i]); continue;}
    if (!strcmp(argv[i], "-n")) { maxEntries = std::stoll(argv[++i]); continue;}
    files.push_back(argv[i]);
  }

  TFile *f(0);
  for (const auto& file : files) {
    f = TFile::Open(file.c_str(), "READ");
    if (!f || f->IsZombie()) {
      std::cerr << "Error: cannot open input file: " << file << std::endl;
      return 2;
    } else {
      break;
    }
  }

  TTree* t = static_cast<TTree*>(f->Get("midasMetaTree"));
  if (!t) {
    std::cerr << "Error: TTree 'midasMetaTree' not found in " << files[0] << std::endl;
    f->Close();
    return 3;
  }

  anaMidasMetaTree ana(t);
  if (run > 0 || chipID > 0) {
    //std::cout << "Printing with run = " << run << " chipID = " << chipID << std::endl;
    ana.print(run, chipID);
  } else {  
    std::cout << "Booking histograms and looping over all entries" << std::endl;
    ana.bookHistograms();
    ana.loop(maxEntries);
    ana.endAnalysis();
  }
  f->Close();
  return 0;
}
