#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <math.h>
#include <string>


#include <iterator>
#include <map>
#include <set>
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
  // Map globalId -> {links, pattern}
  std::map<int, std::pair<std::vector<std::string>, std::string>> byGlobal;

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
    for (int off : orderedOffsets) {
      links.push_back(fmtLink(a.fedID, base + off));
    }
    byGlobal[a.globalId] = std::make_pair(std::move(links), originalPattern);
  }

  std::cout << "  map<int, vector<string>> mLinksChipID = {" << std::endl;
  for (auto it = byGlobal.begin(); it != byGlobal.end(); ++it) {
    const int gid = it->first;
    const auto& links = it->second.first;
    const auto& pattern = it->second.second;
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
}




// ----------------------------------------------------------------------
std::string inferMissingLetters(const std::string& pattern, const std::set<std::string>& knownPatterns) {
  std::string result = pattern;
  int eCount = 0; for (char ch : result) if (ch == 'E') ++eCount;
  
  if (eCount == 0) return result;
  
  // Try to find a complete pattern (no E's) for this globalId
  for (const auto& known : knownPatterns) {
    if (known.find('E') == std::string::npos) {
      // Use this complete pattern as reference
      for (size_t i = 0; i < std::min(result.length(), known.length()); ++i) {
        if (result[i] == 'E' && known[i] != 'E') {
          result[i] = known[i];
        }
      }
      break;
    }
  }
  
  // If still has E's, fill remaining sequentially
  bool used[3] = {false, false, false};
  for (char ch : result) {
    if (ch == 'A') used[0] = true;
    else if (ch == 'B') used[1] = true;
    else if (ch == 'C') used[2] = true;
  }
  
  for (char &ch : result) {
    if (ch == 'E') {
      for (int k = 0; k < 3; ++k) {
        if (!used[k]) {
          ch = 'A' + k;
          used[k] = true;
          break;
        }
      }
    }
  }
  
  return result;
}

// ----------------------------------------------------------------------
void printMasksForFile(const std::vector<AsicInfo>& asics, const std::string& filename) {
  std::map<int, std::vector<int>> byGlobal;
  
  for (const auto& a : asics) {
    const int base = a.idxInSection * 3;
    
    // Use original pattern for mask ordering (before inference)
    std::vector<int> orderedOffsets; orderedOffsets.reserve(3);
    bool used[3] = {false, false, false};
    
    for (char ch : a.linkMatrix) {
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
    
    std::vector<int> masks; masks.reserve(3);
    for (int off : orderedOffsets) {
      if (off < (int)a.linkMask.length()) {
        masks.push_back(a.linkMask[off] - '0');
      } else {
        masks.push_back(0);
      }
    }
    byGlobal[a.globalId] = std::move(masks);
  }
  
  // Count masked (0) links
  int maskedCount = 0;
  for (const auto& pair : byGlobal) {
    for (int mask : pair.second) {
      if (mask == 0) maskedCount++;
    }
  }
  
  std::cout << "  // " << filename << " (" << maskedCount << " masked links)" << std::endl;
  std::cout << "  map<int, vector<int>> mLinkMasksChipID_" << filename.substr(filename.find_last_of("/") + 1) << " = {" << std::endl;
  for (auto it = byGlobal.begin(); it != byGlobal.end(); ++it) {
    const int gid = it->first;
    const auto& masks = it->second;
    std::cout << "    {" << gid << ",    {"
              << masks[0] << ", " << masks[1] << ", " << masks[2] << "}}";
    if (std::next(it) != byGlobal.end()) std::cout << ",";
    std::cout << std::endl;
  }
  std::cout << "  };" << std::endl;
}

// ----------------------------------------------------------------------
int main(int argc, char *argv[]) {
  try {
    if (argc < 2) {
      std::cerr << "Usage: " << argv[0] << " <file1.json> [file2.json] [file3.json] ..." << std::endl;
      return 2;
    }
    
    // Collect all ASICs from all files
    std::vector<AsicInfo> allAsics;
    std::map<int, std::set<std::string>> patternsPerGlobalId;
    std::vector<std::pair<std::string, std::vector<AsicInfo>>> fileAsics;
    
    for (int i = 1; i < argc; ++i) {
      std::string path = argv[i];
      std::cout << "Processing file: " << path << std::endl;
      auto asics = parseJSONFile(path);
      fileAsics.push_back({path, asics});
      
      for (const auto& asic : asics) {
        allAsics.push_back(asic);
        patternsPerGlobalId[asic.globalId].insert(asic.linkMatrix);
      }
    }
    
    // Infer missing letters for unified links
    for (auto& asic : allAsics) {
      if (asic.linkMatrix.find('E') != std::string::npos) {
        std::string inferred = inferMissingLetters(asic.linkMatrix, patternsPerGlobalId[asic.globalId]);
        asic.linkMatrix = inferred;
      }
    }
    
    // Print unified links
    std::cout << "  // Unified links (resolved patterns)" << std::endl;
    printByGlobalId(allAsics);
    
    // Print masks for each file separately
    for (const auto& filePair : fileAsics) {
      printMasksForFile(filePair.second, filePair.first);
    }
    
    return 0;
  } catch (const std::exception &ex) {
    std::cerr << "Error: " << ex.what() << std::endl;
    return 1;
  }
}
