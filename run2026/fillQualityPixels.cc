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

using namespace std;

// ----------------------------------------------------------------------
// fillQualityPixels
// -----------------
// Determine (placeholder/offline) DQM information for pixelQualityLM
//
// Notes: 
// (0) Based on 2025 version
// (1) Dead/problematic links are determined based on average (per-link) hits
// (2) Dead/problematic columns are determined based on average (per-column) hits 
// (3) Noisy pixels are determined on hit counts being larger than 10*sigma of the average (per-pixel) hits. 
// (4) If more than 40 of the pixels in a column are noisy, the entire column is flagged as suspect.
// (5) Since columns do not have a quality flag, both dead and suspect columns simply appear as "bad" columns
// 
// Examples:
// ---------
// cd mu3eanca/run2026
//    ./bin/fillQualityPixels -i datav7.1=2026V0 -g datav7.1=2026V0 \
//    -j ~/data/mu3e/cdb/ -f ~/data/mu3e/run2026/mlzr/merged-dqm_histos_06163.root 
//    -r datav6.4=2025V0-MidasMeta2025Data.root
//
// Create _iov_1 payload:
// moor>./bin/fillQualityPixels -i datav6.3=2025V0 -g datav6.4=2025V0 -j ~/data/mu3e/cdb/ -p 4
// ----------------------------------------------------------------------

#define JSONDIR "/Users/ursl/data/mu3e/cdb"
#define CSVSCHEMA "#chipID,ckdivend,ckdivend2,linkA,linkB,linkC,linkM,ncol[,icol,iqual],npix[,icol,irow,iqual]"
// ----------------------------------------------------------------------
struct pixhit {
  unsigned int ichip;
  int icol, irow;
  int tot;
  int status;
};

// ----------------------------------------------------------------------
void parseMidasMetaData(string filename);
void printRunInfoMap();
void createPayload(string, calAbs *, string, string, string);
void chipIDSpecBook(int chipid, int &station, int &layer, int &phi, int &z);
bool determineBrokenLinks(TH2 *h, vector<int> &links);
bool determineBrokenLinksV0(TH2 *h, vector<int> &links);
void determineDeadColumns(TH2 *h, vector<int> &colums, vector<int> &links);
void determineNoisyPixels(TH2 *h, vector<int> &pixels, vector<int> &colums, vector<int> &links); // icol,irow,iqual


string DATADIR("/Users/ursl/data/mu3e/run2026/raw");
//string DATADIR("/data/experiment/mu3e/data/2026/raw");

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
          2056, 2057, 2058, 2059, 2060, 2061, 2062, 2063, 2064, 2065, 2066, 2067, 2068, 2069, 2070, 2071, 2072, 
          2088, 2089, 2090, 2091, 2092, 2093, 2094, 2095, 2096, 2097, 2098, 2099, 2100, 2101, 2102, 2103, 2104, 
          2120, 2121, 2122, 2123, 2124, 2125, 2126, 2127, 2128, 2129, 2130, 2131, 2132, 2133, 2134, 2135, 2136, 
          2152, 2153, 2154, 2155, 2156, 2157, 2158, 2159, 2160, 2161, 2162, 2163, 2164, 2165, 2166, 2167, 2168, 
          2184, 2185, 2186, 2187, 2188, 2189, 2190, 2191, 2192, 2193, 2194, 2195, 2196, 2197, 2198, 2199, 2200, 
          2216, 2217, 2218, 2219, 2220, 2221, 2222, 2223, 2224, 2225, 2226, 2227, 2228, 2229, 2230, 2231, 2232, 
          2248, 2249, 2250, 2251, 2252, 2253, 2254, 2255, 2256, 2257, 2258, 2259, 2260, 2261, 2262, 2263, 2264, 
          2280, 2281, 2282, 2283, 2284, 2285, 2286, 2287, 2288, 2289, 2290, 2291, 2292, 2293, 2294, 2295, 2296, 
          2312, 2313, 2314, 2315, 2316, 2317, 2318, 2319, 2320, 2321, 2322, 2323, 2324, 2325, 2326, 2327, 2328, 
          2344, 2345, 2346, 2347, 2348, 2349, 2350, 2351, 2352, 2353, 2354, 2355, 2356, 2357, 2358, 2359, 2360, 
          2376, 2377, 2378, 2379, 2380, 2381, 2382, 2383, 2384, 2385, 2386, 2387, 2388, 2389, 2390, 2391, 2392, 
          2408, 2409, 2410, 2411, 2412, 2413, 2414, 2415, 2416, 2417, 2418, 2419, 2420, 2421, 2422, 2423, 2424, 
          2440, 2441, 2442, 2443, 2444, 2445, 2446, 2447, 2448, 2449, 2450, 2451, 2452, 2453, 2454, 2455, 2456, 
          2472, 2473, 2474, 2475, 2476, 2477, 2478, 2479, 2480, 2481, 2482, 2483, 2484, 2485, 2486, 2487, 2488, 
          2504, 2505, 2506, 2507, 2508, 2509, 2510, 2511, 2512, 2513, 2514, 2515, 2516, 2517, 2518, 2519, 2520, 
          2536, 2537, 2538, 2539, 2540, 2541, 2542, 2543, 2544, 2545, 2546, 2547, 2548, 2549, 2550, 2551, 2552, 
          2568, 2569, 2570, 2571, 2572, 2573, 2574, 2575, 2576, 2577, 2578, 2579, 2580, 2581, 2582, 2583, 2584, 
          2600, 2601, 2602, 2603, 2604, 2605, 2606, 2607, 2608, 2609, 2610, 2611, 2612, 2613, 2614, 2615, 2616, 
          2632, 2633, 2634, 2635, 2636, 2637, 2638, 2639, 2640, 2641, 2642, 2643, 2644, 2645, 2646, 2647, 2648, 
          2664, 2665, 2666, 2667, 2668, 2669, 2670, 2671, 2672, 2673, 2674, 2675, 2676, 2677, 2678, 2679, 2680, 
          2696, 2697, 2698, 2699, 2700, 2701, 2702, 2703, 2704, 2705, 2706, 2707, 2708, 2709, 2710, 2711, 2712, 
          2728, 2729, 2730, 2731, 2732, 2733, 2734, 2735, 2736, 2737, 2738, 2739, 2740, 2741, 2742, 2743, 2744, 
          2760, 2761, 2762, 2763, 2764, 2765, 2766, 2767, 2768, 2769, 2770, 2771, 2772, 2773, 2774, 2775, 2776, 
          2792, 2793, 2794, 2795, 2796, 2797, 2798, 2799, 2800, 2801, 2802, 2803, 2804, 2805, 2806, 2807, 2808      
          };



