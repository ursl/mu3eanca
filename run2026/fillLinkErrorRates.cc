#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <string.h>
#include <chrono>
#include <algorithm> // for std::lower_bound

#include <fstream>

//#include "cdbUtil.hh" 
#include "Mu3eConditions.hh"
#include "calPixelAlignment.hh"
#include "calPixelQualityLM.hh"

#include "cdbJSON.hh"
#include "base64.hh"

#include <nlohmann/json.hpp>
using json = nlohmann::ordered_json;


#include "asicInfo.hh"
#include "util.hh"

#include "TCanvas.h"
#include "TStyle.h"
#include "TFile.h"
#include "TTree.h"
#include "TH2F.h"
#include "TMath.h"
#include "TKey.h"
#include "TROOT.h"
#include "TSystem.h"

using namespace std;

// ----------------------------------------------------------------------
// fillLinkErrorRates
// -----------------
// The goal of this simple program is to histogram the link error rates for non-masked links
// ----------------------------------------------------------------------

#define JSONDIR "/data/experiment/mu3e/cdb/"


// A link above this LVDS error rate [Hz] is bad.
static const long long kLvdsBadHz = 1000;

// ----------------------------------------------------------------------
void parseMidasMetaData(string filename);
void printRunInfoMap();


//string DATADIR("/Users/ursl/data/mu3e/run2026/raw");
string DATADIR("/data/experiment/mu3e/data/2026/raw");

// ----------------------------------------------------------------------
static bool fileExists(const string& filename) {
  ifstream in(filename.c_str());
  return in.good();
}


// ----------------------------------------------------------------------
string getDataSubdir(int runnumber) {
  int block = runnumber / 1000;
  string blockdir = Form("%03d", block);
  return Form("%s/%s", DATADIR.c_str(), blockdir.c_str());
}

