#include <RtypesCore.h>
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
#include "../../common/json.h"  // nlohmann::json
#include <nlohmann/json.hpp>    // for nlohmann::ordered_json

#include "util.hh"
#include "asicInfo.hh"

#include "gMapChipIDLinkOffsets.icc"

using namespace std;



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

      // -- Extract also ABC link information
      vector<int> abcmask{9,9,9}, abcmatrix{4,4,4};
      vector<long long> abcerrors{0LL,0LL,0LL};
 
      if (gMapChipIDLinkOffsets[ai.globalId][0] < 9) {
        abcmask[0] = (ai.linkMask[2-gMapChipIDLinkOffsets[ai.globalId][0]] - '0');
        abcmatrix[0] = (ai.linkMatrix[2-gMapChipIDLinkOffsets[ai.globalId][0]] - 'A') ;
      } else {
        abcmask[0] = 9;
        abcmatrix[0] = 4;
      }
      if (gMapChipIDLinkOffsets[ai.globalId][1] < 9) {
        abcmask[1] = (ai.linkMask[2-gMapChipIDLinkOffsets[ai.globalId][1]] - '0');
        abcmatrix[1] = (ai.linkMatrix[2-gMapChipIDLinkOffsets[ai.globalId][1]] - 'A') ;
      } else {
        abcmask[1] = 9;
        abcmatrix[1] = 4;
      }
      if (gMapChipIDLinkOffsets[ai.globalId][2] < 9) {
        abcmask[2] = (ai.linkMask[2-gMapChipIDLinkOffsets[ai.globalId][2]] - '0');
        abcmatrix[2] = (ai.linkMatrix[2-gMapChipIDLinkOffsets[ai.globalId][2]] - 'A') ;
      } else {
        abcmask[2] = 9;
        abcmatrix[2] = 4;
      }

      // -- lvdsErrRates moved into an array [r0, r1, r2]
      if (a.contains("lvdsErrRates") && a["lvdsErrRates"].is_array()) {
        const auto &arr = a["lvdsErrRates"];
        ai.lvdsErrRate0 = (arr.size() > 0 && arr[0].is_number()) ? arr[0].get<long long>() : 0LL;
        ai.lvdsErrRate1 = (arr.size() > 1 && arr[1].is_number()) ? arr[1].get<long long>() : 0LL;
        ai.lvdsErrRate2 = (arr.size() > 2 && arr[2].is_number()) ? arr[2].get<long long>() : 0LL;
 
         // -- errors are filled as if reading from left to right (not least significan first)
        cout << "globalId = " << ai.globalId << endl;
        int idx = 0;
        if (gMapChipIDLinkOffsets[ai.globalId][0] < 9) {
          int idx = 2 - gMapChipIDLinkOffsets[ai.globalId][0];
          cout << "idx = " << idx << " from " << gMapChipIDLinkOffsets[ai.globalId][0] << endl;
          abcerrors[0] = arr[idx];
        }
        if (gMapChipIDLinkOffsets[ai.globalId][1] < 9) {
          int idx = 2 - gMapChipIDLinkOffsets[ai.globalId][1];
          cout << "idx = " << idx << " from " << gMapChipIDLinkOffsets[ai.globalId][1] << endl;
          abcerrors[1] = arr[idx];
        }
        if (gMapChipIDLinkOffsets[ai.globalId][2] < 9) {
          int idx = 2 - gMapChipIDLinkOffsets[ai.globalId][2];
          cout << "idx = " << idx << " from " << gMapChipIDLinkOffsets[ai.globalId][2] << endl;
          abcerrors[2] = arr[idx];
        }
      } else {
        ai.lvdsErrRate0 = ai.lvdsErrRate1 = ai.lvdsErrRate2 = 0LL;
      }
 
      ai.abcLinkMask[0] = abcmask[0];
      ai.abcLinkMask[1] = abcmask[1];
      ai.abcLinkMask[2] = abcmask[2];
      ai.abcLinkErrs[0] = abcerrors[0];
      ai.abcLinkErrs[1] = abcerrors[1];
      ai.abcLinkErrs[2] = abcerrors[2];
      ai.abcLinkMatrix[0] = abcmatrix[0];
      ai.abcLinkMatrix[1] = abcmatrix[1];
      ai.abcLinkMatrix[2] = abcmatrix[2];

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
int extractRunNumber(const std::string& path) {
  // Extract basename first to avoid matching directory names like "run2025"
  std::string base = path;
  auto pos = base.find_last_of("/");
  if (pos != std::string::npos) base = base.substr(pos + 1);
  
  // Search for "run" followed by digits in the filename only
  pos = base.find("run");
  std::string digits;
  if (pos != std::string::npos) {
    for (size_t i = pos + 3; i < base.size(); ++i) {
      if (base[i] >= '0' && base[i] <= '9') {
        digits += base[i];
      } else {
        break;
      }
    }
    return std::stoi(digits);
  }
  return -1;
}

