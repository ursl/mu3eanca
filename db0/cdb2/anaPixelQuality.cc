#include <Rtypes.h>
#include <TAttMarker.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <set>
#include <cmath>
#include <TFile.h>
#include <TTree.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TLatex.h>
#include <TH1.h>
#include <TH2.h>
#include <TLine.h>
#include <string.h>
#include <unordered_map>
#include <sstream>

#include "cdbRest.hh"
#include "cdbJSON.hh"
#include "runRecord.hh"


#include "Mu3eConditions.hh"

#include "calPixelQualityLM.hh"

#include "util.hh"

using namespace std;

// ----------------------------------------------------------------------
// anaPixelQuality [-v] [-f firstRun] [-l lastRun] [-o FILENAME] [-p]
// ----------------
//
// Analyze pixel quality data
//
// -v          verbose output
// -o1 X -o2 Y overlay histograms in files X and Y
// -s  SUFFIX  output filename suffix
// -f first    runnumber
// -l last     runnumber
// -p          plot only (i.e. no reading of all the payloads)
// -r FILENAME runlist file
// -R r1,r2    comma-separated list of runs
//
// Examples:
// 
// ----------------------------------------------------------------------

std::unordered_map<int, int> gRunNumberIndex;

// ----------------------------------------------------------------------
struct runQuality {
  int runNumber;
  int nNoisyPixels;
  int nGoodPixels;
  int nDeadLinks;
  int nBadLinks;
  int nEEE;
  int nActiveChips;
  int nStatus5;
};


int chipIndex(unsigned int chipid);
int runIndex(int runnumber);
int linkStatus(calPixelQualityLM *pPQ, unsigned int chipid);


void writeRunList(string filename, vector<int> &vRuns);
void plotHistograms(string filename, string suffix);
void setRunLabelsY(TH2D *h);
void setRunLabelsX(TH1D *h);
void overlayHistograms(string filename1, string filename2);

int gNruns(0);

