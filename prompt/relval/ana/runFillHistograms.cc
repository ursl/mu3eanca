#include "RelvalAnalyzer.hh"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void printUsage(const char *argv0) {
  std::cout
      << "Usage:\n"
      << "  " << argv0
      << " [options] <input1.root> [input2.root ...]\n\n"
      << "Options:\n"
      << "  --tree <name>       TTree name (default: frames)\n"
      << "  --single            Disable default trirec multi-hist booking\n"
      << "  --branch <name>     Branch to histogram (default: runId)\n"
      << "  --bins <N>          Number of bins (default: 200)\n"
      << "  --xmin <x>          Histogram xmin (default: 0)\n"
      << "  --xmax <x>          Histogram xmax (default: 10000)\n"
      << "  --hist <name>       Histogram object name (default: h_branch)\n"
      << "  --out <file.root>   Output ROOT file (default: histograms.root)\n";
}

} // namespace

int main(int argc, char **argv) {
  RelvalAnalyzer::Config cfg;
  std::vector<std::string> inputs;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    auto takeValue = [&](const char *opt) -> std::string {
      if (i + 1 >= argc) {
        throw std::runtime_error(std::string("Missing value for option ") + opt);
      }
      return argv[++i];
    };

    if (arg == "--tree") {
      cfg.treeName = takeValue("--tree");
    } else if (arg == "--single") {
      cfg.useDefaultTrirecBooking = false;
    } else if (arg == "--branch") {
      cfg.branchName = takeValue("--branch");
    } else if (arg == "--bins") {
      cfg.bins = std::stoi(takeValue("--bins"));
    } else if (arg == "--xmin") {
      cfg.xmin = std::stod(takeValue("--xmin"));
    } else if (arg == "--xmax") {
      cfg.xmax = std::stod(takeValue("--xmax"));
    } else if (arg == "--hist") {
      cfg.histName = takeValue("--hist");
    } else if (arg == "--out") {
      cfg.outputFile = takeValue("--out");
    } else if (arg == "-h" || arg == "--help") {
      printUsage(argv[0]);
      return 0;
    } else if (!arg.empty() && arg[0] == '-') {
      throw std::runtime_error("Unknown option: " + arg);
    } else {
      inputs.emplace_back(arg);
    }
  }

  if (inputs.empty()) {
    printUsage(argv[0]);
    return 1;
  }

  try {
    RelvalAnalyzer analyzer(cfg);
    analyzer.run(inputs);
  } catch (const std::exception &e) {
    std::cerr << "[runFillHistograms] ERROR: " << e.what() << "\n";
    return 2;
  }

  return 0;
}
