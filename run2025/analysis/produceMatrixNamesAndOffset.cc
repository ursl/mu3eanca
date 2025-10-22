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
#include <sstream>
#include "../../common/json.h"  // nlohmann::json
#include <nlohmann/json.hpp>    // for nlohmann::ordered_json

#include "util.hh"
#include "asicInfo.hh"

using namespace std;

// ----------------------------------------------------------------------
// -- the primary purpose of this is to dump the linkMatrix by reading multiple midas meta JSON files
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
std::vector<AsicInfo> parseJSONFile(const std::string& path) {
  std::ifstream in(path);
  if (!in) throw std::runtime_error("Cannot open " + path);
  nlohmann::json j;
  in >> j;

  std::vector<AsicInfo> out;
  // The JSON structure has a "febs" array, not a direct array
  if (!j.contains("febs") || !j["febs"].is_array()) return out;
  
  const auto& febs = j["febs"];
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

    for (size_t idx = 0; idx < section["asics"].size(); ++idx) {
      const auto& a = section["asics"][idx];
      AsicInfo ai;
      // FED index is explicitly provided in the JSON as "index"
      ai.fedID       = section.value("index", static_cast<int>(fedIdx));
      ai.idxInSection= static_cast<int>(idx);
      ai.confId      = a.value("confId", 0);
      ai.globalId    = a.value("globalId", 0);
      ai.linkMask    = a.value("linkMask", "");
      ai.linkMatrix  = a.value("linkMatrix", "");
      // lvdsErrRates moved into an array [r0, r1, r2]
      if (a.contains("lvdsErrRates") && a["lvdsErrRates"].is_array()) {
        const auto &arr = a["lvdsErrRates"];
        ai.lvdsErrRate0 = (arr.size() > 0 && arr[0].is_number()) ? arr[0].get<long long>() : 0LL;
        ai.lvdsErrRate1 = (arr.size() > 1 && arr[1].is_number()) ? arr[1].get<long long>() : 0LL;
        ai.lvdsErrRate2 = (arr.size() > 2 && arr[2].is_number()) ? arr[2].get<long long>() : 0LL;
      } else {
        ai.lvdsErrRate0 = ai.lvdsErrRate1 = ai.lvdsErrRate2 = 0LL;
      }

      // Extract required configuration values from dacs.conf and selected DACs
      ai.ckdivend  = 0;
      ai.ckdivend2 = 0;
      ai.vdacBLPix = 0;
      ai.vdacThHigh = 0;
      ai.vdacThLow = 0;
      ai.biasVNOutPix = 0;
      ai.biasVPDAC = 0;
      ai.biasVNDcl = 0;
      ai.biasVNLVDS = 0;
      ai.biasVNLVDSDel = 0;
      ai.biasVPDcl = 0;
      ai.biasVPTimerDel = 0;
      ai.vdacBaseline = 0;
      if (a.contains("dacs") && a["dacs"].is_object()) {
        const auto &dacs = a["dacs"];
        if (dacs.contains("conf") && dacs["conf"].is_object()) {
          const auto &conf = dacs["conf"];
          ai.ckdivend  = conf.value("ckdivend", 0);
          ai.ckdivend2 = conf.value("ckdivend2", 0);
        }
        if (dacs.contains("vdac") && dacs["vdac"].is_object()) {
          const auto &vdac = dacs["vdac"];
          ai.vdacBLPix  = vdac.value("BLPix", 0);
          ai.vdacThHigh = vdac.value("ThHigh", 0);
          ai.vdacThLow  = vdac.value("ThLow", 0);
          ai.vdacBaseline = vdac.value("Baseline", 0);
        }
        if (dacs.contains("bias") && dacs["bias"].is_object()) {
          const auto &bias = dacs["bias"];
          ai.biasVNOutPix = bias.value("VNOutPix", 0);
          ai.biasVPDAC    = bias.value("VPDAC", 0);
          ai.biasVNDcl    = bias.value("VNDcl", 0);
          ai.biasVNLVDS   = bias.value("VNLVDS", 0);
          ai.biasVNLVDSDel= bias.value("VNLVDSDel", 0);
          ai.biasVPDcl    = bias.value("VPDcl", 0);
          ai.biasVPTimerDel= bias.value("VPTimerDel", 0);
        }
      }
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
static std::string extractRunNumber(const std::string &path) {
  // Try to find "run" followed by digits in the basename
  std::string base = path;
  auto pos = base.find_last_of("/");
  if (pos != std::string::npos) base = base.substr(pos + 1);
  // Search for "run" and collect following digits
  pos = base.find("run");
  if (pos == std::string::npos) return base; // fallback to basename
  std::string digits;
  for (size_t i = pos + 3; i < base.size(); ++i) {
    if (base[i] >= '0' && base[i] <= '9') digits.push_back(base[i]);
    else break;
  }
  if (digits.empty()) return base;
  return digits;
}

// ----------------------------------------------------------------------
static std::vector<std::string> buildLinksForPattern(const AsicInfo &a, const std::string &pattern) {
  const int base = a.idxInSection * 3;
  std::vector<std::string> links; links.reserve(3);
  for (int i = 0; i < 3; ++i) {
    char ch = (i < (int)pattern.size()) ? pattern[i] : 'E';
    int off = letterToOffset(ch);
    if (off >= 0) {
      links.push_back(fmtLink(a.fedID, base + off));
    } else {
      std::ostringstream os;
      os << "lvds/FEB_" << std::setw(2) << std::setfill('0') << a.fedID
         << "/Link_" << std::setw(2) << std::setfill('0') << 99;
      links.push_back(os.str());
    }
  }
  return links;
}

// ----------------------------------------------------------------------
static void printByGlobalId(const std::vector<AsicInfo>& asics, const std::string& filename) {
  // Choose, for each globalId, the entry with the fewest remaining 'E' in linkMatrix
  auto countEs = [](const std::string &s) {
    int c = 0; for (char ch : s) if (ch == 'E') ++c; return c;
  };

  struct BestRec { AsicInfo asic; int eCount; };
  std::map<int, BestRec> bestByGlobal;

  for (const auto &a : asics) {
    int eC = countEs(a.linkMatrix);
    auto it = bestByGlobal.find(a.globalId);
    if (it == bestByGlobal.end() || eC < it->second.eCount) {
      bestByGlobal[a.globalId] = BestRec{a, eC};
    }
  }

  // Map globalId -> {links, pattern} for printing
  std::map<int, std::pair<std::vector<std::string>, std::string>> byGlobal;
  for (const auto &kv : bestByGlobal) {
    const AsicInfo &a = kv.second.asic;
    const int base = a.idxInSection * 3;
    const std::string &pattern = a.linkMatrix;

    std::vector<std::string> links; links.reserve(3);
    for (int i = 0; i < 3; ++i) {
      char ch = (i < (int)pattern.size()) ? pattern[i] : 'E';
      int off = letterToOffset(ch);
      if (off >= 0) {
        links.push_back(fmtLink(a.fedID, base + off));
      } else {
        std::ostringstream os;
        os << "lvds/FEB_" << std::setw(2) << std::setfill('0') << a.fedID
           << "/Link_" << std::setw(2) << std::setfill('0') << 99;
        links.push_back(os.str());
      }
    }
    byGlobal[a.globalId] = std::make_pair(std::move(links), pattern);
  }

  ofstream ofs(filename + ".icc");
  ofs << "  map<int, vector<string>> mChipIDLinkName = {" << endl;
  for (auto it = byGlobal.begin(); it != byGlobal.end(); ++it) {
    const int gid = it->first;
    const auto& links = it->second.first;
    const auto& pattern = it->second.second;
    ofs << "    {" << gid << ",    {"
              << "\"" << links[0] << "\"" << ", "
              << "\"" << links[1] << "\"" << ", "
              << "\"" << links[2] << "\"" << "}}";
    if (std::next(it) != byGlobal.end()) ofs << ",";
    if (pattern.find('E') != std::string::npos) {
      ofs << "  // " << pattern;
    }
    ofs << endl;
  }
  ofs << "  };" << endl;
  ofs.close();
}

// ----------------------------------------------------------------------
static void printLinkOffsetsByGlobalId(const std::vector<AsicInfo>& asics, const std::string& filename) {
  // Choose, for each globalId, the entry with the fewest remaining 'E' in linkMatrix
  auto countEs = [](const std::string &s) {
    int c = 0; for (char ch : s) if (ch == 'E') ++c; return c;
  };

  struct BestRec { AsicInfo asic; int eCount; };
  std::map<int, BestRec> bestByGlobal;

  for (const auto &a : asics) {
    int eC = countEs(a.linkMatrix);
    auto it = bestByGlobal.find(a.globalId);
    if (it == bestByGlobal.end() || eC < it->second.eCount) {
      bestByGlobal[a.globalId] = BestRec{a, eC};
    }
  }

  // Map globalId -> link offsets for printing
  std::map<int, std::vector<int>> byGlobal;
  for (const auto &kv : bestByGlobal) {
    const AsicInfo &a = kv.second.asic;
    const int base = a.idxInSection * 3;
    const std::string &pattern = a.linkMatrix;

    std::vector<int> linkNumbers; linkNumbers.reserve(3);
    const int UNRESOLVED_PLACEHOLDER = 999; // Use 999 as placeholder for unresolved links
    
    for (int i = 0; i < 3; ++i) {
      char ch = (i < (int)pattern.size()) ? pattern[i] : 'E';
      int off = letterToOffset(ch);
      if (off >= 0) {
        linkNumbers.push_back(base + off);
      } else {
        linkNumbers.push_back(UNRESOLVED_PLACEHOLDER); // placeholder for unresolved
      }
    }
    
    // Calculate offsets relative to the minimum link number (excluding placeholders)
    if (!linkNumbers.empty()) {
      // Find minimum among non-placeholder values
      int minLink = 9999; // large number as default
      for (int linkNum : linkNumbers) {
        if (linkNum != UNRESOLVED_PLACEHOLDER && linkNum < minLink) {
          minLink = linkNum;
        }
      }
      
      std::vector<int> offsets;
      for (int linkNum : linkNumbers) {
        if (linkNum == UNRESOLVED_PLACEHOLDER) {
          offsets.push_back(9); // output 9 for unresolved
        } else {
          offsets.push_back(linkNum - minLink);
        }
      }
      byGlobal[a.globalId] = std::move(offsets);
    }
  }

  ofstream ofs(filename + ".icc");
  ofs << "  map<int, vector<int>> mChipIDLinkOffsets = {" << endl;
  for (auto it = byGlobal.begin(); it != byGlobal.end(); ++it) {
    const int gid = it->first;
    const auto& offsets = it->second;
    ofs << "    {" << gid << ",    {"
              << offsets[0] << ", " << offsets[1] << ", " << offsets[2] << "}}";
    if (std::next(it) != byGlobal.end()) ofs << ",";
    ofs << endl;
  }
  ofs << "  };" << endl;
  ofs.close();
}




// ----------------------------------------------------------------------
std::string inferMissingLetters(const std::string& pattern, const std::set<std::string>& knownPatterns) {
  std::string result = pattern;
  int eCount = 0; for (char ch : result) if (ch == 'E') ++eCount;
  
  if (eCount == 0) return result;

  // Local inference for exactly one 'E': deduce missing among A,B,C
  if (eCount == 1) {
    bool hasA = result.find('A') != std::string::npos;
    bool hasB = result.find('B') != std::string::npos;
    bool hasC = result.find('C') != std::string::npos;
    char missing = 'A';
    if (!hasA && hasB && hasC) missing = 'A';
    else if (!hasB && hasA && hasC) missing = 'B';
    else if (!hasC && hasA && hasB) missing = 'C';
    for (char &ch : result) { if (ch == 'E') { ch = missing; break; } }
    return result;
  }
  
  // Only fill positions where we have a complete reference (no E's)
  for (const auto& known : knownPatterns) {
    if (known.find('E') == std::string::npos) {
      // Use this complete pattern as reference: copy only non-E letters into E slots
      for (size_t i = 0; i < std::min(result.length(), known.length()); ++i) {
        if (result[i] == 'E' && known[i] != 'E') {
          result[i] = known[i];
        }
      }
      break;
    }
  }
  return result;
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
    
    // Always attempt inference:
    // - If exactly one 'E' in a pattern, infer locally
    // - Otherwise, only fill from complete patterns seen across files
    for (auto& asic : allAsics) {
      if (asic.linkMatrix.find('E') != std::string::npos) {
        std::string inferred = inferMissingLetters(asic.linkMatrix, patternsPerGlobalId[asic.globalId]);
        asic.linkMatrix = inferred;
      }
    }
    
    // Print unified links
    std::cout << "  // Unified links (resolved patterns)" << std::endl;
    printByGlobalId(allAsics, "mChipIDLinkNames");
    
    // Print link offsets
    std::cout << "  // Link offsets (relative to minimum link number)" << std::endl;
    printLinkOffsetsByGlobalId(allAsics, "mChipIDLinkOffsets");
    
    return 0;
  } catch (const std::exception &ex) {
    std::cerr << "Error: " << ex.what() << std::endl;
    return 1;
  }
}
