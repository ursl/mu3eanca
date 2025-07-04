#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>
#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TH1.h>
#include <TH2.h>
#include <string.h>
#include <unordered_map>
#include <sstream>

#include "cdbRest.hh"
#include "cdbJSON.hh"

#include "Mu3eConditions.hh"

#include "calPixelQualityLM.hh"

using namespace std;

// ----------------------------------------------------------------------
// anaPixelQuality [-v] [-f firstRun] [-l lastRun] [-o FILENAME] [-p]
// ----------------
//
// Analyze pixel quality data
//
// -v          verbose output
// -o FILENAME output filename
// -f first    runnumber
// -l last     runnumber
// -p          plot only (i.e. no reading of all the payloads)
// -r FILENAME runlist file
// -R r1,r2    comma-separated list of runs
//
// ----------------------------------------------------------------------

std::unordered_map<int, int> gRunNumberIndex;


int chipIndex(unsigned int chipid);
int runIndex(int runnumber, vector<int> &vIoV);
int linkStatus(calPixelQualityLM *pPQ, unsigned int chipid);

void plotHistograms(string filename);
void setRunLabelsY(TH2D *h);
void setRunLabelsX(TH1D *h);

// ----------------------------------------------------------------------
int main(int argc, char *argv[]) {

  // -- command line arguments
  string filename("anaPixelQuality.root");
  string gt("datav6.2=2025Beam");
  string db("/Users/ursl/data/mu3e/cdb");
  int first(0), last(0);
  int verbose(0);
  int plot(0);
  string srunfile("");
  string srunlist("");
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-db"))    {db = argv[++i];}
    if (!strcmp(argv[i], "-f"))     {first = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-g"))     {gt = argv[++i];}
    if (!strcmp(argv[i], "-l"))     {last = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-o"))     {filename = argv[++i];}
    if (!strcmp(argv[i], "-p"))     {plot = 1;}
    if (!strcmp(argv[i], "-r"))     {srunfile = argv[++i];}
    if (!strcmp(argv[i], "-R"))     {srunlist = argv[++i];}
    if (!strcmp(argv[i], "-v"))     {verbose = 1;}
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
    string ms("http://pc11740.psi.ch/cdb");
    pDB = new cdbRest(gt, ms, verbose);
  } else {
    // -- hope for the best that this is a JSON directory
    pDB = new cdbJSON(gt, db, verbose);
  }

  // -- initialize the conditions
  Mu3eConditions *pDC = Mu3eConditions::instance(gt, pDB);
  pDC->setVerbosity(verbose);
  vector<string> vTags = pDC->getTags();
  vector<int> vIoV;
  bool foundPQ(false);
  for (auto it: vTags) {
    if (string::npos != it.find("pixelqualitylm_")) {
      foundPQ = true;
      vIoV = pDC->getIOVs(it);
      for (auto itIoV: vIoV)  cout << itIoV << " ";
      cout << endl;
    }
  }
  if (!foundPQ) {
    cout << "ERROR: no pixel quality data found" << endl;
    return 0;
  }

  // -- initialize the run number index
  for (int i = 0; i < vIoV.size(); i++) {
    gRunNumberIndex[vIoV[i]] = i;
  }

  // -- check if we should plot only. This MUST BE after reading the CDB because else the IOVs are not known!
  if (plot) {
    plotHistograms(filename);
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

  // -- get the number of runs
  int minRun(vIoV[0]);
  int maxRun(vIoV[vIoV.size()-1]);
   int nRuns(maxRun - minRun + 1);

  // -- in terms of run index
  minRun = 0; 
  maxRun = vIoV.size();
  nRuns = vIoV.size();

  // -- Plot stuff for each run
  TFile *pFile = new TFile(filename.c_str(), "RECREATE");
  TH2D *hLinkStatus = new TH2D("hLinkStatus", "Working links per chip", 108, 0, 108, nRuns, minRun, maxRun);
  hLinkStatus->SetXTitle("Chip index");
  hLinkStatus->SetYTitle("Run ");
  hLinkStatus->SetZTitle("Working links");  
  hLinkStatus->SetMaximum(6); // simplest way to get reasonable color scheme

  TH1D *hNoisyPixelVsRun = new TH1D("hNoisyPixelVsRun", "Noisy pixels per run", nRuns, minRun, maxRun);
  hNoisyPixelVsRun->SetXTitle("Run");
  hNoisyPixelVsRun->SetYTitle("Noisy pixels");

  // -- this is only for debugging (e.g. if some illegal chipIDs sneaked in)
  TH1D *hGoodPixelVsRun = new TH1D("hGoodPixelVsRun", "Good pixels per run", nRuns, minRun, maxRun);
  hGoodPixelVsRun->SetXTitle("Run");
  hGoodPixelVsRun->SetYTitle("Good pixels");



  for (auto itIoV: vIoV) {
    if (vRuns.size() > 0 && find(vRuns.begin(), vRuns.end(), itIoV) == vRuns.end()) {
      continue;
    }
    pDC->setRunNumber(itIoV);
    uint32_t chipid(0);
    pPQ->resetIterator();
    int nNoisyPixels(0);
    int nGoodPixels(0);
    int nChips(0);
    //cout << " Working links: ";  
    while (pPQ->getNextID(chipid)) {
      hLinkStatus->Fill(chipIndex(chipid), runIndex(itIoV, vIoV), linkStatus(pPQ, chipid));
      int chipNoisy = pPQ->getNpixWithStatus(chipid, calPixelQualityLM::Noisy); 
      int chipGood = pPQ->getNpixWithStatus(chipid, calPixelQualityLM::Good);
      nNoisyPixels += chipNoisy;
      nGoodPixels += chipGood;
      ++nChips;
      //cout << "Chip " << chipid << " has " << chipNoisy << " noisy pixels and " << chipGood << " good pixels" << endl;
    }
    cout << "Run " << itIoV << " has " 
         << nChips << " chips, "
         << nNoisyPixels << " noisy pixels and " 
         << nGoodPixels << " good pixels" 
         << endl;
    hNoisyPixelVsRun->Fill(runIndex(itIoV, vIoV), nNoisyPixels);
    hGoodPixelVsRun->Fill(runIndex(itIoV, vIoV), nGoodPixels);
  }
   // -- save the histograms 
  pFile->Write();
  pFile->Close();

  plotHistograms(filename);


  return 0;
}

// ----------------------------------------------------------------------
int runIndex(int runnumber, vector<int> &vIoV) {
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
              1313, 1314, 1315, 1316, 1317, 1318
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
              1313, 1314, 1315, 1316, 1317, 1318
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
  if (pPQ->getLinkStatus(chipid, 1) == 0) ++result;
  if (pPQ->getLinkStatus(chipid, 2) == 0) ++result;
  return result;
}

// ----------------------------------------------------------------------
// -- plot the histograms
void plotHistograms(string filename) {
  // -- open the file
  TFile *pFile = new TFile(filename.c_str());

  // -- plot the histograms
  TCanvas *c1 = new TCanvas("c1", "c1", 1000, 1000);
  c1->SetRightMargin(0.15);

  TH2D *hLinkStatus = (TH2D*)pFile->Get("hLinkStatus");
  setRunLabelsY(hLinkStatus);
  hLinkStatus->SetStats(0);
  hLinkStatus->Draw("colz");
  c1->SaveAs("hLinkStatus.pdf");

  c1->Clear();
  c1->SetRightMargin(0.1);
  c1->SetBottomMargin(0.15);


  TH1D *hNoisyPixelVsRun = (TH1D*)pFile->Get("hNoisyPixelVsRun");
  setRunLabelsX(hNoisyPixelVsRun);
  hNoisyPixelVsRun->SetMinimum(0.5);
  hNoisyPixelVsRun->SetStats(0);
  hNoisyPixelVsRun->GetXaxis()->SetTitleOffset(1.3);
  c1->SetLogy(1);
  hNoisyPixelVsRun->Draw("hist");
  c1->SaveAs("hNoisyPixelVsRun.pdf");

  TH1D *hGoodPixelVsRun = (TH1D*)pFile->Get("hGoodPixelVsRun");
  setRunLabelsX(hGoodPixelVsRun);
  hGoodPixelVsRun->SetMinimum(0.5);
  hGoodPixelVsRun->SetStats(0);
  hGoodPixelVsRun->GetXaxis()->SetTitleOffset(1.3);
  c1->SetLogy(1);
  hGoodPixelVsRun->Draw("hist");
  c1->SaveAs("hGoodPixelVsRun.pdf");

  // -- close the file  
  pFile->Close();
}

// ----------------------------------------------------------------------
// -- set the run labels
void setRunLabelsY(TH2D *h) {
  int nRuns(h->GetNbinsY());
  h->GetYaxis()->SetNdivisions(2, false);
  h->GetYaxis()->SetTickLength(0.0);

  int empty(0);
  if (nRuns > 100) empty = 10;
  if (nRuns > 500) empty = 50;

  for (int i = 1; i < h->GetNbinsY(); i++) {
    if (i % empty == 1) {
      h->GetYaxis()->SetBinLabel(i, Form("%d", indexRun(i)));
    } else {
      h->GetYaxis()->SetBinLabel(i, "");
    }
  }


}


// ----------------------------------------------------------------------
// -- set the run labels
void setRunLabelsX(TH1D *h) {
  int nRuns(h->GetNbinsX());
  int empty(0);
  if (nRuns > 100) empty = 10;
  if (nRuns > 500) empty = 50;
  h->GetXaxis()->SetNdivisions(2, false);
  h->GetXaxis()->SetTickLength(0.0);

  for (int i = 1; i < h->GetNbinsX(); i++) {
    if (i % empty == 1) {
      h->GetXaxis()->SetBinLabel(i, Form("%d", indexRun(i)));
    } else {
      h->GetXaxis()->SetBinLabel(i, "");
    }
  }
}