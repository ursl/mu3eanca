#ifndef HIST_BOOKING_HH
#define HIST_BOOKING_HH

#include <ROOT/RDataFrame.hxx>
#include <TH1D.h>

#include <string>
#include <vector>

class HistBooking {
public:
  struct HistDef {
    std::string branch;
    std::string histName;
    std::string title;
    int bins = 100;
    double xmin = 0.;
    double xmax = 1.;
  };

  using HistHandle = ROOT::RDF::RResultPtr<TH1D>;

  explicit HistBooking(std::vector<HistDef> defs);

  std::vector<HistHandle> book(ROOT::RDF::RNode df) const;
  const std::vector<HistDef> &definitions() const { return fDefs; }

  static std::vector<HistDef> defaultTrirecDefs();

private:
  std::vector<HistDef> fDefs;
};

#endif