// ----------------------------------------------------------------------
// -- VALID chipIDs for layer 1 and layer 2
// -- without this, the payloads will have too many chips 
// ----------------------------------------------------------------------
vector<int> gChipIDs = {1,2,3,4,5,6,
  33, 34, 35, 36, 37, 38,
  65, 66, 67, 68, 69, 70,
  97, 98, 99, 100, 101, 102,
  129, 130, 131, 132, 133, 134,
  161, 162, 163, 164, 165, 166,
  193, 194, 195, 196, 197, 198,
  225, 226, 227, 228, 229, 230,
  // -- layer 2
  1025, 1026, 1027, 1028, 1029, 1030,
  1057, 1058, 1059, 1060, 1061, 1062,
  1089, 1090, 1091, 1092, 1093, 1094,
  1121, 1122, 1123, 1124, 1125, 1126,
  1153, 1154, 1155, 1156, 1157, 1158,
  1185, 1186, 1187, 1188, 1189, 1190,
  1217, 1218, 1219, 1220, 1221, 1222,
  1249, 1250, 1251, 1252, 1253, 1254,
  1281, 1282, 1283, 1284, 1285, 1286,
  1313, 1314, 1315, 1316, 1317, 1318,
  // -- layer 3 central station (cf. db0/cdb2/cdbPayloadWriter.cc)
  2055, 2056, 2057, 2058, 2059, 2060, 2061, 2062, 2063, 2064, 2065, 2066, 2067, 2068, 2069, 2070, 2071, 
  2087, 2088, 2089, 2090, 2091, 2092, 2093, 2094, 2095, 2096, 2097, 2098, 2099, 2100, 2101, 2102, 2103, 
  2119, 2120, 2121, 2122, 2123, 2124, 2125, 2126, 2127, 2128, 2129, 2130, 2131, 2132, 2133, 2134, 2135, 
  2151, 2152, 2153, 2154, 2155, 2156, 2157, 2158, 2159, 2160, 2161, 2162, 2163, 2164, 2165, 2166, 2167, 
  2183, 2184, 2185, 2186, 2187, 2188, 2189, 2190, 2191, 2192, 2193, 2194, 2195, 2196, 2197, 2198, 2199, 
  2215, 2216, 2217, 2218, 2219, 2220, 2221, 2222, 2223, 2224, 2225, 2226, 2227, 2228, 2229, 2230, 2231, 
  2247, 2248, 2249, 2250, 2251, 2252, 2253, 2254, 2255, 2256, 2257, 2258, 2259, 2260, 2261, 2262, 2263, 
  2279, 2280, 2281, 2282, 2283, 2284, 2285, 2286, 2287, 2288, 2289, 2290, 2291, 2292, 2293, 2294, 2295, 
  2311, 2312, 2313, 2314, 2315, 2316, 2317, 2318, 2319, 2320, 2321, 2322, 2323, 2324, 2325, 2326, 2327, 
  2343, 2344, 2345, 2346, 2347, 2348, 2349, 2350, 2351, 2352, 2353, 2354, 2355, 2356, 2357, 2358, 2359, 
  2375, 2376, 2377, 2378, 2379, 2380, 2381, 2382, 2383, 2384, 2385, 2386, 2387, 2388, 2389, 2390, 2391, 
  2407, 2408, 2409, 2410, 2411, 2412, 2413, 2414, 2415, 2416, 2417, 2418, 2419, 2420, 2421, 2422, 2423, 
  2439, 2440, 2441, 2442, 2443, 2444, 2445, 2446, 2447, 2448, 2449, 2450, 2451, 2452, 2453, 2454, 2455, 
  2471, 2472, 2473, 2474, 2475, 2476, 2477, 2478, 2479, 2480, 2481, 2482, 2483, 2484, 2485, 2486, 2487, 
  2503, 2504, 2505, 2506, 2507, 2508, 2509, 2510, 2511, 2512, 2513, 2514, 2515, 2516, 2517, 2518, 2519, 
  2535, 2536, 2537, 2538, 2539, 2540, 2541, 2542, 2543, 2544, 2545, 2546, 2547, 2548, 2549, 2550, 2551, 
  2567, 2568, 2569, 2570, 2571, 2572, 2573, 2574, 2575, 2576, 2577, 2578, 2579, 2580, 2581, 2582, 2583, 
  2599, 2600, 2601, 2602, 2603, 2604, 2605, 2606, 2607, 2608, 2609, 2610, 2611, 2612, 2613, 2614, 2615, 
  2631, 2632, 2633, 2634, 2635, 2636, 2637, 2638, 2639, 2640, 2641, 2642, 2643, 2644, 2645, 2646, 2647, 
  2663, 2664, 2665, 2666, 2667, 2668, 2669, 2670, 2671, 2672, 2673, 2674, 2675, 2676, 2677, 2678, 2679, 
  2695, 2696, 2697, 2698, 2699, 2700, 2701, 2702, 2703, 2704, 2705, 2706, 2707, 2708, 2709, 2710, 2711, 
  2727, 2728, 2729, 2730, 2731, 2732, 2733, 2734, 2735, 2736, 2737, 2738, 2739, 2740, 2741, 2742, 2743, 
  2759, 2760, 2761, 2762, 2763, 2764, 2765, 2766, 2767, 2768, 2769, 2770, 2771, 2772, 2773, 2774, 2775, 
  2791, 2792, 2793, 2794, 2795, 2796, 2797, 2798, 2799, 2800, 2801, 2802, 2803, 2804, 2805, 2806, 2807
};

map<int, AsicInfo> gRunInfoMap;
bool gIsBeam(true);

// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {
  
  // -- command line arguments
  int verbose(0);
  // note: mode = 1 PixelQuality, 2 PixelQualityV, 3 PixelQualityM
  string jsondir(JSONDIR);
  string igt("datav6.2=2025Beam");
  string mstub("run");
  int run1(-1), run2(-1);
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-g"))      {igt = argv[++i];}
    if (!strcmp(argv[i], "-j"))      {jsondir = argv[++i];}
    if (!strcmp(argv[i], "-r1"))     {run1 = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-r2"))     {run2 = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-v"))      {verbose = atoi(argv[++i]);}
  }
  
  
  TFile *f = new TFile("linkErrorRates.root", "RECREATE");
  TH1D *hLinkErrorRates = new TH1D("hLinkErrorRates", "Link Error Rates", 1000, 0, 1.e6);
  TH1D *hLinkErrorRates2 = new TH1D("hLinkErrorRates2", "Link Error Rates 2", 1000, 0, 2.e4);
  TH2D *hLinkErrorRatesVsRun = new TH2D("hLinkErrorRatesVsRun", "Link Error Rates vs run", 400, 0, 400, 100, 0, 1.e6);
  const double linkErrY[] = {
    0, 10, 20, 50, 100, 500, 1000, 5000, 10000, 50000,
    100000, 500000, 1e6, 10e6, 100e6, 1e9, 10e9
  };
  const int nLinkErrY = sizeof(linkErrY) / sizeof(linkErrY[0]) - 1;
  TH2D *hLinkErrorRatesVsRun2 = new TH2D("hLinkErrorRatesVsRun2", "Link Error Rates vs run 2",
                                         400, 0, 400, nLinkErrY, linkErrY);
  
  if (run1 < 0 || run2 < run1) {
    cout << "need -r1 and -r2, with run1 <= run2" << endl;
    return 1;
  }

  int runIdx(1);
  for (int run = run1; run <= run2; ++run) {
    cout << "-------- run " << run << " (" << runIdx << ")" << endl;
    
    // -- open midas meta data file
    string filename = getDataSubdir(run) + Form("/%s%d.mid.lz4.json", mstub.c_str(), run);
    if (!fileExists(filename)) {
      cout << "no midas metadata (" << filename << ") for run " << run << ", skipping" << endl;
      continue;
    }
    
    
    parseMidasMetaData(filename);
    //printRunInfoMap();
        
    // ------------------------------------------------------------------    
    for (auto it : gRunInfoMap) {
      if (0) cout  << " globalChipID = " << it.first << " globalId = " << it.second.globalId 
      << " linkMask = " << it.second.linkMask[0] << it.second.linkMask[1] << it.second.linkMask[2]
      << " linkMatrix = " << it.second.linkMatrix[0] << it.second.linkMatrix[1] << it.second.linkMatrix[2]
      << " abcLinkMask = " << it.second.abcLinkMask[0] << it.second.abcLinkMask[1] << it.second.abcLinkMask[2]
      << " abcLinkMatrix = " << it.second.abcLinkMatrix[0] << it.second.abcLinkMatrix[1] << it.second.abcLinkMatrix[2]
      << " abcLinkErrs = " << it.second.abcLinkErrs[0] << ","<< it.second.abcLinkErrs[1] << "," << it.second.abcLinkErrs[2]
      << " ckdivend = " << it.second.ckdivend << " ckdivend2 = " << it.second.ckdivend2
      << endl;
      // -- loop over all links
      for (int link = 0; link < 3; ++link) {
        if (it.first >= 2055 && link > 0) continue;
        if (it.second.abcLinkMask[link] == 1) {
          hLinkErrorRates->Fill(it.second.abcLinkErrs[link]);
          hLinkErrorRates2->Fill(it.second.abcLinkErrs[link]);
          hLinkErrorRatesVsRun->Fill(runIdx, it.second.abcLinkErrs[link]);
          hLinkErrorRatesVsRun2->Fill(runIdx, it.second.abcLinkErrs[link]);
        }
      }
    }
    runIdx++;
  }
  
  f->Write();
  f->Close();
  
  cout << "This is the end, my friend" << endl;
  return 0;
}


// ----------------------------------------------------------------------
// Section names such as "L1 US 1-4" and "L2 DS 8-10".
// "Central US Layer 3" does not match; that is a one-link section.
static bool isL12Section(const string& name) {
  return name.find("L1") != string::npos
  || name.find("L2") != string::npos;
}

static bool isLayer3Section(const string& name) {
  return name.find("Layer 3") != string::npos;
}