map<int, AsicInfo> gRunInfoMap;
bool gIsBeam(true);

// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {

  // -- filter on US only?
  bool filterUS(true);

  // -- chipIDs that are in fact turned on (complete vertex)
  vector<pair<int, int> > vLayLdrTurnedOn = {
    {1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}, {1, 7}, {1, 8},
    {2, 1}, {2, 2}, {2, 3}, {2, 4}, {2, 5}, {2, 6}, {2, 7}, {2, 8}, {2, 9}, {2, 10}
  };
 
  // -- command line arguments
  int verbose(0), mode(1), printMode(0);
  // note: mode = 1 PixelQuality, 2 PixelQualityV, 3 PixelQualityM
  string jsondir(JSONDIR), filename("nada.root");
  string gt("datav6.3=2025V0");
  string igt("datav6.2=2025Beam");
  string rootMetaMidasFilename("nada.root");
  string comment("no comment provided :-(");
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-c"))      {comment = argv[++i];}
    if (!strcmp(argv[i], "-f"))      {filename = argv[++i];}
    if (!strcmp(argv[i], "-g"))      {gt = argv[++i];}
    if (!strcmp(argv[i], "-i"))      {igt = argv[++i];}
    if (!strcmp(argv[i], "-j"))      {jsondir = argv[++i];}
    if (!strcmp(argv[i], "-m"))      {mode = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-p"))      {printMode = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-r"))      {rootMetaMidasFilename = argv[++i];}
    if (!strcmp(argv[i], "-v"))      {verbose = atoi(argv[++i]);}
  }
  

  parseMidasMetaData(rootMetaMidasFilename);
  printRunInfoMap();

  cout << "printMode = " << printMode << endl;

  // -- this is just to get the list of all chipIDs
  cdbAbs *pDB = new cdbJSON(jsondir, verbose);
  Mu3eConditions* pDC = Mu3eConditions::instance(igt, pDB);
  pDC->setRunNumber(1);
  if(!pDC->getDB()) {
      std::cout << "CDB database not found" << std::endl;
  }

  if (1 <= printMode && printMode <= 3) {
    cout << "print chipIDs printMode = " << printMode << endl;
    int station(0), layer(0), phi(0), z(0);
    calAbs* cal = pDC->getCalibration("pixelalignment_");
    calPixelAlignment* cpa = dynamic_cast<calPixelAlignment*>(cal);
    uint32_t i = 0;
    cpa->resetIterator();
    while(cpa->getNextID(i)) {
      uint32_t chipID = cpa->id(i);
      chipIDSpecBook(chipID, station, layer, phi, z);
      if (printMode == 1 && (layer < 3)) {
        // -- print all chipIDs
        cout << "chipID = " << cpa->id(i) << endl;
      } else if (printMode == 2 && (layer < 3) && (z < 4)) {
        // -- print US chipIDs
        cout << "chipID = " << cpa->id(i) <<  endl;
      } else if (printMode == 3 && (layer< 3) && (z > 3)) {
        // -- print DS chipIDs
        cout << "chipID = " << cpa->id(i) << endl;

      }

    }
  }
  if (4 == printMode) {
    int station(0), layer(0), phi(0), z(0);

    // -- create turned on CSV for all chipIDs
    ofstream ofs;
    string filename = Form("csv/deadlinks-allpixels.csv");
    ofs.open(filename);
    ofs << CSVSCHEMA << " NB: " << calPixelQualityLM::getStatusDocumentation() << endl;
    
    cout << "print all chipIDs" << endl;
    calAbs* cal = pDC->getCalibration("pixelalignment_");
    calPixelAlignment* cpa = dynamic_cast<calPixelAlignment*>(cal);
    uint32_t i = 0;
    cpa->resetIterator();
    while(cpa->getNextID(i)) {
      uint32_t chipID = cpa->id(i);
      chipIDSpecBook(chipID, station, layer, phi, z);
      if (layer < 3) {
        cout << "chipID = " << cpa->id(i) << " station/layer/phi/z = " << station << "/" << layer << "/" << phi << "/" << z << endl;
        ofs << cpa->id(i) << "," << 31 << "," << 0 << "," << 0 << "," << 0 << "," << 0 << "," << 0 << "," << 0 << "," << 0 << endl;
      }
    }  
    ofs.close();

    string hash = string("tag_pixelqualitylm_") + gt + string("_iov_1");
    calPixelQualityLM *cpq = new calPixelQualityLM();
    cpq->readCsv(Form("csv/deadlinks-allpixels.csv"));
    string blob = cpq->makeBLOB();
    string schema = cpq->getSchema();
    string payloadcomment = comment + " " +  calPixelQualityLM::getStatusDocumentation();
    cout << "XXXXXXXXX createPayload with hash = " << hash << " comment = " << payloadcomment << " schema = " << schema << endl;
    createPayload(hash, cpq, "./payloads", schema, payloadcomment);
  
    return 0; 
  }


  vector<uint32_t> allSensorIDs;
  allSensorIDs.reserve(3000);
  calAbs* cal = pDC->getCalibration("pixelalignment_");
  calPixelAlignment* cpa = dynamic_cast<calPixelAlignment*>(cal);
  uint32_t i = 0;
  cpa->resetIterator();
  while(cpa->getNextID(i)) {
      allSensorIDs.push_back(cpa->id(i));
  }
  
  int run(-1);
  string sbla(filename);
  cout << "filename ->" << filename << "<-" << endl;
  if (string::npos != filename.find_last_of("/")) {
    string dir = filename.substr(0, filename.find_last_of("/")+1);
    cout << "dir ->" << dir << "<-" << endl;
    (sbla, dir, "");
  }
  // -- extract the runnumber and runclass
  replaceAll(sbla, ".root", "");
  size_t rIdx = sbla.rfind("_")+1;
  sbla = sbla.substr(rIdx);
  cout << "sbla ->" << sbla << "<-" << endl;  
  run = ::stoi(sbla);
  pDC->setRunNumber(run);
  runRecord rr = pDC->getRunRecord(run);
  rr.print();
  string runclass = rr.fBORRunClass;
  int qVertex = rr.fvDQ[rr.fDataQualityIdx].vertex;
  cout << "run = " << run << " runclass = " << runclass << " qVertex = " << qVertex << endl;
  if (runclass.find("Cosmic") != string::npos) {
    gIsBeam = false;
  }
  if (runclass.find("Calibration") != string::npos) {
    gIsBeam = false;
  }
  if (runclass.find("Test") != string::npos) {
    gIsBeam = false;
  }
  cout << "gIsBeam = " << gIsBeam << endl;

  // ------------------------------------------------------------------
  cout << "gRunInfoMap.size() = " << gRunInfoMap.size() << endl;
  for (auto it : gRunInfoMap) {
    cout << "run = " << it.second.confId
      << " globalChipID = " << it.first << " globalId = " << it.second.globalId 
      << " linkMask = " << it.second.linkMask[0] << it.second.linkMask[1] << it.second.linkMask[2]
      << " linkMatrix = " << it.second.linkMatrix[0] << it.second.linkMatrix[1] << it.second.linkMatrix[2]
      << " abcLinkMask = " << it.second.abcLinkMask[0] << it.second.abcLinkMask[1] << it.second.abcLinkMask[2]
      << " abcLinkMatrix = " << it.second.abcLinkMatrix[0] << it.second.abcLinkMatrix[1] << it.second.abcLinkMatrix[2]
      << " abcLinkErrs = " << it.second.abcLinkErrs[0] << ","<< it.second.abcLinkErrs[1] << "," << it.second.abcLinkErrs[2]
      << " ckdivend = " << it.second.ckdivend << " ckdivend2 = " << it.second.ckdivend2
      << endl;
  }
  
  // -------------------------
  // -- ckdivend2 and ckdivend
  // -------------------------
  int ckdivendDefault(0), ckdivend2Default(31);
  if (run < 3098) {
    ckdivend2Default = 15;
  } else {
    ckdivend2Default = 31;
  }
  // FIXME
  int ckdivend(gRunInfoMap[1].ckdivend), ckdivend2(gRunInfoMap[1].ckdivend2);

  string hash = string("tag_pixelqualitylm_") + gt + string("_iov_") + to_string(run);
  
  cout << "hallo" << endl;
  TFile *f = TFile::Open(filename.c_str());
  if (f) {
    if (!f->IsOpen()) {
      cout << "XXXXXXXXX pixelFillQualityLM failed to open file " << filename << endl;
      return 0;
    }
  } else {
    cout  << "XXXXXXXXX pixelFillQualityLM not found file " << filename << endl;
    return 0;
  }

  // -- read in ALL chipids in VTX
  vector<unsigned int> vchipid;
  vector<string> vStations = {"station_0"};
  map<string, int> vLayers  = {{"layer_1", 8}, {"layer_2", 10}};
  map<int, TH2*> mHitmaps;

  // -- read in all hitmaps
  for (auto itS : vStations) {
    for (auto itL : vLayers) {
      for (int iLdr = 1; iLdr <= itL.second; ++iLdr) {
        stringstream ss;
        ss << "pixel/hitmaps/" << itS << "/" << itL.first
           << Form("/ladder_%02d", iLdr);
        string s = ss.str();
        if (0) cout << "TDirectory " << s << "<-" << endl;
        f->cd(s.c_str());
        TIter next(gDirectory->GetListOfKeys());
        TKey *key(0);
        while ((key = (TKey *)next())) {
          if (key) {
            if (0) cout << "found key " << s << endl;
            if (gROOT->GetClass(key->GetClassName())->InheritsFrom("TDirectory")) continue;
            if (!gROOT->GetClass(key->GetClassName())->InheritsFrom("TH1")) continue;       
            string name = key->GetName();
            string oname = s + "/" + name;
            if (string::npos != name.find("hitmap_perChip")) {
              unsigned int chipID;
              replaceAll(name, "hitmap_perChip_", "");
              if (1) cout << "   .. chipID = " << chipID << " from name = " << oname << endl;
              chipID = ::stoi(name); 
              if (0 == chipID) {
                cout << "  XXXXXXXXX  skipping illegal chipID = " << chipID << " from name = " << name << " is 0, skipping" << endl;
                continue;
              }
              vchipid.push_back(chipID);
              TH2F *h = (TH2F*)key->ReadObj();
              mHitmaps.insert(make_pair(chipID, h));
            }
          }
        }
      }
    }
  }

  cout << "mHitmaps.size() = " << mHitmaps.size() << endl;

  // -- do analysis
  vector<int> deadlinks, deadcolumns, noisyPixels;
  ofstream ofs;
  ofs.open(Form("csv/pixelqualitylm-run%d.csv", run));
  ofs << CSVSCHEMA << " NB: " << calPixelQualityLM::getStatusDocumentation() << endl;
  for (auto it: mHitmaps){
    // -- debug with first 12 only FIXME
    //if (it.first != 1315) continue;
    // -- check if chipID is in the list of valid chipIDs
    if (find(gChipIDs.begin(), gChipIDs.end(), it.first) == gChipIDs.end()) {
      cout << "XXXXXXXXXchipID = " << it.first << " is not in the list of valid chipIDs" << endl;
      continue;
    }
    // -- clear all vectors for this new chipID
    deadlinks.clear();
    deadcolumns.clear();
    noisyPixels.clear();
    // -- determine broken links
    cout << "mHitmaps[" << it.first << "] : " << mHitmaps[it.first]->GetName() << endl;
    bool turnedOn = determineBrokenLinks(mHitmaps[it.first], deadlinks);
    cout << "chipID = " << it.first << " turnedOn = " << turnedOn << endl;
    if (deadlinks.size() > 0) {
      cout << "chipID = " << it.first << " with link quality: ";
      ofs << it.first << ",";
      // -- ckdivend and ckdivend2
      ofs << ckdivend << "," << ckdivend2 << ",";
      for (auto itL : deadlinks) {
        ofs << itL << ",";
        cout << itL << " ";
      }
      cout << endl;
    } else {
      cout << "chipID = " << it.first << " has no broken links" << endl;
    }
    // -- determine dead columns
    if (gIsBeam) {
      determineDeadColumns(mHitmaps[it.first], deadcolumns, deadlinks);
    } else {
      deadcolumns.clear();
    }
   
    // -- determine noisy pixels
    determineNoisyPixels(mHitmaps[it.first], noisyPixels, deadcolumns, deadlinks);

    // -- write out dead columns only after potential correction in noisy pixel determination
    if (deadcolumns.size() > 0) {
      cout << "chipID = " << it.first << " has " << deadcolumns.size()/2 << " dead/bad columns: ";
      ofs << deadcolumns.size()/2 << ",";
      for (auto itC = deadcolumns.begin(); itC != deadcolumns.end(); ++itC) {
        cout << *itC << " ";
        ofs << *itC << ",";
      }
      cout << endl;
    } else {
      cout << "chipID = " << it.first << " has no dead columns" << endl;
      ofs << 0 << ",";
    }
   
    if (noisyPixels.size() > 0) {
      cout << "chipID = " << it.first << " has " << noisyPixels.size()/3 << " noisy pixels: ";
      ofs << noisyPixels.size()/3 << ",";
      for (auto itN = noisyPixels.begin(); itN != noisyPixels.end(); ++itN) {
        cout << *itN << " ";
        ofs << *itN;
        if (itN != noisyPixels.end() - 1) ofs << ",";
      }
      cout << endl;
    } else {
      cout << "chipID = " << it.first << " has no noisy pixels" << endl;
      ofs << 0;
    }
    
    ofs << endl;
    //break;
  }
  ofs.close();

  cout << "READING CSV: " << Form("csv/pixelqualitylm-run%d.csv", run) << endl;
  calPixelQualityLM *cpq = new calPixelQualityLM();
  cpq->readCsv(Form("csv/pixelqualitylm-run%d.csv", run));
  string schema = cpq->getSchema();
  string payloadcomment = comment + " " +  calPixelQualityLM::getStatusDocumentation();

  createPayload(hash, cpq, "./payloads", schema, payloadcomment);

  if (0) {
    cout << "READ CSV: " << Form("csv/pixelqualitylm-run%d.csv", run) << endl;
    cout << "WRITING CSV for validation: " << Form("csv/validatepixelqualitylm-run%d.csv", run) << endl;
    cpq->writeCsv(Form("csv/validatepixelqualitylm-run%d.csv", run));
  }
  cout << "This is the end, my friend" << endl;
  return 0;
}