// ----------------------------------------------------------------------
vector<int> convertLinkTLAToUShort(const std::string tla, const char offset) {
  vector<int> linkMaskU(3);

  for (size_t j = 0; j < 3; ++j) {
    linkMaskU[j] = (tla[j] - offset);
  }
  cout << "tla = " << tla << " offset = " << offset << " linkMaskU = " << linkMaskU[0] << " " << linkMaskU[1] << " " << linkMaskU[2] << endl;
  return linkMaskU;
}

// ----------------------------------------------------------------------
int main(int argc, char *argv[]) {
  if (argc < 2) {
    cout << "Usage: " << argv[0] << " <output.root> [file1.json] [file2.json] ..." << endl;
    return 0; 
  }

  if (string::npos == string(argv[1]).find(".root")) {
    cout << "Usage: " << argv[0] << " <output.root> <file1.json> [file2.json] [file3.json] ..." << endl;
    cout << "Output file name is mandatory and must be a .root file." << endl;
    return 0;
  }
  TFile *f = TFile::Open(argv[1], "RECREATE");
  TTree *t = new TTree("midasMetaTree", "midasMetaTree");

  int runNumber;
  int globalChipID;
  int nlinks(3);
  int linkMask[3];
  int linkMatrix[3];
  int abcLinkMask[3];
  long long abcLinkErrs[3];
  int abcLinkMatrix[3]; // 4 = E!
  long long lvdsErrRate0;
  long long lvdsErrRate1;
  long long lvdsErrRate2;
  int ckdivend;
  int ckdivend2;
  int vdacBLPix;
  int vdacThHigh;
  int vdacThLow;
  int biasVNOutPix;
  int biasVPDAC;
  int biasVNDcl;
  int biasVNLVDS;
  int biasVNLVDSDel;
  int biasVPDcl;
  int biasVPTimerDel;
  int vdacBaseline;

  t->Branch("runNumber", &runNumber, "runNumber/I");
  t->Branch("globalChipID", &globalChipID, "globalChipID/I");
  t->Branch("nlinks", &nlinks, "nlinks/I");
  // -- NOTABENE: linkMask and linkMatrix transcribe EXACTLY the string in the JSON (A=0,B=1,C=2,E=4)
  // -- i.e. "CAB" -> "201", i.e. the order is from right to left!
  t->Branch("linkMask", linkMask, "linkMask[nlinks]/I");
  t->Branch("linkMatrix", linkMatrix, "linkMatrix[nlinks]/I");
  // -- NOTABENE: abcLinkMask and abcLinkErrs represent EXACTLY in the order ABC
  // -- i.e. [0] -> A, [1] -> B, [2] -> C
  // -- i.e. the ordering is from left to right (as in the chip submatrix or the array enumeration)
  // -- i.e. opposite ordering to linkMask and linkMatrix
  t->Branch("abcLinkMask", abcLinkMask, "abcLinkMask[nlinks]/I");
  t->Branch("abcLinkErrs", abcLinkErrs, "abcLinkErrs[nlinks]/L");
  t->Branch("abcLinkMatrix", abcLinkMatrix, "abcLinkMatrix[nlinks]/I");

  t->Branch("lvdsErrRate0", &lvdsErrRate0, "lvdsErrRate0/L");
  t->Branch("lvdsErrRate1", &lvdsErrRate1, "lvdsErrRate1/L");
  t->Branch("lvdsErrRate2", &lvdsErrRate2, "lvdsErrRate2/L");
  t->Branch("ckdivend", &ckdivend, "ckdivend/I");
  t->Branch("ckdivend2", &ckdivend2, "ckdivend2/I");
  t->Branch("vdacBLPix", &vdacBLPix, "vdacBLPix/I");
  t->Branch("vdacThHigh", &vdacThHigh, "vdacThHigh/I");
  t->Branch("vdacThLow", &vdacThLow, "vdacThLow/I");
  t->Branch("biasVNOutPix", &biasVNOutPix, "biasVNOutPix/I");
  t->Branch("biasVPDAC", &biasVPDAC, "biasVPDAC/I");
  t->Branch("biasVNDcl", &biasVNDcl, "biasVNDcl/I");
  t->Branch("biasVNLVDS", &biasVNLVDS, "biasVNLVDS/I");
  t->Branch("biasVNLVDSDel", &biasVNLVDSDel, "biasVNLVDSDel/I");
  t->Branch("biasVPDcl", &biasVPDcl, "biasVPDcl/I");
  t->Branch("biasVPTimerDel", &biasVPTimerDel, "biasVPTimerDel/I");
  t->Branch("vdacBaseline", &vdacBaseline, "vdacBaseline/I");

  for (int i = 2; i < argc; ++i) {
    std::string path = argv[i];
    std::cout << "Processing file: " << path << std::endl;
    auto asics = parseJSONFile(path);
    runNumber = extractRunNumber(path);
    for (const auto& asic : asics) {
      globalChipID = asic.globalId;
      nlinks = 3; 
      vector<int> linkMaskU = convertLinkTLAToUShort(asic.linkMask, '0');    
      if (linkMaskU.size() != 3) {
        cout << "XXXXXXXXXXXXXXXXXXXXXXX size of linkMaskU = " << linkMaskU.size() << endl;  
      }
      linkMask[0] = linkMaskU[0];
      linkMask[1] = linkMaskU[1];
      linkMask[2] = linkMaskU[2];
      cout << "asic.linkMatrix = " << asic.linkMatrix << endl;
      vector<int> linkMatrixU = convertLinkTLAToUShort(asic.linkMatrix, 'A');
      if (linkMatrixU.size() != 3) {
        cout << "XXXXXXXXXXXXXXXXXXXXXXX size of linkMatrixU = " << linkMatrixU.size() << endl;
      }
      linkMatrix[0] = linkMatrixU[0];
      linkMatrix[1] = linkMatrixU[1];
      linkMatrix[2] = linkMatrixU[2];
      abcLinkMask[0] = asic.abcLinkMask[0];
      abcLinkMask[1] = asic.abcLinkMask[1];
      abcLinkMask[2] = asic.abcLinkMask[2];
      abcLinkErrs[0] = asic.abcLinkErrs[0];
      abcLinkErrs[1] = asic.abcLinkErrs[1];
      abcLinkErrs[2] = asic.abcLinkErrs[2];
      abcLinkMatrix[0] = asic.abcLinkMatrix[0];
      abcLinkMatrix[1] = asic.abcLinkMatrix[1];
      abcLinkMatrix[2] = asic.abcLinkMatrix[2];
      lvdsErrRate0 = asic.lvdsErrRate0;
      lvdsErrRate1 = asic.lvdsErrRate1;
      lvdsErrRate2 = asic.lvdsErrRate2;
      cout << "chipID = " << globalChipID 
           << " abcLinkMask = " << abcLinkMask[0] << " " << abcLinkMask[1] << " " << abcLinkMask[2] 
           << " abcLinkErrs = " << abcLinkErrs[0] << " " << abcLinkErrs[1] << " " << abcLinkErrs[2] 
           << endl;
      ckdivend = asic.ckdivend;
      ckdivend2 = asic.ckdivend2;
      vdacBLPix = asic.vdacBLPix;
      vdacThHigh = asic.vdacThHigh;
      vdacThLow = asic.vdacThLow;
      biasVNOutPix = asic.biasVNOutPix;
      biasVPDAC = asic.biasVPDAC;
      biasVNDcl = asic.biasVNDcl;
      biasVNLVDS = asic.biasVNLVDS;
      biasVNLVDSDel = asic.biasVNLVDSDel;
      biasVPDcl = asic.biasVPDcl;
      biasVPTimerDel = asic.biasVPTimerDel;
      vdacBaseline = asic.vdacBaseline;
      t->Fill();
    }    
  }
  cout << "Writing tree to file: " << argv[1] << endl;
  f->Write();
  f->Close();
  return 0;
}