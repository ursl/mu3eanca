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
void fixSingleE(AsicInfo &ai) {
  int eCount = std::count(ai.linkMatrix.begin(), ai.linkMatrix.end(), 'E');
  if (eCount == 1) {
    char missing = 'A';
    bool hasA = ai.linkMatrix.find('A') != std::string::npos;
    bool hasB = ai.linkMatrix.find('B') != std::string::npos;
    bool hasC = ai.linkMatrix.find('C') != std::string::npos;
    size_t epos = 2 - ai.linkMatrix.find('E');
    int ipos = -1;
    if (!hasA && hasB && hasC) {
      missing = 'A';
      ipos = 0;
    } else if (!hasB && hasA && hasC) {
      missing = 'B';
      ipos = 1;
    } else if (!hasC && hasA && hasB) {
      missing = 'C';
      ipos = 2;
    }
    ai.abcLinkMask[ipos] = 9;
    ai.abcLinkOffsets[ipos] = epos;
    ai.abcLinkMaxLvdsErrRate[ipos] = -1;
  }
}

// ----------------------------------------------------------------------
void calcABCInformation(AsicInfo &ai) {
  for (int i = 0; i < 3; ++i) {
    ai.abcLinkMask[i] = 9;
    ai.abcLinkOffsets[i] = 9;
    ai.abcLinkMaxLvdsErrRate[i] = -1;
  }

  for (int i = 2; i >= 0; --i) {
    char ch = ai.linkMatrix[i];
    int off = 2-i;
    cout << "  ch = " << ch << " off = " << off << " linkMask = " << (int)(ai.linkMask[off] - '0') << endl;
    if (ch == 'A') {
      ai.abcLinkOffsets[0] = off; 
      ai.abcLinkMask[0] = (int)(ai.linkMask[i] - '0');
    }
    if (ch == 'B') {
      ai.abcLinkOffsets[1] = off; 
      ai.abcLinkMask[1] = (int)(ai.linkMask[i] - '0');
    }
    if (ch == 'C') {
      ai.abcLinkOffsets[2] = off; 
      ai.abcLinkMask[2] = (int)(ai.linkMask[i] - '0');
    }

  }

  fixSingleE(ai);

  cout << "ai.globalId = " << ai.globalId << " linkMatrix = " << ai.linkMatrix 
  << " linkMask = " << ai.linkMask
  << " FEBLinkName = " << ai.FEBLinkName
  << " abcLinkOffsets = " << ai.abcLinkOffsets[0] << ", " << ai.abcLinkOffsets[1] << ", " << ai.abcLinkOffsets[2] 
  << " abcLinkMask = " << ai.abcLinkMask[0] << ", " << ai.abcLinkMask[1] << ", " << ai.abcLinkMask[2] 
  << endl;
}

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

      ai.globalId    = a.value("globalId", 0);
      ai.FEBLinkName = a.value("FEBLink", "unset");
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
      calcABCInformation(ai);
      out.push_back(std::move(ai));
    }
  }
  return out;
}


// ----------------------------------------------------------------------
static std::string fmtLink(int fedIdx, int linkIdx) {
  std::ostringstream os;
  os << "lvds/FEB_" << std::setw(2) << std::setfill('0') << fedIdx
     << "/Link_" << std::setw(2) << std::setfill('0') << linkIdx;
  return os.str();
}


// ----------------------------------------------------------------------
void printLinkMatrixAndOffsets(const std::map<int, AsicInfo> &allAsics) {
  for (const auto& asic : allAsics) {
    cout << "  globalId = " << asic.first << " linkMatrix = " 
         << asic.second.linkMatrix << " linkMask = " << asic.second.linkMask 
         << " abcLinkOffsets = " << asic.second.abcLinkOffsets[0] << ", " << asic.second.abcLinkOffsets[1] << ", " << asic.second.abcLinkOffsets[2] 
         << " abcLinkMask = " << asic.second.abcLinkMask[0] << ", " << asic.second.abcLinkMask[1] << ", " << asic.second.abcLinkMask[2] 
         << " abcLinkNames = " << asic.second.abcLinkNames[0] << ", " << asic.second.abcLinkNames[1] << ", " << asic.second.abcLinkNames[2] 
         << endl;
  }
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
int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <file1.json> [file2.json] [file3.json] ..." << std::endl;
    return 2;
  }
  
  // Collect all ASICs from all files
  std::map<int, AsicInfo> allAsics;
  std::map<int, std::set<std::string>> patternsPerGlobalId;
  
  for (int i = 1; i < argc; ++i) {
    std::string path = argv[i];
    std::cout << "Processing file: " << path << std::endl;
    auto asics = parseJSONFile(path);
    
    for (const auto& asic : asics) {
      if (allAsics.find(asic.globalId) == allAsics.end()) {
        allAsics[asic.globalId] = asic;
      } else {
        int eCount0 = std::count(allAsics[asic.globalId].linkMatrix.begin(), allAsics[asic.globalId].linkMatrix.end(), 'E');
        int eCount1 = std::count(asic.linkMatrix.begin(), asic.linkMatrix.end(), 'E');
        if (eCount0 > eCount1) {
          allAsics[asic.globalId] = asic;
          cout << "  replacing globalId = " << asic.globalId << " linkMatrix = " << asic.linkMatrix << " is better than " << allAsics[asic.globalId].linkMatrix 
              << " eCount0 = " << eCount0 << " eCount1 = " << eCount1 << " from run " << extractRunNumber(path)
               << endl;
        } else {
          cout << "  keeping   globalId = " << asic.globalId << " linkMatrix = " << asic.linkMatrix << " is better than " << allAsics[asic.globalId].linkMatrix << endl;
        }
      }
    }
  }

  printLinkMatrixAndOffsets(allAsics);
  return 0;
}