// ----------------------------------------------------------------------
void createPayload(string hash, calAbs *a, string jsondir, string schema, string comment) {

  string sblob = a->makeBLOB();
  payload pl;
  pl.fHash = hash;
  pl.fComment = comment;
  pl.fSchema = schema;
  pl.fBLOB = sblob;
  cout << "######################################################################" << endl;
  cout << "### createPayload" << endl;
  cout <<  pl.printString(false) << endl;
  a->printBLOB(sblob);
  cout << "######################################################################" << endl;
  a->writePayloadToFile(hash, jsondir, pl);
}

// ----------------------------------------------------------------------
void chipIDSpecBook(int chipid, int &station, int &layer, int &phi, int &z) {
  station = chipid/(0x1<<12);
  layer   = chipid/(0x1<<10) % 4 + 1;
  phi     = chipid/(0x1<<5) % (0x1<<5) + 1;
 
  int zp  = chipid % (0x1<<5);
 
  if (layer == 3) {
    z = zp - 7;
  } else if (layer == 4) {
    z = zp - 6;
  } else {
    z = zp;
  }

}

// ----------------------------------------------------------------------
// -- this is completely new and now relies on PCLS data read from midas meta data tree
bool determineBrokenLinks(TH2 *h, vector<int> &links) {
  bool DBX(true);

  // -- the histogram still used to provide the chip name
  string schip = h->GetName();
  replaceAll(schip, "hitmap_perChip_", "");
  int chipID = ::stoi(schip);

  AsicInfo ai = gRunInfoMap[chipID];

  if (DBX) cout << "XXX chipID = " << chipID << " linkMask = " << ai.abcLinkMask[0] << " " << ai.abcLinkMask[1] << " " << ai.abcLinkMask[2] << endl;
  
  // -- hand-curated list to avoid getting lost in algorithmic nightmares
  vector<int> vBadChips = {};

  if (find(vBadChips.begin(), vBadChips.end(), chipID) != vBadChips.end()) {  
    links.push_back(calPixelQualityLM::DeclaredBad);
    links.push_back(calPixelQualityLM::DeclaredBad);
    links.push_back(calPixelQualityLM::DeclaredBad);
    links.push_back(0); // now used to keep 1./LVDS-overflow-rate
    return false;
  }
  
  // -- completely masked chip
  if (0 == ai.abcLinkMask[0]
      && 0 == ai.abcLinkMask[1]
      && 0 == ai.abcLinkMask[2]) {
    links.push_back(calPixelQualityLM::Masked);
    links.push_back(calPixelQualityLM::Masked);
    links.push_back(calPixelQualityLM::Masked);
    links.push_back(0); // now used to keep 1./LVDS-overflow-rate
    return false;
  }

  // -- completely dead chips
  double nLinkA = h->Integral(1, 88, 1, 250);
  double nLinkB = h->Integral(89, 172, 1, 250);
  double nLinkC = h->Integral(173, 256, 1, 250);
  vector<double> vLink = {nLinkA, nLinkB, nLinkC};
  double nLinkAverage = (nLinkA + nLinkB + nLinkC)/3.;
  if (DBX) cout << "  nLinkA = " << nLinkA << " nLinkB = " << nLinkB << " nLinkC = " << nLinkC << " nLinkAverage = " << nLinkAverage << endl;
  if (nLinkA < 1 && nLinkB < 1 && nLinkC < 1) {
    links.push_back(calPixelQualityLM::NoHits);
    links.push_back(calPixelQualityLM::NoHits);
    links.push_back(calPixelQualityLM::NoHits);
    links.push_back(0); // now used to keep 1./LVDS-overflow-rate
    return false;
  }

  // -- check individual links. i = 0,1,2 is sensor A, B, C.
  //    Mask and rate live at the right-hand index of that letter.
  int lkStatus[4] = {0, 0, 0, 0};
  int nBadLinks(0);
  int badChip = -1;
  bool badLink[3] = {false, false, false};
  static const char kSensor[3] = {'A', 'B', 'C'};
  for (int i = 0; i < 3; ++i) {
    int idx = -1;
    if (!ai.letterToIndex(kSensor[i], idx)) {
      if (DBX) cout << "Chip " << chipID << " sensor " << kSensor[i] << " unmapped " << idx << endl;
      lkStatus[i] = calPixelQualityLM::LVDSErrorLink;
      continue;
    }
    if (vLink[i] < 1) lkStatus[i] = calPixelQualityLM::NoHits;
    if (0 == ai.abcLinkMask[idx]) lkStatus[i] = calPixelQualityLM::Masked;
    if (9 == ai.abcLinkMask[idx]) lkStatus[i] = calPixelQualityLM::Masked;
    if (ai.abcLinkErrs[idx] > 10.) lkStatus[i] = calPixelQualityLM::LVDSErrorLink;
    // -- if LVDS error rate is high and link is not masked, set to badChip!
    if (ai.abcLinkErrs[idx] > 10. && 1 == ai.abcLinkMask[idx]) {
      badChip = 1;
      ++nBadLinks;
      badLink[i] = true;
    }
  }
 
  if (DBX) {
    for (int i = 0; i < 3; ++i) {
      int idx = -1;
      bool mapped = ai.letterToIndex(kSensor[i], idx);
      if (DBX) cout << "Chip " << chipID << " sensor " << kSensor[i] << " status = " << lkStatus[i]
         << " idx = " << (mapped ? idx : -1)
         << " mask = " << (mapped ? ai.abcLinkMask[idx] : -1)
         << " errs = " << (mapped ? ai.abcLinkErrs[idx] : -1)
         << " badLink = " << badLink[i]
         << endl;
    }
  }

  // -- Check for row overflow (indicating LVDS errors), stored in the fourth lkStatus[]
  int iOverFlow = h->Integral(1, 256, 251, 252);
  if (0) cout << "Chip " << chipID << " overflow bin: " << iOverFlow << endl;

  if (iOverFlow > 0) {
    double nhits = h->Integral(1, 256, 1, 250);
    uint32_t istatus = static_cast<uint32_t>(nhits/iOverFlow);
    if (iOverFlow > nhits) istatus = 1;
    if (nhits/iOverFlow > UINT32_MAX) istatus = UINT32_MAX;
    lkStatus[3] = istatus;
    if (DBX) cout << "Chip " << chipID << " overflow: " << iOverFlow << " nhits = " << nhits << " status = " << istatus 
         << " UINT32_MAX = " << UINT32_MAX << endl;
  } else {
    lkStatus[3] = 0;
    if (DBX) cout << "Chip " << chipID << " NO overflow: " << iOverFlow << " status = " << lkStatus[3] <<  endl;
  }

  // -- badChip is true for unmasked links with LVDS error rates > 10 Hz. 
  //    These leak into other matrices.
  if (badChip >= 0) {    
    cout << "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZ badChip = " << badChip << " lkStatus = " << lkStatus[0] << " " << lkStatus[1] << " " << lkStatus[2] << endl;
    for (int i = 0; i < 3; ++i) {
      if (badLink[i]) {
        lkStatus[i] = calPixelQualityLM::LVDSErrorLink;
      } else {
        // -- do not override 9 status (masked link stays masked)
        if (lkStatus[i] != calPixelQualityLM::Masked) {
          lkStatus[i] = calPixelQualityLM::LVDSErrorOtherLink;
        }
        // -- if masked link has hits, set to bad
        if (calPixelQualityLM::Masked == lkStatus[i] && vLink[i] > 0) {
          lkStatus[i] = calPixelQualityLM::LVDSErrorTopBottomEdge;
        }
      }
    }
  }

  // -- hits piled at the top or bottom edge. Only a sensor that is still
  //    Good is marked; E, mask, and a high LVDS rate stay as they are.
  vector<double> vLinkMeanY;
  vLinkMeanY.push_back(h->ProjectionY("A", 1, 88)->GetMean());
  vLinkMeanY.push_back(h->ProjectionY("B", 89, 172)->GetMean());
  vLinkMeanY.push_back(h->ProjectionY("C", 173, 256)->GetMean());
  badChip = -1;
  if (nLinkA > 0 && (vLinkMeanY[0] < 10 || vLinkMeanY[0] > 245)) badChip = 1;
  if (nLinkB > 0 && (vLinkMeanY[1] < 10 || vLinkMeanY[1] > 245)) badChip = 2;
  if (nLinkC > 0 && (vLinkMeanY[2] < 10 || vLinkMeanY[2] > 245)) badChip = 3;
  if (badChip > 0) {
    cout << "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZ badChip = " << badChip
         << " vLinkMeanY = " << vLinkMeanY[0] << " " << vLinkMeanY[1] << " " << vLinkMeanY[2]
         << endl;
    const double nHits[3] = {nLinkA, nLinkB, nLinkC};
    const bool atEdge[3] = {
      nHits[0] > 0 && (vLinkMeanY[0] < 10 || vLinkMeanY[0] > 245),
      nHits[1] > 0 && (vLinkMeanY[1] < 10 || vLinkMeanY[1] > 245),
      nHits[2] > 0 && (vLinkMeanY[2] < 10 || vLinkMeanY[2] > 245)
    };
    for (int i = 0; i < 3; ++i) {
      if (atEdge[i] && lkStatus[i] == calPixelQualityLM::Good) {
        lkStatus[i] = calPixelQualityLM::LVDSErrorTopBottomEdge;
      }
    }
  }

  links.push_back(lkStatus[0]);
  links.push_back(lkStatus[1]);
  links.push_back(lkStatus[2]);
  links.push_back(lkStatus[3]);

  return true;
}