// ----------------------------------------------------------------------
int main(int argc, char *argv[]) {

  // -- command line arguments
  string filename("anaPixelQuality.root");
  // -- datav7.1=2026V2 does not include the mask files, much faster
  string gt("datav7.1=2026V2");
  string db("/Users/ursl/data/mu3e/cdb");
  string metaMidasTreeFile("");
  int first(0), last(0);
  int verbose(0);
  int plot(0);
  string srunfile("");
  string srunlist("");
  string suffix("");
  string filename1("");
  string filename2("");
  string payloadDir("");
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-d"))     {payloadDir = argv[++i];}
    if (!strcmp(argv[i], "-db"))    {db = argv[++i];}  
    if (!strcmp(argv[i], "-f"))     {first = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-g"))     {gt = argv[++i];}
    if (!strcmp(argv[i], "-l"))     {last = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-m"))     {metaMidasTreeFile = argv[++i];}
    if (!strcmp(argv[i], "-o1"))    {filename1 = argv[++i];}
    if (!strcmp(argv[i], "-o2"))    {filename2 = argv[++i];}
    if (!strcmp(argv[i], "-p"))     {plot = 1;}
    if (!strcmp(argv[i], "-r"))     {srunfile = argv[++i];}
    if (!strcmp(argv[i], "-R"))     {srunlist = argv[++i];}
    if (!strcmp(argv[i], "-s"))     {suffix = argv[++i];}
    if (!strcmp(argv[i], "-v"))     {verbose = 1;}
  }
  
  if (suffix != "") {
    suffix = "-" + suffix;
    filename = string("anaPixelQuality") + suffix + ".root";
    cout << "Suffix: " << suffix << endl;
    cout << "Filename: " << filename << endl;
  } else {
    filename = string("anaPixelQuality.root");
  }

  if ("" != filename1 && "" != filename2) {
    overlayHistograms(filename1, filename2);
    return 0;
  } 


  // -- fill vRuns from sruns
  vector<int> vRuns;
  if (srunfile != "") {
    std::ifstream infile(srunfile);
    if (!infile) {
        std::cerr << "Could not open file: " << srunfile << std::endl;
        // handle error as needed
    }
    std::string line, content;
    while (std::getline(infile, line)) {
        content += line;
    }
    infile.close();

    // Remove curly braces
    size_t start = content.find('{');
    size_t end = content.find('}');
    if (start != std::string::npos && end != std::string::npos && end > start) {
        content = content.substr(start + 1, end - start - 1);
    }

    std::stringstream ss(content);
    std::string token;
    while (std::getline(ss, token, ',')) {
        // Remove leading/trailing whitespace
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);

        if (!token.empty()) {
          int run = std::stoi(token); 
          if (run >= first && run <= last) {
            vRuns.push_back(run);
          }
        }
    }
  }

  if (srunlist != "") {
    stringstream ss(srunlist);
    string token;
    while (std::getline(ss, token, ',')) {
      vRuns.push_back(std::stoi(token));
    }
  }

  if (vRuns.size() > 0) {
    cout << "Runs: ";
    for (auto it: vRuns) cout << it << " ";
    cout << endl;
  }


  cdbAbs *pDB(0);
  if (string::npos != db.find("rest") || string::npos != db.find("http://")) {
    string ms("http://mu3edb0.psi.ch/cdb");
    pDB = new cdbRest(ms, verbose);
  } else {
    // -- hope for the best that this is a JSON directory
    cout << "Using JSON directory: " << db << endl;
    pDB = new cdbJSON(db, verbose);
  }

  // -- initialize the conditions
  Mu3eConditions *pDC = Mu3eConditions::instance(gt, pDB);
  pDC->setVerbosity(verbose);
  vector<string> vTags = pDC->getTags();
  // -- keep the definition of the run ranges from the CDB
  vector<int> vIoV;
  bool foundPQ(false);
  for (auto it: vTags) {
    if (string::npos != it.find("pixelqualitylm_")) {
      foundPQ = true;
      vIoV = pDC->getIOVs(it);
      cout << "Tag " << it << " has " << vIoV.size() << " IOVs: ";
      for (auto itIoV: vIoV)  cout << itIoV << " ";
      cout << endl;
    }
  }
  if (!foundPQ) {
    cout << "ERROR: no pixel quality data found" << endl;
    return 0;
  }

  // -- get the number of runs
  int minRun(-1);
  int maxRun(-1);

  if (first > 0) {
    minRun = first;
    // -- in terms of run index
    if (find(vIoV.begin(), vIoV.end(), minRun) == vIoV.end()) {
      cout << "Run " << minRun << " not found in vIoV" << endl;
      for (auto it: vIoV) {
        if (it < minRun) {
        } else {
          minRun = it;
          break;
        }
      }
    }
  }

  if (last > 0) {
    maxRun = last;
    // -- in terms of run index
    if (find(vIoV.begin(), vIoV.end(), maxRun) == vIoV.end()) {
      cout << "Run " << maxRun << " not found in vIoV" << endl;
      for (auto it: vIoV) {
        if (it > maxRun) {
        } else {
          maxRun = it;
          break;
        }
      }
    }
  }

  int nRuns = std::distance(vIoV.begin(), std::find(vIoV.begin(), vIoV.end(), maxRun)) 
            - std::distance(vIoV.begin(), std::find(vIoV.begin(), vIoV.end(), minRun)) + 1;
  gNruns = nRuns;

  cout << "Number of runs: " << nRuns 
       << " from " << minRun << " to " << maxRun
       << endl;


  // -- initialize the run number index
  int cnt(0);
  for (unsigned int i = 0; i < vIoV.size(); i++) {
    if (first > 0 && vIoV[i] < first) {
      continue;
    }
    if (last > 0 && vIoV[i] > last) {
      continue;
    }

    gRunNumberIndex[vIoV[i]] = cnt;
    ++cnt;
  }

  // -- check if we should plot only. This MUST BE after reading the CDB because else the IOVs are not known!
  if (plot) {
    plotHistograms(filename, suffix);
    return 0;
  }

  // ---------------------------------------------------------------------------------------------------
  // -- Proper start of analysis of payloads
  // ---------------------------------------------------------------------------------------------------

  // -- get pixel quality interface
  calPixelQualityLM *pPQ = (calPixelQualityLM*)pDC->getCalibration("pixelqualitylm_");
  if (pPQ) {
    cout << "pixelqualitylm_ found" << endl;
    pPQ->setVerbosity(verbose);
  } else {
    cout << "pixelqualitylm_ not found" << endl;
    return 0;
  }


  // -- Plot stuff for each run
  TFile *pFile = new TFile(filename.c_str(), "RECREATE");

  // -- Number chips
  const int nChips = 516;

  // -- Working links per chip
  TH2D *rLinkStatus = new TH2D("rLinkStatus", "Working links per chip", nChips, 0, nChips, nRuns, 0, nRuns);
  setTitles(rLinkStatus , "Chip index", "Run", 0.05, 1.1, 1.2);
  rLinkStatus->GetZaxis()->SetTitle("Working links"); rLinkStatus->SetTitleOffset(1.2, "z");
  rLinkStatus->SetMaximum(3); // simplest way to get reasonable color scheme
  TH1D *hLinkStatus = new TH1D("hLinkStatus", "Working links (Perfect: 3x108+408 = 728)", 129, 0., 3*108+408.);
  setTitles(hLinkStatus, "Working links", "Number of runs", 0.05, 1.0, 1.2, 0.04);

  // -- Number of noisy pixels per run
  TH1D *rNoisyPixels = new TH1D("rNoisyPixels", "Total number of noisy pixels per run", nRuns, 0, nRuns);
  setTitles(rNoisyPixels, "Run", "Noisy pixels", 0.05, 1.0, 1.2);
  TH1D *hNoisyPixels = new TH1D("hNoisyPixels", "Noisy pixels per run", 100, 0, 100000.);
  setTitles(hNoisyPixels, "Noisy pixels", "Number of runs", 0.05, 1.0, 1.2, 0.04);

  // -- Number of masked/LVDS error pixels per run
  TH1D *rErrorPixels = new TH1D("rErrorPixels", "Total number of masked/LVDS error pixels per run", nRuns, 0, nRuns);
  setTitles(rErrorPixels, "Run", "Masked/LVDS error pixels", 0.05, 1.0, 1.2);
  TH1D *hErrorPixels = new TH1D("hErrorPixels", "Masked/LVDS error pixels per run", 100, 0, 4.e7);
  setTitles(hErrorPixels, "Masked/LVDS error pixels", "Number of runs", 0.05, 1.0, 1.2, 0.04);


  // -- Number of noisy pixels per chip
  TH1D *cNoisyPixels = new TH1D("cNoisyPixels", "Total number of noisy pixels per chip", 100, 0, 2000.);
  setTitles(cNoisyPixels, "Noisy pixels", "Number of chips", 0.05, 1.0, 1.2, 0.04);

  TH1D *rEEE = new TH1D("rEEE", "Chips without good links", nRuns, 0, nRuns);
  setTitles(rEEE, "Run", "Chips without good links", 0.05, 1.0, 1.2);
  TH1D *hEEE = new TH1D("hEEE", "Chips without good links", 328, 0., 328.);
  setTitles(hEEE, "Chips without good links", "Number of runs", 0.05, 1.0, 1.2, 0.04);

  // -- this is only for debugging (e.g. if some illegal chipIDs sneaked in)
  TH1D *rGoodPixel = new TH1D("rGoodPixel", "Total number of good pixels per run", nRuns, 0, nRuns);
  setTitles(rGoodPixel, "Run", "Good pixels", 0.05, 1.0, 1.2);
  TH1D *hGoodPixel = new TH1D("hGoodPixel", "Good pixels per run", 70, 0, 4.e7);
  setTitles(hGoodPixel, "Good pixels", "Number of runs", 0.05, 1.0, 1.2, 0.04);

  // -- Size of JSON payload
  TH1D *rPayloadSize = new TH1D("rPayloadSize", "Size of JSON payload", nRuns, 0, nRuns);
  setTitles(rPayloadSize, "Run", "Size of JSON payload [kB]", 0.05, 1.0, 1.2, 0.04);
  TH1D *hPayloadSize = new TH1D("hPayloadSize", "Size of JSON payload [kB]", 50, 0, 1.e6);
  setTitles(hPayloadSize, "Size of JSON payload [kB]", "Number of runs", 0.05, 1.0, 1.2, 0.04);

  vector<int> vAllBadLinkRuns, vGoodPixelRuns, vGoodRuns;

  // -- write the run index to a file
  ofstream OS(filename + "-runIdx.txt");
  for (auto itIoV: vIoV) {
    if (vRuns.size() > 0 && find(vRuns.begin(), vRuns.end(), itIoV) == vRuns.end()) {
      continue;
    }
    if (first > 0 && itIoV < first) {
      continue;
    }
    if (last > 0 && itIoV > last) {
      continue;
    }
    OS << "runIdx = " << runIndex(itIoV)+1 << " " << itIoV << endl;
  }
  OS.close();
  
  // -- write out chip index to a file
  ofstream OC(filename + "-chipIdx.txt");
  pDC->setRunNumber(minRun);
  uint32_t cid(0);
  pPQ->resetIterator();
  while (pPQ->getNextID(cid)) {
    OC << "chipIdx = " << chipIndex(cid)+1 << " " << cid << endl;
  }
  OC.close();


  // ---------------------------------------------------------------------------------------------------
  // -- loop over all runs
  vector<int> vRunsWithZeroWorkingLinks, vRunsWithZeroNoisyPixels;

  for (auto itIoV: vIoV) {
    if (vRuns.size() > 0 && find(vRuns.begin(), vRuns.end(), itIoV) == vRuns.end()) {
      continue;
    }
    if (first > 0 && itIoV < first) {
      continue;
    }
    if (last > 0 && itIoV > last) {
      continue;
    }
    if (payloadDir == "") {
      cout << "Reading payload via CDB" << endl;
      pDC->setRunNumber(itIoV);
    } else {
      string hash = "tag_pixelqualitylm_" + gt + "_iov_" + to_string(itIoV);
      cout << "Reading payload from " << payloadDir << "/" << hash << endl;
      pPQ->readPayloadFromFile(hash, payloadDir);
      pPQ->calculate(hash);
    }

    //cout << " Working links: "; 
    int nWorkingLinks(0);
    int runIdx = runIndex(itIoV)+1;


    runRecord rr = pDC->getRunRecord(itIoV);
    rr.print();
    string runclass = rr.fBORRunClass;
    int qVertex = rr.fvDQ[rr.fDataQualityIdx].vertex;
    bool okVertex = (qVertex != DQ_UNSET);
    okVertex = okVertex && (qVertex != DQ_NODATA);

    int qPixel  = rr.fvDQ[rr.fDataQualityIdx].pixel;
    bool okPixel = (qPixel != DQ_UNSET);
    okPixel = okPixel && (qPixel != DQ_NODATA);
    cout << "Run " << itIoV << " (index = " << runIdx << ") has runclass = " << runclass 
         << " and qVertex = " << qVertex 
         << " and qPixel = " << qPixel 
         << endl;
    // -- loop over all chips
    uint32_t chipid(0);
    int nNoisyPixels(0);
    int nErrorPixels(0);
    int nGoodPixels(0);
    int nChips(0);
    pPQ->resetIterator();
    while (pPQ->getNextID(chipid)) {
      int lStatus = linkStatus(pPQ, chipid);
      nWorkingLinks += lStatus;
      int chipIdx = chipIndex(chipid)+1;
      rLinkStatus->Fill(chipIdx, runIdx, lStatus);

      int chipNoisy = pPQ->getNpixWithStatus(chipid, calPixelQualityLM::Noisy); 
      int chipGood = pPQ->getNpixWithStatus(chipid, calPixelQualityLM::Good);
      int chipError = pPQ->getNpixWithStatus(chipid, calPixelQualityLM::LVDSErrorLink);
      chipError += pPQ->getNpixWithStatus(chipid, calPixelQualityLM::LVDSErrorOtherLink);
      chipError += pPQ->getNpixWithStatus(chipid, calPixelQualityLM::LVDSErrorTopBottomEdge);
      chipError += pPQ->getNpixWithStatus(chipid, calPixelQualityLM::Masked);
      cout << "Run " << itIoV << " (index = " << runIdx << ") has chip " << chipid 
           << " with " << chipNoisy << " noisy pixels and " 
           << chipError << " masked/LVDS error pixels and "
           << chipGood << " good pixels" 
           << endl;
      nNoisyPixels += chipNoisy;
      nGoodPixels += chipGood;
      nErrorPixels += chipError;
      // -- count noisy pixels  
      //    - for all chips that are not completely dead
      //    - runs where vtx/pixel data are present
      if (!pPQ->isChipDead(chipid) && okVertex && okPixel) {
        double xNoisy = chipNoisy;
        const TAxis *ax = cNoisyPixels->GetXaxis();
        if (xNoisy >= ax->GetXmax()) {
          xNoisy = ax->GetBinCenter(cNoisyPixels->GetNbinsX());
        }
        cNoisyPixels->Fill(xNoisy);
    
      }
      ++nChips;
    }
    if (nWorkingLinks == 0) {
      vRunsWithZeroWorkingLinks.push_back(itIoV);
    } 


    // -- fill the histograms per run
    hLinkStatus->Fill(nWorkingLinks);
    // -- fill the histograms per run only if there are working links
    if (nNoisyPixels == 0) {
      cout << "Run " << itIoV << " (index = " << runIdx << ") has no noisy pixels" 
           << " and nGoodPixels = " << nGoodPixels
           << " and nWorkingLinks = " << nWorkingLinks
           << " and is " << (okVertex ? "vertex" : "no vertex")
           << " and is " << (okPixel ? "pixel" : "no pixel")
           << "*****************************************************"
      << endl;
    }
    hErrorPixels->Fill(nErrorPixels);
    rErrorPixels->Fill(runIdx, nErrorPixels);

    if (nWorkingLinks > 0 && okVertex && okPixel) {
      cout << "  filling nNoisyPixels = " << nNoisyPixels << endl;
      hNoisyPixels->Fill(nNoisyPixels);
      hGoodPixel->Fill(nGoodPixels);
      rNoisyPixels->Fill(runIdx, nNoisyPixels);
      rGoodPixel->Fill(runIdx, nGoodPixels);
      if (nNoisyPixels == 0) {
        vRunsWithZeroNoisyPixels.push_back(itIoV);
      }
    }

    // -- get the size of the JSON payload via size of BLOB
    string sblob = pPQ->makeBLOB();
    rPayloadSize->Fill(runIdx, sblob.size()/1024.);
    hPayloadSize->Fill(sblob.size()/1024.);

    cout << "Run " << itIoV << " (index = " << runIdx << ") has " 
         << nChips << " chips, "
         << nNoisyPixels << " noisy pixels and " 
         << nGoodPixels << " good pixels" 
         << " and " << nWorkingLinks << " working links"
         << endl;
  }
   // -- save the histograms 
  pFile->Write();
  pFile->Close();

  plotHistograms(filename, suffix);

  cout << "Bad run list (all bad links) has " << vAllBadLinkRuns.size() << " runs" << endl;
  string sfilename = "badLinksTooManyRunList" + suffix + ".txt";
  writeRunList(sfilename, vAllBadLinkRuns);

  cout << "BAD pixel run list (less than 1M good pixels) has " << vGoodPixelRuns.size() << " runs" << endl;
  sfilename = "goodPixelTooFewRunList" + suffix + ".txt";
  writeRunList(sfilename, vGoodPixelRuns);

  set<int> sBadRuns;
  sBadRuns.insert(vAllBadLinkRuns.begin(), vAllBadLinkRuns.end());
  sBadRuns.insert(vGoodPixelRuns.begin(), vGoodPixelRuns.end());
  cout << "Bad run list (too many  bad links, and too few good pixel) has " << sBadRuns.size() << " runs" << endl;
  sfilename = "badRunsList" + suffix + ".txt";
  for (auto it: sBadRuns) {
        cout << it << ",";
  }
  cout << endl;

  cout << "Runs with zero working links has " << vRunsWithZeroWorkingLinks.size() << " runs" << endl;
  sfilename = "zeroWorkingLinksRunList" + suffix + ".txt";
  writeRunList(sfilename, vRunsWithZeroWorkingLinks);
  sfilename = "zeroNoisyPixelsRunList" + suffix + ".txt";
  writeRunList(sfilename, vRunsWithZeroNoisyPixels);

  for (auto itIoV: vIoV) {
    if (vRuns.size() > 0 && find(vRuns.begin(), vRuns.end(), itIoV) == vRuns.end()) {
      continue;
    }
    if (first > 0 && itIoV < first) {
      continue;
    }
    if (last > 0 && itIoV > last) {
      continue;
    }
    if (find(sBadRuns.begin(), sBadRuns.end(), itIoV) != sBadRuns.end()) {
      continue;
    }
    vGoodRuns.push_back(itIoV);
  }
  cout << "Good run list has " << vGoodRuns.size() << " runs" << endl;
  sfilename = "goodRunsList" + suffix + ".txt";
  writeRunList(sfilename, vGoodRuns);



  return 0;
}


