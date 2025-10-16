#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <math.h>
#include <string>


#include <iterator>
#include <map>
#include <vector>
#include "../../common/json.h"  // nlohmann::json

#include "util.hh"

using namespace std;

// ----------------------------------------------------------------------
struct AsicInfo {
  int confId{};
  int globalId{};
  int fedID{};
  int idxInSection{}; // position within the section (0..)
  std::string linkMask;
  std::string linkMatrix;
  long long lvdsErrRate0{};
  long long lvdsErrRate1{};
  long long lvdsErrRate2{};
};

// ----------------------------------------------------------------------
std::vector<AsicInfo> parseJSONFile(const std::string& path) {
  std::ifstream in(path);
  if (!in) throw std::runtime_error("Cannot open " + path);
  nlohmann::json j;
  in >> j;

  std::vector<AsicInfo> out;
  if (!j.is_array()) return out;

  for (size_t fedIdx = 0; fedIdx < j.size(); ++fedIdx) {
    const auto& section = j[fedIdx];
    if (!section.contains("asics") || !section["asics"].is_array()) continue;

    for (size_t idx = 0; idx < section["asics"].size(); ++idx) {
      const auto& a = section["asics"][idx];
      AsicInfo ai;
      ai.fedID       = static_cast<int>(fedIdx);  // 0..N-1
      ai.idxInSection= static_cast<int>(idx);
      ai.confId      = a.value("confId", 0);
      ai.globalId    = a.value("globalId", 0);
      ai.linkMask    = a.value("linkMask", "");
      ai.linkMatrix  = a.value("linkMatrix", "");
      ai.lvdsErrRate0= a.value("lvdsErrRate0", 0LL);
      ai.lvdsErrRate1= a.value("lvdsErrRate1", 0LL);
      ai.lvdsErrRate2= a.value("lvdsErrRate2", 0LL);
      out.push_back(std::move(ai));
    }
  }
  return out;
}

// ----------------------------------------------------------------------
int letterToOffset(char c) {
  // Map bit-pattern letters to vector offsets: C->0, B->1, A->2
  if (c == 'C') return 0;
  if (c == 'B') return 1;
  if (c == 'A') return 2;
  return -1; // 'E' or other
}

// ----------------------------------------------------------------------
static std::string fmtLink(int fedIdx, int linkIdx) {
  std::ostringstream os;
  os << "lvds/FEB_" << std::setw(2) << std::setfill('0') << fedIdx
     << "/Link_" << std::setw(2) << std::setfill('0') << linkIdx;
  return os.str();
}

// ----------------------------------------------------------------------
static void printByGlobalId(const std::vector<AsicInfo>& asics) {
  // Map globalId -> {links, masks, pattern}
  std::map<int, std::tuple<std::vector<std::string>, std::vector<int>, std::string>> byGlobal;

  for (const auto& a : asics) {
    // Reset enumeration per FED: base is per-ASIC within its FED
    const int base = a.idxInSection * 3;

    std::vector<int> orderedOffsets; orderedOffsets.reserve(3);
    bool used[3] = {false, false, false};

    const std::string originalPattern = a.linkMatrix;
    std::string pattern = originalPattern;
    // If exactly one 'E', infer the missing letter among {A,B,C}
    int eCount = 0; for (char ch : pattern) if (ch == 'E') ++eCount;
    if (eCount == 1) {
      bool hasA = pattern.find('A') != std::string::npos;
      bool hasB = pattern.find('B') != std::string::npos;
      bool hasC = pattern.find('C') != std::string::npos;
      char missing = 'A';
      if (!hasA && hasB && hasC) missing = 'A';
      else if (!hasB && hasA && hasC) missing = 'B';
      else if (!hasC && hasA && hasB) missing = 'C';
      // replace the sole 'E' with the inferred missing letter
      for (char &ch : pattern) { if (ch == 'E') { ch = missing; break; } }
    }

    // Build offsets from possibly adjusted pattern; if any 'E' remains (0,2,3 Es), fill sequentially
    for (char ch : pattern) {
      int off = letterToOffset(ch);
      if (off >= 0) {
        orderedOffsets.push_back(off);
        used[off] = true;
      } else {
        for (int k = 0; k < 3; ++k) {
          if (!used[k]) { orderedOffsets.push_back(k); used[k] = true; break; }
        }
      }
    }
    for (int k = 0; (int)orderedOffsets.size() < 3 && k < 3; ++k) {
      if (!used[k]) { orderedOffsets.push_back(k); used[k] = true; }
    }

    std::vector<std::string> links; links.reserve(3);
    std::vector<int> masks; masks.reserve(3);
    for (int off : orderedOffsets) {
      links.push_back(fmtLink(a.fedID, base + off));
      // Extract mask bit for this link position (0 or 1)
      if (off < (int)a.linkMask.length()) {
        masks.push_back(a.linkMask[off] - '0');
      } else {
        masks.push_back(0); // default if mask too short
      }
    }
    byGlobal[a.globalId] = std::make_tuple(std::move(links), std::move(masks), originalPattern);
  }

  std::cout << "  map<int, vector<string>> mLinksChipID = {" << std::endl;
  for (auto it = byGlobal.begin(); it != byGlobal.end(); ++it) {
    const int gid = it->first;
    const auto& links = std::get<0>(it->second);
    const auto& pattern = std::get<2>(it->second);
    std::cout << "    {" << gid << ",    {"
              << "\"" << links[0] << "\"" << ", "
              << "\"" << links[1] << "\"" << ", "
              << "\"" << links[2] << "\"" << "}}";
    if (std::next(it) != byGlobal.end()) std::cout << ",";
    if (pattern.find('E') != std::string::npos) {
      std::cout << "  // " << pattern;
    }
    std::cout << std::endl;
  }
  std::cout << "  };" << std::endl;

  std::cout << "  map<int, vector<int>> mLinkMasksChipID = {" << std::endl;
  for (auto it = byGlobal.begin(); it != byGlobal.end(); ++it) {
    const int gid = it->first;
    const auto& masks = std::get<1>(it->second);
    const auto& pattern = std::get<2>(it->second);
    std::cout << "    {" << gid << ",    {"
              << masks[0] << ", " << masks[1] << ", " << masks[2] << "}}";
    if (std::next(it) != byGlobal.end()) std::cout << ",";
    if (pattern.find('E') != std::string::npos) {
      std::cout << "  // " << pattern;
    }
    std::cout << std::endl;
  }
  std::cout << "  };" << std::endl;
}




// ----------------------------------------------------------------------
int main(int argc, char *argv[]) {
  try {
    if (argc < 2) {
      std::cerr << "Usage: " << argv[0] << " <path/to/file.json>" << std::endl;
      return 2;
    }
    std::string path = argv[1];
    auto asics = parseJSONFile(path);
    printByGlobalId(asics);
    return 0;
  } catch (const std::exception &ex) {
    std::cerr << "Error: " << ex.what() << std::endl;
    return 1;
  }
}