// ----------------------------------------------------------------------
// -- old code
bool determineBrokenLinksV0(TH2 *h, vector<int> &links) {
  bool DBX(false);

  // -- the histogram still used to provide the chip name
  string schip = h->GetName();
  replaceAll(schip, "hitmap_perChip_", "");
  int chipID = ::stoi(schip);

  // -- determine dead chips
  double nLinkA = h->Integral(1, 88, 1, 250);
  double nLinkB = h->Integral(89, 172, 1, 250);
  double nLinkC = h->Integral(173, 256, 1, 250);
  vector<double> vLink = {nLinkA, nLinkB, nLinkC};
  double nLinkAverage = (nLinkA + nLinkB + nLinkC)/3.;
  if (1) cout << "  nLinkA = " << nLinkA << " nLinkB = " << nLinkB << " nLinkC = " << nLinkC << " nLinkAverage = " << nLinkAverage << endl;
  if (nLinkA < 1 && nLinkB < 1 && nLinkC < 1) {
    links.push_back(8);
    links.push_back(8);
    links.push_back(8);
    links.push_back(8);
    return false;
  }

  // -- search for pathological cases where all hits are very close to the top (bottom) edge
  vector<double> vLinkMeanY;
  vLinkMeanY.push_back(h->ProjectionY("A", 1, 88)->GetMean());
  vLinkMeanY.push_back(h->ProjectionY("B", 89, 172)->GetMean());
  vLinkMeanY.push_back(h->ProjectionY("C", 173, 256)->GetMean());
  int badChip(-1); // -- if any link is broken and not masked, the chip is bad
  if (nLinkA > 0 && (vLinkMeanY[0] < 10 || vLinkMeanY[0] > 245)) badChip = 1;
  if (nLinkB > 0 && (vLinkMeanY[1] < 10 || vLinkMeanY[1] > 245)) badChip = 2;
  if (nLinkC > 0 && (vLinkMeanY[2] < 10 || vLinkMeanY[2] > 245)) badChip = 3;
  if (badChip > 0) {
    cout << "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZ badChip = " << badChip << " vLinkMeanY = " << vLinkMeanY[0] << " " << vLinkMeanY[1] << " " << vLinkMeanY[2] << endl;  
    if (nLinkA < 1) {
      links.push_back(8);
    } else {
      links.push_back(3);
    }
    if (nLinkB < 1) {
      links.push_back(8);
    } else {
      links.push_back(3);
    }
    if (nLinkC < 1) {
      links.push_back(8);
    } else {
      links.push_back(3);
    }
    links.push_back(0);
    return true;
  }


  // -- hand-curated list to avoid getting lost in algorithmic nightmares
  vector<int> vBadChips = {102};

  if (find(vBadChips.begin(), vBadChips.end(), chipID) != vBadChips.end()) {  
    links.push_back(3);
    links.push_back(3);
    links.push_back(3);
    links.push_back(3);
    return true;
  }

  AsicInfo ai = gRunInfoMap[chipID];

  // -- completely disabled chip
  if (0 == ai.abcLinkMask[0]
      && 0 == ai.abcLinkMask[1]
      && 0 == ai.abcLinkMask[2]) {
    links.push_back(9);
    links.push_back(9);
    links.push_back(9);
    links.push_back(0);
    return false;
  }

  
  // -- check individual links. i = 0,1,2 is sensor A, B, C.
  int lkStatus[3] = {0, 0, 0};
  int nBadLinks(0);
  badChip = -1;
  static const char kSensor[3] = {'A', 'B', 'C'};
  for (int i = 0; i < 3; ++i) {
    int idx = -1;
    if (!ai.letterToIndex(kSensor[i], idx)) {
      lkStatus[i] = 4;
      cout << "Chip " << chipID << " sensor " << kSensor[i] << " status = " << lkStatus[i]
           << " unmapped" << endl;
      continue;
    }
    if (vLink[i] < 1) lkStatus[i] = 8;
    if (0 == ai.abcLinkMask[idx]) lkStatus[i] = 9;
    if (9 == ai.abcLinkMask[idx]) lkStatus[i] = 9;
    if (ai.abcLinkErrs[idx] > 10) lkStatus[i] = 4;
    if (ai.abcLinkErrs[idx] > 10 && 1 == ai.abcLinkMask[idx]) {
      badChip = i;
      ++nBadLinks;
    }
    cout << "Chip " << chipID << " sensor " << kSensor[i] << " status = " << lkStatus[i]
         << " idx = " << idx
         << " mask = " << ai.abcLinkMask[idx] << " errs = " << ai.abcLinkErrs[idx]
         << endl;
  }


  if (badChip >= 0) {    
    cout << "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZ badChip = " << badChip << " lkStatus = " << lkStatus[0] << " " << lkStatus[1] << " " << lkStatus[2] << endl;
    for (int i = 0; i < 3; ++i) {
      if (i != badChip) {
        lkStatus[i] = 5;
      } else {
        lkStatus[i] = 4;
      }
    }
  }

  // An all-E matrix maps no sensor letter. Those sensors are already status 4.

  links.push_back(lkStatus[0]);
  links.push_back(lkStatus[1]);
  links.push_back(lkStatus[2]);
  links.push_back(0);

  return true;
}


