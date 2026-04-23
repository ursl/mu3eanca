#ifndef RELVAL_ANALYZER_HH
#define RELVAL_ANALYZER_HH

#include <string>
#include <vector>

class RelvalAnalyzer {
public:
  struct Config {
    std::string treeName = "frames";
    bool useDefaultTrirecBooking = true;
    std::string branchName = "runId";
    std::string histName = "h_branch";
    int bins = 200;
    double xmin = 0.;
    double xmax = 10000.;
    std::string outputFile = "ana.root";
  };

  explicit RelvalAnalyzer(Config cfg);
  void run(const std::vector<std::string> &inputFiles) const;

private:
  Config fCfg;
};

#endif
