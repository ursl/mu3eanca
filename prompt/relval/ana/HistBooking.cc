#include "HistBooking.hh"

#include <algorithm>
#include <iostream>
#include <unordered_set>

HistBooking::HistBooking(std::vector<HistDef> defs) : fDefs(std::move(defs)) {}

std::vector<HistBooking::HistHandle> HistBooking::book(ROOT::RDF::RNode df) const {
  std::vector<HistHandle> handles;
  const auto cols = df.GetColumnNames();
  const std::unordered_set<std::string> colset(cols.begin(), cols.end());

  for (const auto &def : fDefs) {
    if (colset.find(def.branch) == colset.end()) {
      std::cout << "[runFillHistograms] Skip missing branch: " << def.branch << "\n";
      continue;
    }
    ROOT::RDF::TH1DModel model(def.histName.c_str(), def.title.c_str(), def.bins, def.xmin,
                               def.xmax);
    handles.emplace_back(df.Histo1D(model, def.branch));
  }

  return handles;
}

std::vector<HistBooking::HistDef> HistBooking::defaultTrirecDefs() {
  return {
      {"runId", "h_runId", "runId", 200, 0., 10000.},
      {"frameId", "h_frameId", "frameId", 200, 0., 1.0e9},
      {"flags", "h_flags", "flags", 64, 0., 64.},
      {"n", "h_n", "n", 200, 0., 200.},
      {"n3", "h_n3", "n3", 200, 0., 200.},
      {"n4", "h_n4", "n4", 200, 0., 200.},
      {"n6", "h_n6", "n6", 200, 0., 200.},
      {"n8", "h_n8", "n8", 200, 0., 200.},
      {"mc_eventId", "h_mc_eventId", "mc_eventId", 200, 0., 1.0e7},
      {"mc_eventWeight", "h_mc_eventWeight", "mc_eventWeight", 200, 0., 2.0},
  };
}