// ----------------------------------------------------------------------
void determineDeadColumns(TH2 *h, vector<int> &columns, vector<int> &links) {
  bool DBX(true);
  double minHits(1);
  double meanHits = h->Integral(1, 256, 1, 250)/h->GetNbinsX();
  if (meanHits < 10) {
    minHits = 0;
  } else {
    minHits = meanHits/10.;
    minHits = 1;
  }
  // -- the histogram still used to provide the chip name
  string schip = h->GetName();
  replaceAll(schip, "hitmap_perChip_", "");
  int chipID = ::stoi(schip);
 
  if (DBX) cout << "chipID = " << chipID << " meanHits = " << meanHits << " minHits = " << minHits << endl;
  bool linkA = links[0] > 0;
  bool linkB = links[1] > 0;
  bool linkC = links[2] > 0;
  // -- loop over bin indices, not column indices
  for (int ix = 1; ix <= h->GetNbinsX(); ++ix) {
    double nHits = h->Integral(ix, ix, 1, 250);

    if (nHits < minHits) {
      // -- first check if already contained in dead links  
      if (linkA > 0 && ix < 89) {
        continue;
      } 
      if (linkB > 0 && (ix >= 89 && ix < 173)) {
       continue;
      }
      if (linkC > 0 && ix >= 173) {
        continue;
      }
      columns.push_back(ix-1);
      columns.push_back(8); 
      if (DBX) {
        cout << "determineDeadColumns> ix = " << ix << " pushed! nHits = " << nHits
             << " minHits = " << minHits << " deadlinks = ";
        for (auto itL : links) {
          cout << itL << " ";
        }
        cout << endl; 
      }

    }
  }

}

