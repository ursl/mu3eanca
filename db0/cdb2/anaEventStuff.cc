#include <Rtypes.h>
#include <TAttMarker.h>
#include <TStyle.h>
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

#include "Mu3eConditions.hh"

#include "calEventStuffV1.hh"

#include "util.hh"

using namespace std;

// ----------------------------------------------------------------------
// anaEventStuff [-v] [-f firstRun] [-l lastRun] [-o FILENAME] [-p]
// -------------
//
// Analyze eventStuff data
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
// ----------------------------------------------------------------------

std::unordered_map<int, int> gRunNumberIndex;


int runIndex(int runnumber);
void plotHistograms(string filename, string suffix);
void setRunLabelsY(TH2D *h);
void setRunLabelsX(TH1D *h);
void overlayHistograms(string filename1, string filename2);

// ----------------------------------------------------------------------
int main(int argc, char *argv[]) {

  // -- command line arguments
  string filename("anaEventStuff.root");
  string gt("datav6.3=2025V1");
  string db("/Users/ursl/data/mu3e/cdb");
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
    filename = string("anaEventStuff") + suffix + ".root";
    cout << "Suffix: " << suffix << endl;
    cout << "Filename: " << filename << endl;
  } else {
    filename = string("anaEventStuff.root");
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
  // -- keep the definition of the run ranges from the CDB
  vector<int> vIoV;
  bool foundPQ(false);
  for (auto it: vTags) {
    if (string::npos != it.find("eventstuffv1_")) {
      foundPQ = true;
      vIoV = pDC->getIOVs(it);
      cout << "Tag " << it << " has " << vIoV.size() << " IOVs: ";
      for (auto itIoV: vIoV)  cout << itIoV << " ";
      cout << endl;
    }
  }
  if (!foundPQ) {
    cout << "ERROR: no event stuff data found" << endl;
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
  calEventStuffV1 *pPQ = (calEventStuffV1*)pDC->getCalibration("eventstuffv1_");
  if (pPQ) {
    cout << "eventstuffv1_ found" << endl;
    pPQ->setVerbosity(verbose);
  } else {
    cout << "eventstuffv1_ not found" << endl;
    return 0;
  }


  // -- Plot stuff for each run
  TFile *pFile = new TFile(filename.c_str(), "RECREATE");
  filename1 = filename;
  replaceAll(filename1, ".root", ".log");
  ofstream ofs(filename1.c_str());

  // -- last frame
  TH1D *hLastFrame = new TH1D("hLastFrame", "Last frame", 100, 0, 1.e12);
  setTitles(hLastFrame, "Last frame", "Number of runs", 0.05, 1.0, 1.2, 0.04);

  TH1D *rLastFrame = new TH1D("rLastFrame", "Last frame vs Run", nRuns, 0, nRuns);
  setTitles(rLastFrame, "Run", "Last frame", 0.05, 1.0, 1.2, 0.03);
  rLastFrame->SetMinimum(0.5);

  // -- pixel frame duration  
  TH1D *hPixelFrameDuration = new TH1D("hPixelFrameDuration", "Pixel frame duration", 100, 0, 1.e10);
  setTitles(hPixelFrameDuration, "Pixel frame duration", "Number of runs", 0.05, 1.0, 1.4, 0.04);

  TH1D *rPixelFrameDuration = new TH1D("rPixelFrameDuration", "Pixel frame duration vs Run", nRuns, 0, nRuns);
  setTitles(rPixelFrameDuration, "Run", "Pixel frame duration", 0.05, 1.0, 1.4, 0.03);
  rPixelFrameDuration->SetMinimum(0.5);

  // -- pixel uptime ratio (to total length)
  TH1D *hPixelUptimeRatio = new TH1D("hPixelUptimeRatio", "Pixel uptime ratio", 100, 0, 1.0);
  setTitles(hPixelUptimeRatio, "Pixel uptime ratio", "Number of runs", 0.05, 1.0, 1.4, 0.04);

  TH1D *rPixelUptimeRatio = new TH1D("rPixelUptimeRatio", "Pixel uptime ratio vs Run", nRuns, 0, nRuns);
  setTitles(rPixelUptimeRatio, "Run", "Pixel uptime ratio", 0.05, 1.0, 1.4, 0.03);
  rPixelUptimeRatio->SetMinimum(0.0);
  rPixelUptimeRatio->SetMaximum(1.0);

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
      string hash = "tag_eventstuffv1_" + gt + "_iov_" + to_string(itIoV);
      cout << "Reading payload from " << payloadDir << "/" << hash << endl;
      pPQ->readPayloadFromFile(hash, payloadDir);
      pPQ->calculate(hash);
    }
    uint64_t pixelStart = pPQ->startFrameGoodPixelData();
    uint64_t pixelEnd = pPQ->endFrameGoodPixelData();
    uint64_t lastFrame = pPQ->endFrameEventData();

    // -- skip runs with invalid data
    if (pixelStart > 1.e16 || 
        pixelEnd > 1.e16 ||
        lastFrame > 1.e16) {
      continue;
    }
 
    if (pixelEnd > lastFrame) {
      cout << "Run " << itIoV << " pixelEnd > lastFrame" << endl;
      continue;
    }

    double pixelDuration = pixelEnd - pixelStart;
    double pixelUptimeRatio = pixelDuration/(lastFrame - pixelStart);

    rLastFrame->Fill(runIndex(itIoV), static_cast<double>(lastFrame));
    hLastFrame->Fill(static_cast<double>(lastFrame));

    rPixelFrameDuration->Fill(runIndex(itIoV), static_cast<double>(pixelDuration));
    hPixelFrameDuration->Fill(static_cast<double>(pixelDuration));

    rPixelUptimeRatio->Fill(runIndex(itIoV), static_cast<double>(pixelUptimeRatio));
    hPixelUptimeRatio->Fill(static_cast<double>(pixelUptimeRatio));

    ofs << "Run " << itIoV << " lastFrame: "  << lastFrame 
         << " pixelDuration: " << pixelDuration 
         << " pixelUptimeRatio: " << pixelUptimeRatio 
         << " pixelDuration: " << pixelDuration  << " "
         << " pixelStart: " << pixelStart << " "
         << " pixelEnd: " << pixelEnd << " "
         << " lastFrame: " << lastFrame << " "
         << endl;
  }
  ofs.close();
   // -- save the histograms 
  pFile->Write();
  pFile->Close();

  plotHistograms(filename, suffix);

  return 0;
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
// -- plot the histograms
void plotHistograms(string filename, string suffix) {
  // -- open the file
  TFile *pFile = new TFile(filename.c_str());

  // -- plot the histograms
  TCanvas *c1 = new TCanvas("c1", "c1", 1000, 1000);
  shrinkPad(0.12, 0.15, 0.15);

  gStyle->SetOptStat(111111);

  TH1D *hLastFrame = (TH1D*)pFile->Get("hLastFrame");
  cout << " plotting hLastFrame" << endl;
  hLastFrame->Draw("hist");
  c1->SaveAs(("hLastFrame" + suffix + ".pdf").c_str());


  gStyle->SetOptStat(0);

  TH1D *rLastFrame = (TH1D*)pFile->Get("rLastFrame");
  gPad->SetLogy(1);
  setRunLabelsX(rLastFrame);
  cout << " plotting rLastFrame" << endl;
  rLastFrame->Draw("hist");
  c1->SaveAs(("rLastFrame" + suffix + ".pdf").c_str());


  shrinkPad(0.15, 0.15, 0.1);
  gPad->SetLogy(0);
  TH1D *hPixelFrameDuration = (TH1D*)pFile->Get("hPixelFrameDuration");
  cout << " plotting hPixelFrameDuration" << endl;
  hPixelFrameDuration->Draw("hist");
  c1->SaveAs(("hPixelFrameDuration" + suffix + ".pdf").c_str());

  TH1D *rPixelFrameDuration = (TH1D*)pFile->Get("rPixelFrameDuration");
  setRunLabelsX(rPixelFrameDuration);
  gPad->SetLogy(1);
  cout << " plotting rPixelFrameDuration" << endl;
  rPixelFrameDuration->Draw("hist");
  c1->SaveAs(("rPixelFrameDuration" + suffix + ".pdf").c_str());


  shrinkPad(0.15, 0.15, 0.1);
  TH1D *hPixelUptimeRatio = (TH1D*)pFile->Get("hPixelUptimeRatio");
  gPad->SetLogy(0);
  cout << " plotting hPixelUptimeRatio" << endl;
  hPixelUptimeRatio->Draw("hist");
  c1->SaveAs(("hPixelUptimeRatio" + suffix + ".pdf").c_str());

  TH1D *rPixelUptimeRatio = (TH1D*)pFile->Get("rPixelUptimeRatio");
  setRunLabelsX(rPixelUptimeRatio);
  gPad->SetLogy(0);
  cout << " plotting rPixelUptimeRatio" << endl;
  rPixelUptimeRatio->Draw("hist");
  c1->SaveAs(("rPixelUptimeRatio" + suffix + ".pdf").c_str());
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
  TH1D *hLastFrame1 = (TH1D*)pFile1->Get("hLastFrame");
  TH1D *hLastFrame2 = (TH1D*)pFile2->Get("hLastFrame");
  hLastFrame1->SetMarkerStyle(2);
  hLastFrame1->SetMarkerSize(0.8);
  setFilledHist(hLastFrame2, kBlack, kYellow, 1000);
  hLastFrame2->Draw("hist");
  hLastFrame1->Draw("psame"); 
  TLegend *legg = new TLegend(0.18, 0.7, 0.50, 0.85);
  legg->SetBorderSize(0);
  legg->AddEntry(hLastFrame1, filename1.c_str(), "p");
  legg->AddEntry(hLastFrame2, filename2.c_str(), "f");
  legg->SetTextSize(0.03);
  legg->Draw();
  c1->SaveAs("hLastFrame-overlay.pdf");
}