#include "RelvalAnalyzer.hh"
#include "HistBooking.hh"

#include <ROOT/RDataFrame.hxx>
#include <TFile.h>

#include <iostream>
#include <stdexcept>

RelvalAnalyzer::RelvalAnalyzer(Config cfg) : fCfg(std::move(cfg)) {}

void RelvalAnalyzer::run(const std::vector<std::string> &inputFiles) const {
  if (inputFiles.empty()) {
    throw std::runtime_error("No input ROOT files provided.");
  }

  std::cout << "[runFillHistograms] Tree:    " << fCfg.treeName << "\n";
  std::cout << "[runFillHistograms] Mode:    "
            << (fCfg.useDefaultTrirecBooking ? "default trirec multi-hist" : "single-hist")
            << "\n";
  std::cout << "[runFillHistograms] Outputs: " << fCfg.outputFile << "\n";
  std::cout << "[runFillHistograms] Inputs:  " << inputFiles.size() << " file(s)\n";

  ROOT::RDataFrame df(fCfg.treeName, inputFiles);

  HistBooking booking(
      fCfg.useDefaultTrirecBooking
          ? HistBooking::defaultTrirecDefs()
          : std::vector<HistBooking::HistDef>{
                {fCfg.branchName, fCfg.histName, fCfg.branchName, fCfg.bins, fCfg.xmin, fCfg.xmax}});
                
  auto handles = booking.book(df);
  if (handles.empty()) {
    throw std::runtime_error(
        "No requested branches found in tree; no histograms were booked.");
  }

  TFile outFile(fCfg.outputFile.c_str(), "RECREATE");
  if (outFile.IsZombie()) {
    throw std::runtime_error("Could not create output ROOT file: " + fCfg.outputFile);
  }

  for (auto &h : handles) {
    h->Write();
  }
  outFile.Close();
}