// ----------------------------------------------------------------------
void determineNoisyPixels(TH2 *h, vector<int> &noisyPixels, vector<int> &columns, vector<int> &links) {
  bool DBX(false);
  bool DODEADPIX(false);

  // -- the histogram still used to provide the chip name
  string schip = h->GetName();
  replaceAll(schip, "hitmap_perChip_", "");
  int chipID = ::stoi(schip);

  int chipCnt = h->GetSumOfWeights();
  int npix(0);
  for (int ix = 1; ix <= h->GetNbinsX(); ++ix) {
    for (int iy = 1; iy <= h->GetNbinsY(); ++iy) {
      if (h->GetBinContent(ix,iy) > 0) ++npix;
    }
  }
  if (DBX) cout << "chipID = " << h->GetName() << " chipCnt =  " << chipCnt << " for npix = " << npix << endl;
  if (npix > 0) {
    // -- noise threshold to be 10 x sigma above the mean hit number per pixel
    double NSIGMA(10.0);
    // -- if more than SUSPECTFRACTION of the pixel in a column are noisy, flag the entire column as suspect
    double SUSPECTMAXIMUM(40);
    double meanHits  = static_cast<double>(chipCnt)/npix;
    // -- this is WRONG and biased by noisy pixels
    double meanHitsE = TMath::Sqrt(meanHits);
    double noiseThr  = meanHits + NSIGMA*meanHitsE;
    double noiseThr2  = meanHits - NSIGMA*meanHitsE;
    if (noiseThr2 < 0.) noiseThr2 = 1.;
    if (meanHits > 0.) {
      if (DBX) cout << "meanHits = " << meanHits << " +/- " << meanHitsE << " noise threshold = " << noiseThr << endl;
    }
    int nNoisyPix(0);
    int nDeadPix(0);
    int colNoisyPixels(0);
    for (int ix = 1; ix <= h->GetNbinsX(); ++ix) {
      colNoisyPixels = 0;
      // -- skip defective links 
      if (links[0] > 0 && ix < 89) {
        continue;
      }
      if (links[1] > 0 && (ix >= 89 && ix < 173)) { 
        continue;
      }
      if (links[2] > 0 && ix >= 173) {
        continue;
      }
      for (int iy = 1; iy <= h->GetNbinsY(); ++iy) {

        double nhits = h->GetBinContent(ix,iy);
        if (nhits > noiseThr) {
          if (DBX) cout << "chipID = " << chipID << " noisy pixel at icol/irow = " << ix-1 << "/" << iy-1  << " nhits = " << nhits << " noiseThr = " << noiseThr << endl;
          ++nNoisyPix;
          ++colNoisyPixels;
          noisyPixels.push_back(ix-1);
          noisyPixels.push_back(iy-1);
          noisyPixels.push_back(1);  
          if (DBX) {
            cout << "  noisy pixel at icol/irow = " << ix-1 << "/" << iy-1  << " nhits = " << nhits << endl;
          }
        }
        if (DODEADPIX && (nhits < noiseThr2)) {
          // -- check if pixel is in dead column
          if (find(columns.begin(), columns.end(), ix-1) != columns.end()) {
            continue;
          }
          // -- check if pixel is in dead link
          if (links[0] > 0 && ix < 89) {
            continue;
          }
          if (links[1] > 0 && (ix >= 89 && ix < 173)) { 
            continue;
          }
          if (links[2] > 0 && ix >= 173) {
            continue;
          }
          ++nDeadPix;
          noisyPixels.push_back(ix-1);
          noisyPixels.push_back(iy-1);
          noisyPixels.push_back(9);  
          if (DBX) {
            cout << "  dead pixel at icol/irow = " << ix-1 << "/" << iy-1  << " nhits = " << nhits << endl;
          }
        }
      }
      if (DBX) cout << "col = " << ix-1 << "  colNoisyPixels = " << colNoisyPixels << " SUSPECTMAXIMUM = " << SUSPECTMAXIMUM << endl;
      if (colNoisyPixels > SUSPECTMAXIMUM) {
        for (int i = 0; i < colNoisyPixels; ++i) {
          // -- remove the noisy pixel col,row,iqual
          noisyPixels.pop_back(); 
          noisyPixels.pop_back();
          noisyPixels.pop_back();
        }

        // -- possibly out of order!
        columns.push_back(ix-1);
        columns.push_back(2);

        if (DBX) {
          cout << "  -> colNoisyPixels > SUSPECTMAXIMUM -> removing " << colNoisyPixels << " noisy pixels, deadcolumn: " << endl;
          for (auto it00: columns) {
            cout << it00 << " ";
          }
          cout << endl;
        }
      }

    }
    if (DBX) cout << "  -> nNoisyPixel = " << nNoisyPix << " nDeadPix = " << nDeadPix << endl;
  }
}


