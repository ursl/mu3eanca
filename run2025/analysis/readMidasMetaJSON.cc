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
  int ckdivend{};
  int ckdivend2{};
  int vdacBLPix{};
  int vdacThHigh{};
  int vdacThLow{};
  int biasVNOutPix{};
  int biasVPDAC{};
  int biasVNDcl{};
  int biasVNLVDS{};
  int biasVNLVDSDel{};
  int biasVPDcl{};
  int biasVPTimerDel{};
  int vdacBaseline{};
};

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
static void printByGlobalId(const std::vector<AsicInfo>& asics) {
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
void writeJsonSummaryPerFile(const std::vector<AsicInfo>& asics, const std::string& filename) {
  // Build byGlobal data with links, masks, and lvdsErrRates
  std::map<int, nlohmann::ordered_json> byGlobal;
  for (const auto& a : asics) {
    // Links from possibly inferred pattern present in a.linkMatrix
    auto links = buildLinksForPattern(a, a.linkMatrix);

    // Masks aligned to pattern order
    std::vector<int> masks; masks.reserve(3);
    for (int i = 0; i < 3; ++i) {
      char ch = (i < (int)a.linkMatrix.size()) ? a.linkMatrix[i] : 'E';
      int off = letterToOffset(ch);
      if (off >= 0 && off < (int)a.linkMask.size()) masks.push_back(a.linkMask[off] - '0');
      else masks.push_back(0);
    }

    // Build entry with globalChipID first to ensure it appears first
    nlohmann::ordered_json jentry;
    jentry["globalChipID"] = a.globalId;
    jentry["links"] = links;
    jentry["masks"] = masks;
    jentry["lvdsErrRates"] = { a.lvdsErrRate0, a.lvdsErrRate1, a.lvdsErrRate2 };
    jentry["conf"] = {
      {"ckdivend",  a.ckdivend},
      {"ckdivend2", a.ckdivend2},
      {"BLPix",     a.vdacBLPix},
      {"ThHigh",    a.vdacThHigh},
      {"ThLow",     a.vdacThLow},
      {"VNOutPix",  a.biasVNOutPix},
      {"VPDAC",     a.biasVPDAC},
      {"VNDcl",     a.biasVNDcl},
      {"VNLVDS",    a.biasVNLVDS},
      {"VNLVDSDel", a.biasVNLVDSDel},
      {"VPDcl",     a.biasVPDcl},
      {"VPTimerDel",a.biasVPTimerDel},
      {"Baseline",  a.vdacBaseline}
    };
    byGlobal[a.globalId] = std::move(jentry);
  }

  // Compose final JSON
  nlohmann::ordered_json jout;
  // Determine and insert run number FIRST so it appears at top
  std::string runnum = extractRunNumber(filename);
  bool allDigits = !runnum.empty();
  for (char c : runnum) { if (c < '0' || c > '9') { allDigits = false; break; } }
  if (allDigits && !runnum.empty()) {
    try { jout["runNumber"] = std::stoi(runnum); }
    catch (...) { jout["runNumber"] = runnum; }
  } else {
    jout["runNumber"] = runnum;
  }
  // Emit chips array after runNumber so runNumber is top
  nlohmann::ordered_json arr = nlohmann::ordered_json::array();
  for (const auto &kv : byGlobal) {
    arr.push_back(kv.second); // already has globalChipID as first key
  }
  jout["chips"] = std::move(arr);

  // Determine output filename
  std::string outname = std::string("./midasMeta-summary-run") + runnum + std::string(".json");
  std::ofstream ofs(outname);
  if (!ofs) {
    std::cerr << "Error: cannot write to " << outname << std::endl;
    return;
  }
  ofs << std::setw(2) << jout << std::endl;
  std::cout << "Wrote summary: " << outname << std::endl;
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
    printByGlobalId(allAsics);
    
    // Print masks for each file separately
    for (const auto& filePair : fileAsics) {
      printMasksForFile(filePair.second, filePair.first);
    writeJsonSummaryPerFile(filePair.second, filePair.first);
    }
    
    return 0;
  } catch (const std::exception &ex) {
    std::cerr << "Error: " << ex.what() << std::endl;
    return 1;
  }
}
