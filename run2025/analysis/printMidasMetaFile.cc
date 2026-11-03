#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <math.h>
#include <string>

#include <TFile.h>
#include <TTree.h>
#include <iterator>
#include <map>
#include <set>
#include <vector>
#include <sstream>
#include <nlohmann/json.hpp>    // for nlohmann::ordered_json

#include "util.hh"

using namespace std;


// ----------------------------------------------------------------------
int main(int argc, char *argv[]) {
  if (argc < 2) {
    cout << "Usage: " << argv[0] << " file.json" << endl;
    return 0; 
  }

  std::string path = argv[1];
  std::cout << "Processing file: " << path << std::endl;
  
  std::ifstream in(path);
  if (!in) throw std::runtime_error("Cannot open " + path);
  nlohmann::json j;
  in >> j;

  // The JSON structure has a "febs" array, not a direct array
  if (!j.contains("febs") || !j["febs"].is_array()) {
    cout << "No febs found in the JSON file" << endl;
    return 0;
  }
  
  const auto& febs = j["febs"];
  const int LVDS_ERROR_RATE_THRESHOLD = 10;
  int nAsics(0);
  int nMaskedLinks(0), nStatus4Links(0);
  for (size_t fedIdx = 0; fedIdx < febs.size(); ++fedIdx) {
    const auto& section = febs[fedIdx];
    if (!section.contains("asics") || !section["asics"].is_array()) continue;
    // Only keep sections whose name matches "L1 *" or "L2 *"
    if (section.contains("name") && section["name"].is_string()) {
      const std::string secName = section["name"].get<std::string>();
      const bool isL1 = secName.rfind("L1 ", 0) == 0; // starts with
      const bool isL2 = secName.rfind("L2 ", 0) == 0; // starts with
      if (!(isL1 || isL2)) continue;
    } else {
      // If no name, skip
      continue;
    }

    vector<int> lvdsErrRates{0,0,0};
    string linkMask = "";
    for (size_t idx = 0; idx < section["asics"].size(); ++idx) {
      const auto& a = section["asics"][idx];
     
      linkMask = a.value("linkMask", "");
      if (a.contains("lvdsErrRates") && a["lvdsErrRates"].is_array()) {
        const auto& rates = a["lvdsErrRates"];
        lvdsErrRates[0] = (rates.size() > 0 ? rates[0].get<int>() : 0);
        lvdsErrRates[1] = (rates.size() > 1 ? rates[1].get<int>() : 0);
        lvdsErrRates[2] = (rates.size() > 2 ? rates[2].get<int>() : 0);
      }
      if (lvdsErrRates[0] > LVDS_ERROR_RATE_THRESHOLD || lvdsErrRates[0] < 0) nStatus4Links++;
      if (lvdsErrRates[1] > LVDS_ERROR_RATE_THRESHOLD || lvdsErrRates[1] < 0) nStatus4Links++;
      if (lvdsErrRates[2] > LVDS_ERROR_RATE_THRESHOLD || lvdsErrRates[2] < 0) nStatus4Links++;

      if (linkMask[0] == '0') nMaskedLinks++;
      if (linkMask[1] == '0') nMaskedLinks++;
      if (linkMask[2] == '0') nMaskedLinks++;

      cout << "chipid: " << a.value("globalId", 0) << " linkMask: " << linkMask << " lvdsErrRates: " << lvdsErrRates[0] << "," << lvdsErrRates[1] << "," << lvdsErrRates[2] << endl;
      nAsics++;
    }
  }
  cout << "nMaskedLinks: " << nMaskedLinks 
       << " nStatus4Links: " << nStatus4Links 
       << endl;
  return 0;
}