// ----------------------------------------------------------------------
static bool fileExists(const string& filename) {
  ifstream in(filename.c_str());
  return in.good();
}

// A link above this LVDS error rate [Hz] is bad.
static const long long kLvdsBadHz = 10;

// ----------------------------------------------------------------------
// Section names such as "L1 US 1-4" and "L2 DS 8-10".
// "Central US Layer 3" does not match; that is a one-link section.
static bool isL12Section(const string& name) {
  return name.find("L1") != string::npos
      || name.find("L2") != string::npos
      || name.find("L3") != string::npos;
}

static bool isLayer3Section(const string& name) {
  return name.find("Layer 3") != string::npos;
}

// ----------------------------------------------------------------------
// The JSON stores linkMask, linkMatrix and lvdsErrRates left to right.
// abcLink*[link] and lvdsErrRate* are filled in right-hand order:
// link 0 is the rightmost entry. Which sensor that is comes from
// AsicInfo::letterToIndex / indexToLetter, not from the slot number.
// abcLinkMatrix: 0=A, 1=B, 2=C, 4=E (or missing).
static void decodeLinksRhs(AsicInfo& ai, const vector<long long>& rates) {
  const int n = 3;
  for (int link = 0; link < n; ++link) {
    int src = n - 1 - link;
    ai.abcLinkMask[link] = 0;
    ai.abcLinkErrs[link] = (src < (int)rates.size() ? rates[src] : 0);
    ai.abcLinkMatrix[link] = 4;
    ai.abcLinkNames[link] = "E";
    ai.abcLinkOffsets[link] = link;
    if ((int)ai.linkMask.size() > src) {
      char c = ai.linkMask[src];
      if (c >= '0' && c <= '9') ai.abcLinkMask[link] = c - '0';
    }
    if ((int)ai.linkMatrix.size() > src) {
      char c = ai.linkMatrix[src];
      ai.abcLinkNames[link] = string(1, c);
      if (c == 'E') ai.abcLinkMatrix[link] = 4;
      else if (c >= 'A' && c <= 'C') ai.abcLinkMatrix[link] = c - 'A';
    }
  }
  ai.lvdsErrRate0 = ai.abcLinkErrs[0];
  ai.lvdsErrRate1 = ai.abcLinkErrs[1];
  ai.lvdsErrRate2 = ai.abcLinkErrs[2];
}