// ----------------------------------------------------------------------
// The JSON stores linkMask, linkMatrix and lvdsErrRates left to right.
// abcLink*[link] and lvdsErrRate* are filled in right-hand order:
// link 0 is the rightmost entry. Which sensor that is comes from
// AsicInfo::letterToIndex / indexToLetter, not from the slot number.
// abcLinkMatrix holds the letter itself: 'A', 'B', 'C', 'E', or 'M'.
static void decodeLinksRhs(AsicInfo& ai, const vector<long long>& rates) {
  const int n = 3;
  for (int link = 0; link < n; ++link) {
    int src = n - 1 - link;
    ai.abcLinkMask[link] = 0;
    ai.abcLinkErrs[link] = (src < (int)rates.size() ? rates[src] : 0);
    ai.abcLinkMatrix[link] = 'E';
    ai.abcLinkNames[link] = "E";
    ai.abcLinkOffsets[link] = link;
    if ((int)ai.linkMask.size() > src) {
      char c = ai.linkMask[src];
      if (c >= '0' && c <= '9') ai.abcLinkMask[link] = c - '0';
    }
    if ((int)ai.linkMatrix.size() > src) {
      char c = ai.linkMatrix[src];
      ai.abcLinkNames[link] = string(1, c);
      if (c == 'E' || c == 'M' || (c >= 'A' && c <= 'C')) ai.abcLinkMatrix[link] = c;
    }
  }
  ai.lvdsErrRate0 = ai.abcLinkErrs[0];
  ai.lvdsErrRate1 = ai.abcLinkErrs[1];
  ai.lvdsErrRate2 = ai.abcLinkErrs[2];
}

// ----------------------------------------------------------------------
static bool linkIsBad(const AsicInfo& ai, int link) {
  return ai.abcLinkMatrix[link] == 'E' || ai.abcLinkErrs[link] > kLvdsBadHz;
}

// ----------------------------------------------------------------------
// -- gRunInfoMap is a std::map, so iteration is in globalId order.
void printRunInfoMap() {
  cout << "gRunInfoMap " << gRunInfoMap.size() << " entries, sorted by globalId" << endl;
  bool badCase(false);
  for (const auto& it : gRunInfoMap) {
    const AsicInfo& ai = it.second;
    cout << "globalId=" << it.first
    << " " << ai.FEBName
    << " linkMatrix=" << ai.linkMatrix
    << " linkMask=" << ai.linkMask;
    for (int link = 0; link < 3; ++link) {
      if (ai.abcLinkMask[link] == 9) continue;
      char letter = '?';
      bool mapped = ai.indexToLetter(link, letter);
      bool bad = !mapped || linkIsBad(ai, link);
      cout << " | link" << link
      << " " << (mapped ? string(1, letter) : ai.abcLinkNames[link])
      << " mask=" << ai.abcLinkMask[link]
      << " rate=" << ai.abcLinkErrs[link]
      << (bad ? " BAD" : "");
    }
    for (char letter : {'A', 'B', 'C'}) {
      int idx = -1;
      if (ai.letterToIndex(letter, idx)) cout << " " << letter << "->link" << idx;
      else cout << " " << letter << "->?";
    }
    if (badCase) {
      cout << "BAD CASE" << endl;
    }
    cout << endl;
  }
}

// ----------------------------------------------------------------------
// One physical link reads out submatrices A, B, and C. The same mask,
// rate, and letter 'M' are stored in every slot.
static void setSingleLink(AsicInfo& ai, int mask, long long rate) {
  char digit = (mask >= 0 && mask <= 9) ? static_cast<char>('0' + mask) : '0';
  ai.linkMask = string(3, digit);
  ai.linkMatrix = "MMM";
  ai.lvdsErrRate0 = rate;
  ai.lvdsErrRate1 = rate;
  ai.lvdsErrRate2 = rate;
  for (int link = 0; link < 3; ++link) {
    ai.abcLinkMask[link] = mask;
    ai.abcLinkErrs[link] = rate;
    ai.abcLinkMatrix[link] = 'M';
    ai.abcLinkNames[link] = "M";
    ai.abcLinkOffsets[link] = link;
  }
}

