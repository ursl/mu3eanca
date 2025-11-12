#include <Rtypes.h>
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

struct runQuality jakGoodRunList(calPixelQualityLM *pPQ, int runnumber, vector<int> &vJaksRuns);


void writeRunList(string filename, vector<int> &vRuns);
void plotHistograms(string filename, string suffix);
void setRunLabelsY(TH2D *h);
void setRunLabelsX(TH1D *h);

// ----------------------------------------------------------------------
int main(int argc, char *argv[]) {

  // -- command line arguments
  string filename("anaPixelQuality.root");
  string gt("datav6.3=2025V0");
  string db("/Users/ursl/data/mu3e/cdb");
  int first(0), last(0);
  int verbose(0);
  int plot(0);
  string srunfile("");
  string srunlist("");
  string suffix("");
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-db"))    {db = argv[++i];}
    if (!strcmp(argv[i], "-f"))     {first = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-g"))     {gt = argv[++i];}
    if (!strcmp(argv[i], "-l"))     {last = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-o"))     {filename = argv[++i];}
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
  int minRun(vIoV[0]);
  if (first > 0) minRun = first;
  int maxRun(vIoV[vIoV.size()-1]);
  if (last > 0) maxRun = last;
  int nRuns(maxRun - minRun + 1);

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

  nRuns = std::distance(vIoV.begin(), std::find(vIoV.begin(), vIoV.end(), maxRun)) 
          - std::distance(vIoV.begin(), std::find(vIoV.begin(), vIoV.end(), minRun)) + 1;


  cout << "Number of runs: " << nRuns 
       << " from " << minRun << " to " << maxRun
       << endl;


  // -- initialize the run number index
  int cnt(0);
  for (int i = 0; i < vIoV.size(); i++) {
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
  TH2D *hLinkStatus = new TH2D("hLinkStatus", "Working links per chip", 108, 0, 108, nRuns, 0, nRuns);
  hLinkStatus->SetXTitle("Chip index");
  hLinkStatus->SetYTitle("Run ");
  hLinkStatus->SetZTitle("Working links");  
  hLinkStatus->SetMaximum(3); // simplest way to get reasonable color scheme

  // -- Number of noisy pixels per run
  TH1D *hNoisyPixelVsRun = new TH1D("hNoisyPixelVsRun", "Total number of noisy pixels per run", nRuns, 0, nRuns);
  hNoisyPixelVsRun->SetXTitle("Run");
  hNoisyPixelVsRun->SetYTitle("Noisy pixels");

  TH1D *hEEEVsRun = new TH1D("hEEEVsRun", "Number of links with LVDS errors per run", nRuns, 0, nRuns);
  hEEEVsRun->SetXTitle("Run");
  hEEEVsRun->SetYTitle("Links with LVDS errors");

  // -- this is only for debugging (e.g. if some illegal chipIDs sneaked in)
  TH1D *hGoodPixelVsRun = new TH1D("hGoodPixelVsRun", "Total number of good pixels per run", nRuns, 0, nRuns);
  hGoodPixelVsRun->SetXTitle("Run");
  hGoodPixelVsRun->SetYTitle("Good pixels");

  // -- Size of JSON payload
  TH1D *hPayloadSize = new TH1D("hPayloadSize", "Size of JSON payload", nRuns, 0, nRuns);
  hPayloadSize->SetXTitle("Run");
  hPayloadSize->SetYTitle("Size of JSON payload [kB]");

  vector<int> vJaksRuns, vAllBadLinkRuns, vGoodPixelRuns, vGoodRuns;
  
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
    pDC->setRunNumber(itIoV);
    uint32_t chipid(0);
    pPQ->resetIterator();
    int nNoisyPixels(0);
    int nGoodPixels(0);
    int nChips(0);
    //cout << " Working links: ";  
    while (pPQ->getNextID(chipid)) {
      hLinkStatus->Fill(chipIndex(chipid), runIndex(itIoV), linkStatus(pPQ, chipid));
      int chipNoisy = pPQ->getNpixWithStatus(chipid, calPixelQualityLM::Noisy); 
      int chipGood = pPQ->getNpixWithStatus(chipid, calPixelQualityLM::Good);
      nNoisyPixels += chipNoisy;
      nGoodPixels += chipGood;
      ++nChips;
      if (0) cout << "Chip " << chipid << " has " << chipNoisy << " noisy pixels and " << chipGood << " good pixels" << endl;
    }
    cout << "Run " << itIoV << " has " 
         << nChips << " chips, "
         << nNoisyPixels << " noisy pixels and " 
         << nGoodPixels << " good pixels" 
         << endl;
    hNoisyPixelVsRun->Fill(runIndex(itIoV), nNoisyPixels);
    hGoodPixelVsRun->Fill(runIndex(itIoV), nGoodPixels);
    if (nGoodPixels < 1.e6) {
      vGoodPixelRuns.push_back(itIoV);
    }
    // -- get the size of the JSON payload via size of BLOB
    string sblob = pPQ->makeBLOB();
    hPayloadSize->Fill(runIndex(itIoV), sblob.size()/1024.);

    struct runQuality rn = jakGoodRunList(pPQ, itIoV, vJaksRuns);
    hEEEVsRun->Fill(runIndex(itIoV), rn.nEEE);
    if (rn.nEEE > 40) {
      vAllBadLinkRuns.push_back(itIoV);
    }
  }
   // -- save the histograms 
  pFile->Write();
  pFile->Close();

  plotHistograms(filename, suffix);


  vector<int> vJakGoodRuns = {4756, 4757, 4758, 4863, 4864, 4865, 4866, 4868, 4869, 4870, 4871, 4872, 4873, 4876, 4877, 4878, 4880, 4881, 4882, 4883, 4884, 4885, 4886, 4887, 4888, 4889, 4890, 4891, 4892, 4893, 4894, 4896, 4897, 4898, 4899, 4900, 5102};

  cout << "Jak's run list has " << vJaksRuns.size() << " runs" << endl;
  string sfilename = "goodJakRunList" + suffix + ".txt";
  writeRunList(sfilename, vJaksRuns);

  cout << "Bad run list (all bad links) has " << vAllBadLinkRuns.size() << " runs" << endl;
  sfilename = "badLinksTooManyRunList" + suffix + ".txt";
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
    if (find(vJakGoodRuns.begin(), vJakGoodRuns.end(), it) != vJakGoodRuns.end()) {
      cout << "*" << it << "*,";
    }
    cout << it << ",";
  }
  cout << endl;


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
void plotHistograms(string filename, string suffix) {
  // -- open the file
  TFile *pFile = new TFile(filename.c_str());

  // -- plot the histograms
  TCanvas *c1 = new TCanvas("c1", "c1", 1000, 1000);
  c1->SetRightMargin(0.15);

  TH2D *hLinkStatus = (TH2D*)pFile->Get("hLinkStatus");
  setRunLabelsY(hLinkStatus);
  hLinkStatus->SetStats(0);
  hLinkStatus->Draw("colz");
  c1->SaveAs(("hLinkStatus" + suffix + ".pdf").c_str());

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
  c1->SaveAs(("hNoisyPixelVsRun" + suffix + ".pdf").c_str());

  TH1D *hGoodPixelVsRun = (TH1D*)pFile->Get("hGoodPixelVsRun");
  setRunLabelsX(hGoodPixelVsRun);
  hGoodPixelVsRun->SetMinimum(1.e5);
  hGoodPixelVsRun->SetStats(0);
  hGoodPixelVsRun->GetXaxis()->SetTitleOffset(1.3);
  c1->SetLogy(1);
  hGoodPixelVsRun->Draw("hist");
  TLine *l1 = new TLine(0., 108*64000., hGoodPixelVsRun->GetXaxis()->GetBinCenter(hGoodPixelVsRun->GetNbinsX()), 108*64000.);
  l1->SetLineColor(kBlack);
  l1->SetLineStyle(kDashed);
  l1->Draw();
  TLatex *t1 = new TLatex();
  t1->SetTextColor(kBlack);
  t1->SetTextSize(0.02);
  t1->DrawLatexNDC(0.2, 0.82, "N_{pix}^{VTX} = 108*64000");

  c1->SaveAs(("hGoodPixelVsRun" + suffix + ".pdf").c_str());


  TH1D *hPayloadSize = (TH1D*)pFile->Get("hPayloadSize");
  setRunLabelsX(hPayloadSize);
  hPayloadSize->SetMinimum(0.5);
  hPayloadSize->SetStats(0);
  hPayloadSize->GetXaxis()->SetTitleOffset(1.3);
  c1->SetLogy(1);
  hPayloadSize->Draw("hist");
  c1->SaveAs(("hPayloadSize" + suffix + ".pdf").c_str());



  TH1D *hEEEVsRun = (TH1D*)pFile->Get("hEEEVsRun");
  setRunLabelsX(hEEEVsRun);
  hEEEVsRun->SetMinimum(0.5);
  hEEEVsRun->SetStats(0);
  hEEEVsRun->GetXaxis()->SetTitleOffset(1.3);
  c1->SetLogy(1);
  hEEEVsRun->Draw("hist");
  c1->SaveAs(("hEEEVsRun" + suffix + ".pdf").c_str());

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
  if (nRuns > 100) empty = 50;
  if (nRuns > 1000) empty = 100;

  for (int i = 1; i < h->GetNbinsY(); i++) {
    if (i ==1 || i % empty == 0) {
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
// -- print the good run list
struct runQuality jakGoodRunList(calPixelQualityLM *pPQ, int runnumber, vector<int> &vJaksRuns) {
  uint32_t chipid(0);
  pPQ->resetIterator();
  struct runQuality rn = {runnumber, 0, 0, 0, 0, 0, 0, 0};
  string sStatus("");
  while (pPQ->getNextID(chipid)) {
    int status(0), nNoisy(0), nDeadLinks(0), nBadLinks(0);
    nNoisy = pPQ->getNpixWithStatus(chipid, calPixelQualityLM::Noisy);
    rn.nNoisyPixels += nNoisy;
    for (int i = 0; i < 3; i++) {
      if (pPQ->getLinkStatus(chipid, i) == 4 || pPQ->getLinkStatus(chipid, i) == 5) {
        nBadLinks |= (1 << i);
      }

      if (pPQ->isLinkDead(chipid, i)) {
        nDeadLinks++;
        rn.nDeadLinks++;
      }
    }
    if (nBadLinks == 7) {
      //      cout << "Run " << runnumber << " has EEE" << endl;
      status = 5;
      sStatus += "/" + to_string(chipid) + "/"+ " EEE,";
      rn.nEEE++;
    }

    if (nDeadLinks < 3) {
      rn.nActiveChips++;
    }

    if (nNoisy > 10) {
      if (0 == status) {
        sStatus += "/" + to_string(chipid) + "/" + " Ns: " + to_string(nNoisy) + ",";
      } else {
        sStatus +=  " Ns: " + to_string(nNoisy) + ",";
      }
      status = 5;
    }
    if (5 == status) {
      rn.nStatus5++;
    }
  }

  if (rn.nStatus5 < 0.2*rn.nActiveChips) {
    cout << "******************** Run " << runnumber << " is good, has " << rn.nActiveChips << " active chips and " << rn.nStatus5 << " status 5" << endl;
    vJaksRuns.push_back(runnumber);
  } else {
    cout << "Run " << runnumber << " is bad, has " << rn.nActiveChips << " active chips and nStatus5 = " << rn.nStatus5 << " -> " << sStatus << "<-" << endl;
  }
  return rn;
}