// ----------------------------------------------------------------------
static bool linkIsBad(const AsicInfo& ai, int link) {
  return ai.abcLinkMatrix[link] == 4 || ai.abcLinkErrs[link] > kLvdsBadHz;
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
// One physical link, stored in slot 0. Slots 1 and 2 are absent (mask 9),
// so they are not counted as the three-link 'E' case.
static void setSingleLink(AsicInfo& ai, int mask, char matrix, long long rate) {
  ai.linkMask = to_string(mask);
  ai.linkMatrix = string(1, matrix);
  ai.lvdsErrRate0 = rate;
  ai.lvdsErrRate1 = 0;
  ai.lvdsErrRate2 = 0;
  ai.abcLinkMask[0] = mask;
  ai.abcLinkErrs[0] = rate;
  if (matrix == 'E') ai.abcLinkMatrix[0] = 4;
  else if (matrix >= 'A' && matrix <= 'C') ai.abcLinkMatrix[0] = matrix - 'A';
  else ai.abcLinkMatrix[0] = 4;
  for (int link = 1; link < 3; ++link) {
    ai.abcLinkMask[link] = 9;
    ai.abcLinkErrs[link] = 0;
    ai.abcLinkMatrix[link] = -1;
  }
}

// mask / matrix / lvds_error_rate, as on a Layer 3 links[] entry.
static bool readSingleLink(const json& src, int& mask, char& matrix, long long& rate) {
  bool any = false;
  if (src.contains("mask") && src["mask"].is_number()) {
    mask = src["mask"].get<int>();
    any = true;
  }
  if (src.contains("matrix") && src["matrix"].is_string()) {
    string s = src["matrix"].get<string>();
    if (!s.empty()) matrix = s[0];
    any = true;
  }
  if (src.contains("lvds_error_rate") && src["lvds_error_rate"].is_number()) {
    rate = src["lvds_error_rate"].get<long long>();
    any = true;
  }
  return any;
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
      const auto& asics = section["asics"];
      const json* links = (section.contains("links") && section["links"].is_array())
          ? &section["links"] : nullptr;
      for (size_t i = 0; i < asics.size(); ++i) {
        const auto& a = asics[i];
        int gid = a.value("globalId", 0);
        if (gid <= 0) {
          ++nLayer3Skip;
          continue;
        }
        int mask = 0;
        char matrix = 'E';
        long long rate = 0;
        bool have = readSingleLink(a, mask, matrix, rate);
        if (!have && links && i < links->size()) {
          have = readSingleLink((*links)[i], mask, matrix, rate);
        }
        if (!have) {
          cout << "parseMidasMetaData> WARNING: " << secName
               << " globalId=" << gid << " has no single-link fields" << endl;
          continue;
        }
        AsicInfo ai;
        ai.FEBName = secName;
        ai.fedID = section.value("index", -1);
        ai.confId = a.value("confId", 0);
        ai.globalId = gid;
        setSingleLink(ai, mask, matrix, rate);
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