// ----------------------------------------------------------------------
void parseMidasMetaData(string filename) {
  if (!fileExists(filename)) {
    cout << "parseMidasMetaData> ERROR: file " << filename << " does not exist" << endl;
    return;
  }
  json j;
  ifstream f(filename);
  f >> j;
  if (!j.contains("febs") || !j["febs"].is_array()) {
    cout << "parseMidasMetaData> ERROR: no febs array in " << filename << endl;
    return;
  }
  
  gRunInfoMap.clear();
  int nAsic = 0;
  int nBad = 0;
  int nLayer3Skip = 0;
  for (const auto& section : j["febs"]) {
    if (!section.contains("name") || !section["name"].is_string()) continue;
    string secName = section["name"].get<string>();
    if (!section.contains("asics") || !section["asics"].is_array()) continue;
    
    if (isLayer3Section(secName)) {
      for (const auto& a : section["asics"]) {
        int gid = a.value("globalId", 0);
        if (gid <= 0) {
          ++nLayer3Skip;
          continue;
        }
        if (!a.contains("linkMask") || !a["linkMask"].is_string()) {
          cout << "parseMidasMetaData> WARNING: " << secName
          << " globalId=" << gid << " has no linkMask" << endl;
          continue;
        }
        AsicInfo ai;
        ai.FEBName = secName;
        ai.fedID = section.value("index", -1);
        ai.confId = a.value("confId", 0);
        ai.globalId = gid;
        ai.idxInSection = a.value("index", 0);
        ai.FEBLinkName = a.value("FEBLink", "");
        ai.linkMask = a.value("linkMask", "");
        if (a.contains("dacs") && a["dacs"].contains("conf")) {
          ai.ckdivend = a["dacs"]["conf"].value("ckdivend", 0);
          ai.ckdivend2 = a["dacs"]["conf"].value("ckdivend2", 0);
        }
        long long rate = 0;
        if (a.contains("lvdsErrRate") && a["lvdsErrRate"].is_number()) {
          rate = a["lvdsErrRate"].get<long long>();
        }
        int mask = 0;
        if (!ai.linkMask.empty() && ai.linkMask[0] >= '0' && ai.linkMask[0] <= '9') {
          mask = ai.linkMask[0] - '0';
        }
        setSingleLink(ai, mask, rate);
        bool bad = linkIsBad(ai, 0);
        if (bad) ++nBad;
        if (gRunInfoMap.count(ai.globalId)) {
          cout << "parseMidasMetaData> WARNING: duplicate globalId " << ai.globalId << endl;
        }
        gRunInfoMap[ai.globalId] = ai;
        ++nAsic;
      }
      continue;
    }
    
    if (!isL12Section(secName)) continue;
    
    for (const auto& a : section["asics"]) {
      if (!a.contains("linkMatrix") || !a["linkMatrix"].is_string()) continue;
      AsicInfo ai;
      ai.FEBName = secName;
      ai.fedID = section.value("index", -1);
      ai.confId = a.value("confId", 0);
      ai.globalId = a.value("globalId", 0);
      ai.FEBLinkName = a.value("FEBLink", "");
      ai.linkMask = a.value("linkMask", "");
      ai.linkMatrix = a.value("linkMatrix", "");
      if (a.contains("dacs") && a["dacs"].contains("conf")) {
        ai.ckdivend = a["dacs"]["conf"].value("ckdivend", 0);
        ai.ckdivend2 = a["dacs"]["conf"].value("ckdivend2", 0);
      }
      
      vector<long long> rates(3, 0);
      if (a.contains("lvdsErrRates") && a["lvdsErrRates"].is_array()) {
        const auto& arr = a["lvdsErrRates"];
        for (size_t i = 0; i < arr.size() && i < rates.size(); ++i) {
          if (arr[i].is_number()) rates[i] = arr[i].get<long long>();
        }
      }
      decodeLinksRhs(ai, rates);
      
      if (gRunInfoMap.count(ai.globalId)) {
        cout << "parseMidasMetaData> WARNING: duplicate globalId " << ai.globalId << endl;
      }
      gRunInfoMap[ai.globalId] = ai;
      ++nAsic;
    }
  }
  if (nLayer3Skip) {
    cout << "parseMidasMetaData> Layer 3 skipped " << nLayer3Skip
    << " ASICs with no globalId" << endl;
  }
} 