// ----------------------------------------------------------------------
void writeRunList(string filename, vector<int> &vRuns) {
  ofstream outfile(filename);
  outfile << "{";     
  for (size_t i = 0; i < vRuns.size(); ++i) {
    cout << vRuns[i] << " ";
    outfile << vRuns[i];
    if (i < vRuns.size() - 1) {
      outfile << ",";
    }
  }
  outfile << "}" << endl;
  outfile.close();
  cout << endl;
}

// ----------------------------------------------------------------------
int runIndex(int runnumber) {
  if (0) cout << "runIndex(" << runnumber << ") = " << gRunNumberIndex[runnumber] << endl;
  return gRunNumberIndex[runnumber];
}


// ----------------------------------------------------------------------
int indexRun(int index) {
  for (auto it: gRunNumberIndex) {
    if (it.second == index) {
      return it.first;
    }
  }
  return -1;
}



// ----------------------------------------------------------------------
// -- map chip ID to index in the vector of chip IDs
int chipIndex(unsigned int chipid) {
  static int first(1); 
  static std::unordered_map<int, int> chipID_to_index;
  if (first) {
    vector<int> vChipIDs = {1,2,3,4,5,6,
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
              // -- layer 3
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

    for (size_t i = 0; i < vChipIDs.size(); ++i) {
      chipID_to_index[vChipIDs[i]] = i;
    }
    first = 0;
  }
  return chipID_to_index[chipid];
}


// ----------------------------------------------------------------------
// -- map chip index to chipID
int indexChip(int index) {
  static int first(1); 
  static std::unordered_map<int, int> index_to_chipID;
  if (first) {
    vector<int> vChipIDs = {1,2,3,4,5,6,
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
              // -- layer 3
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

    for (size_t i = 0; i < vChipIDs.size(); ++i) {
      index_to_chipID[i] = vChipIDs[i];
    }
    first = 0;
  }
  return index_to_chipID[index];
}

// ----------------------------------------------------------------------
// -- number of working links in chip
int linkStatus(calPixelQualityLM *pPQ, unsigned int chipid) {
  int result(0);
  if (pPQ->getLinkStatus(chipid, 0) == 0) ++result;
  // -- layer 3 has only one link
  if (chipid >= 2055) return result;
  if (pPQ->getLinkStatus(chipid, 1) == 0) ++result;
  if (pPQ->getLinkStatus(chipid, 2) == 0) ++result;
  return result;
}

// ----------------------------------------------------------------------
// -- plot the histograms
void plotHistograms(string filename, string suffix) {
  const int nChips = 516;

  // -- open the file
  TFile *pFile = new TFile(filename.c_str());

  // -- by default pdf, except for hard-coded pngs (2D plots)
  string format(".pdf");
 
  // -- plot the histograms
  TCanvas *c1 = new TCanvas("c1", "c1", 3000, 2000);
  shrinkPad(0.12, 0.15, 0.15);

  TLatex *t1 = new TLatex();
  t1->SetTextColor(kBlack);
  t1->SetTextFont(42);
  t1->SetTextSize(0.04);

  // -- plot the link status
  TH2D *rLinkStatus  = (TH2D*)pFile->Get("rLinkStatus");
  cout << " plotting rLinkStatus" << endl;
  rLinkStatus->SetStats(0);
  rLinkStatus->Draw("colz");
  setRunLabelsY(rLinkStatus);
  // -- png mandatory because else pdf aliasing make single-bin and double-bin columns very similar
  c1->SaveAs(("rLinkStatus" + suffix + ".png").c_str());

  c1->Clear();
  c1->SetRightMargin(0.1);
  c1->SetBottomMargin(0.15);

  shrinkPad(0.15, 0.15, 0.1);
  TH1D *hLinkStatus = (TH1D*)pFile->Get("hLinkStatus");
  cout << " plotting hLinkStatus" << endl;
  hLinkStatus->SetStats(0);
  hLinkStatus->Draw("hist");
  hLinkStatus->SetTitle("");

  t1->DrawLatexNDC(0.7, 0.86, Form("N_{ runs}^{ total} = %d", gNruns));
  t1->DrawLatex(20., hLinkStatus->GetBinContent(1), Form("N_{ runs}^{ 0} = %d", static_cast<int>(hLinkStatus->GetBinContent(1))));
  
  t1->SetTextSize(0.03);
  t1->DrawLatexNDC(0.15, 0.915, "(Perfect: 3x108+408 = 732)");
  t1->SetTextSize(0.04);
  
  c1->SaveAs(("hLinkStatus" + suffix + format).c_str());


  TH1D *rNoisyPixels = (TH1D*)pFile->Get("rNoisyPixels");
  cout << " plotting rNoisyPixels" << endl;
  setRunLabelsX(rNoisyPixels);
  rNoisyPixels->SetMinimum(0.5);
  rNoisyPixels->SetStats(0);
  rNoisyPixels->GetXaxis()->SetTitleOffset(1.3);
  c1->SetLogy(1);
  rNoisyPixels->Draw("hist");
  c1->SaveAs(("rNoisyPixels" + suffix + format).c_str());

  TH1D *hNoisyPixels = (TH1D*)pFile->Get("hNoisyPixels");
  cout << " plotting hNoisyPixels" << endl;
  hNoisyPixels->SetStats(0);
  hNoisyPixels->Draw("hist");
  c1->SaveAs(("hNoisyPixels" + suffix + format).c_str());

  TH1D *cNoisyPixels = (TH1D*)pFile->Get("cNoisyPixels");
  cout << " plotting cNoisyPixels" << endl;
  cNoisyPixels->SetStats(11111);
  cNoisyPixels->Draw("hist");
  c1->SaveAs(("cNoisyPixels" + suffix + format).c_str());


  TH1D *rErrorPixels = (TH1D*)pFile->Get("rErrorPixels");
  cout << " plotting rErrorPixels" << endl;
  setRunLabelsX(rErrorPixels);
  rErrorPixels->SetMinimum(0.5);
  rErrorPixels->SetStats(0);
  rErrorPixels->GetXaxis()->SetTitleOffset(1.3);
  c1->SetLogy(1);
  rErrorPixels->Draw("hist");
  c1->SaveAs(("rErrorPixels" + suffix + format).c_str());

  TH1D *hErrorPixels = (TH1D*)pFile->Get("hErrorPixels");
  cout << " plotting hErrorPixels" << endl;
  hErrorPixels->SetStats(0);
  hErrorPixels->Draw("hist");
  t1->DrawLatexNDC(0.2, 0.86, Form("<N_{error}> = %d", static_cast<int>(hErrorPixels->GetMean())));
  t1->DrawLatexNDC(0.2, 0.81, Form("f = %4.3f", hErrorPixels->GetMean()/nChips/64000.));

  c1->SaveAs(("hErrorPixels" + suffix + format).c_str());




  TH1D *rGoodPixel = (TH1D*)pFile->Get("rGoodPixel");
  cout << " plotting rGoodPixel" << endl;
  setRunLabelsX(rGoodPixel);
  rGoodPixel->SetMinimum(1.e5);
  rGoodPixel->SetMaximum(1.5*nChips*64000.);
  rGoodPixel->SetStats(0);
  rGoodPixel->GetXaxis()->SetTitleOffset(1.3);
  c1->SetLogy(1);
  rGoodPixel->Draw("hist");
  TLine *l1 = new TLine(0., nChips*64000., rGoodPixel->GetXaxis()->GetBinCenter(rGoodPixel->GetNbinsX()), nChips*64000.);
  l1->SetLineColor(kBlack);
  l1->SetLineStyle(kDashed);
  l1->Draw();
  t1->DrawLatexNDC(0.2, 0.87, Form("N_{pix} = %d #times 64000", nChips));

  c1->SaveAs(("rGoodPixel" + suffix + format).c_str());

  TH1D *hGoodPixel = (TH1D*)pFile->Get("hGoodPixel");
  cout << " plotting hGoodPixel" << endl;
  hGoodPixel->SetStats(0);
  hGoodPixel->Draw("hist");
  t1->DrawLatexNDC(0.2, 0.86, Form("<N_{good}> = %d", static_cast<int>(hGoodPixel->GetMean())));
  t1->DrawLatexNDC(0.2, 0.81, Form("#varepsilon = %4.3f", hGoodPixel->GetMean()/nChips/64000.));

  c1->SaveAs(("hGoodPixel" + suffix + format).c_str());



  TH1D *rPayloadSize = (TH1D*)pFile->Get("rPayloadSize");
  cout << " plotting rPayloadSize" << endl;
  setRunLabelsX(rPayloadSize);
  rPayloadSize->SetMinimum(0.5);
  rPayloadSize->SetStats(0);
  rPayloadSize->GetXaxis()->SetTitleOffset(1.3);
  c1->SetLogy(1);
  rPayloadSize->Draw("hist");
  c1->SaveAs(("rPayloadSize" + suffix + format).c_str());



  TH1D *rEEE = (TH1D*)pFile->Get("rEEE");
  cout << " plotting rEEE" << endl;
  setRunLabelsX(rEEE);
  rEEE->SetMinimum(0.5);
  rEEE->SetStats(0);
  rEEE->GetXaxis()->SetTitleOffset(1.3);
  c1->SetLogy(1);
  rEEE->Draw("hist");
  c1->SaveAs(("rEEE" + suffix + format).c_str());

  TH1D *hEEE = (TH1D*)pFile->Get("hEEE");
  cout << " plotting hEEE" << endl;
  hEEE->SetStats(0);
  hEEE->Draw("hist");
  c1->SaveAs(("hEEE" + suffix + format).c_str());

  // -- close the file  
  pFile->Close();
}

// ----------------------------------------------------------------------
// -- set the run labels
void setRunLabelsY(TH2D *h) {
  int nRuns(h->GetNbinsY());
  h->GetYaxis()->SetNdivisions(2, false);
  h->GetYaxis()->SetTickLength(0.0);

  int empty(5);
  if (nRuns > 100) empty = 20;
  if (nRuns > 500) empty = 50;
  if (nRuns > 1000) empty = 200;

  TLine *l1 = new TLine();
  l1->SetLineStyle(kDashed);
  for (int i = 0; i < h->GetNbinsY(); i++) {
    if (i == 0 || i % empty == 0) {
      h->GetYaxis()->SetBinLabel(i+1, Form("%d", indexRun(i)));
      l1->DrawLine(h->GetXaxis()->GetBinCenter(1), h->GetYaxis()->GetBinCenter(i+1), h->GetXaxis()->GetBinCenter(h->GetNbinsX()), h->GetYaxis()->GetBinCenter(i+1));
    } else {
      h->GetYaxis()->SetBinLabel(i+1, "");
    }
  }


}


// ----------------------------------------------------------------------
// -- set the run labels
void setRunLabelsX(TH1D *h) {
  int nRuns(h->GetNbinsX());
  int empty(5);
  if (nRuns > 100) empty = 50;
  if (nRuns > 1000) empty = 100;
  h->GetXaxis()->SetNdivisions(2, false);
  h->GetXaxis()->SetTickLength(0.0);

  for (int i = 1; i < h->GetNbinsX(); i++) {
    if (i == 1 ||i % empty == 0) {
      h->GetXaxis()->SetBinLabel(i, Form("%d", indexRun(i)));
    } else {
      h->GetXaxis()->SetBinLabel(i, "");
    }
  }
}

// ----------------------------------------------------------------------
// -- overlay histograms in files X and Y
void overlayHistograms(string filename1, string filename2) {
  TFile *pFile1 = new TFile(filename1.c_str());
  TFile *pFile2 = new TFile(filename2.c_str());
  TCanvas *c1 = new TCanvas("c1", "c1", 1000, 1000);
  shrinkPad(0.12, 0.15, 0.15);

  TH1D *hLinkStatus1 = (TH1D*)pFile1->Get("hLinkStatus");
  TH1D *hLinkStatus2 = (TH1D*)pFile2->Get("hLinkStatus");
  hLinkStatus1->SetStats(0);
  hLinkStatus2->SetStats(0);
  hLinkStatus1->SetMarkerStyle(2);
  hLinkStatus1->SetMarkerSize(0.8);
  setFilledHist(hLinkStatus2, kBlack, kYellow, 1000);
  hLinkStatus2->Draw("hist");
  hLinkStatus1->Draw("psame");

  TLegend *legg = new TLegend(0.18, 0.7, 0.50, 0.85);
  legg->SetBorderSize(0);
  legg->AddEntry(hLinkStatus1, filename1.c_str(), "p");
  legg->AddEntry(hLinkStatus2, filename2.c_str(), "f");
  legg->SetTextSize(0.03);
  legg->Draw();
  c1->SaveAs("hLinkStatus-overlay.pdf");


  c1->Clear();
  shrinkPad(0.12, 0.15, 0.15);
  TH1D *hNoisyPixels1 = (TH1D*)pFile1->Get("hNoisyPixels");
  TH1D *hNoisyPixels2 = (TH1D*)pFile2->Get("hNoisyPixels");
  hNoisyPixels1->SetStats(0);
  hNoisyPixels2->SetStats(0);
  hNoisyPixels1->SetMarkerStyle(2);
  hNoisyPixels1->SetMarkerSize(0.8);
  hLinkStatus1->SetMarkerSize(0.8);
  setFilledHist(hNoisyPixels2, kBlack, kYellow, 1000);
  hNoisyPixels2->Draw("hist");
  hNoisyPixels1->Draw("psame");
  legg = new TLegend(0.18, 0.7, 0.50, 0.85);
  legg->SetBorderSize(0);
  legg->AddEntry(hNoisyPixels1, filename1.c_str(), "p");
  legg->AddEntry(hNoisyPixels2, filename2.c_str(), "f");
  legg->SetTextSize(0.03);
  legg->Draw();

  c1->SaveAs("hNoisyPixels-overlay.pdf"); 

  c1->Clear();
  shrinkPad(0.12, 0.15, 0.15);
  TH1D *hGoodPixel1 = (TH1D*)pFile1->Get("hGoodPixel");
  TH1D *hGoodPixel2 = (TH1D*)pFile2->Get("hGoodPixel");
  hGoodPixel1->SetStats(0);
  hGoodPixel2->SetStats(0);
  hGoodPixel1->SetMarkerStyle(2);
  hGoodPixel1->SetMarkerSize(0.8);
  hGoodPixel2->SetMarkerSize(0.8);
  setFilledHist(hGoodPixel2, kBlack, kYellow, 1000);
  hGoodPixel2->Draw("hist");
  hGoodPixel1->Draw("psame");
  legg = new TLegend(0.18, 0.7, 0.50, 0.85);
  legg->SetBorderSize(0);
  legg->AddEntry(hGoodPixel1, filename1.c_str(), "p");
  legg->AddEntry(hGoodPixel2, filename2.c_str(), "f");
  legg->SetTextSize(0.03);
  legg->Draw();
  c1->SaveAs("hGoodPixel-overlay.pdf");
}