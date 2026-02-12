#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <cstring>
#include <glob.h>
#include <map>
#include "calPixelQualityLM.hh"

#include "TH2D.h"
#include "TH1D.h"
#include "TFile.h"
#include "TCanvas.h"

#include "util.hh"

using namespace std;

// ----------------------------------------------------------------------
vector<string> glob(const string& pattern) {
  vector<string> vFiles;
  glob_t globbuf;
  string lPattern = pattern;
  if (lPattern.find("*") == string::npos) {
    lPattern = lPattern + "*";
  }
  if (glob(lPattern.c_str(), GLOB_TILDE, NULL, &globbuf) == 0) {
    for (size_t i = 0; i < globbuf.gl_pathc; i++) {
      vFiles.push_back(string(globbuf.gl_pathv[i]));
    }
  }
  globfree(&globbuf);
  return vFiles;
}

// ----------------------------------------------------------------------
map<string, int> getPayloadStats(calPixelQualityLM *pPQ) {
  map<string, int> payloadStats = {
    {"nGoodPixels", 0},
    {"nNoisyPixels", 0},
    {"nGoodLinks", 0},
    {"nStatus4Links", 0},
    {"nStatus5Links", 0},
    {"nStatus9Links", 0}
  };
  uint32_t chipid(0);
  while (pPQ->getNextID(chipid)) {
    payloadStats["nGoodPixels"] += pPQ->getNpixWithStatus(chipid, calPixelQualityLM::Good);
    payloadStats["nNoisyPixels"] += pPQ->getNpixWithStatus(chipid, calPixelQualityLM::Noisy);
    payloadStats["nGoodLinks"] += (pPQ->getLinkStatus(chipid, 0) == calPixelQualityLM::Good) ? 1 : 0;
    payloadStats["nGoodLinks"] += (pPQ->getLinkStatus(chipid, 1) == calPixelQualityLM::Good) ? 1 : 0;
    payloadStats["nGoodLinks"] += (pPQ->getLinkStatus(chipid, 2) == calPixelQualityLM::Good) ? 1 : 0;

    payloadStats["nStatus4Links"] += (pPQ->getLinkStatus(chipid, 0) == calPixelQualityLM::LVDSErrorLink) ? 1 : 0;
    payloadStats["nStatus4Links"] += (pPQ->getLinkStatus(chipid, 1) == calPixelQualityLM::LVDSErrorLink) ? 1 : 0;
    payloadStats["nStatus4Links"] += (pPQ->getLinkStatus(chipid, 2) == calPixelQualityLM::LVDSErrorLink) ? 1 : 0;
    payloadStats["nStatus5Links"] += (pPQ->getLinkStatus(chipid, 0) == calPixelQualityLM::LVDSErrorOtherLink) ? 1 : 0;
    payloadStats["nStatus5Links"] += (pPQ->getLinkStatus(chipid, 1) == calPixelQualityLM::LVDSErrorOtherLink) ? 1 : 0;
    payloadStats["nStatus5Links"] += (pPQ->getLinkStatus(chipid, 2) == calPixelQualityLM::LVDSErrorOtherLink) ? 1 : 0;
    payloadStats["nStatus9Links"] += (pPQ->getLinkStatus(chipid, 0) == calPixelQualityLM::Masked) ? 1 : 0;
    payloadStats["nStatus9Links"] += (pPQ->getLinkStatus(chipid, 1) == calPixelQualityLM::Masked) ? 1 : 0;
    payloadStats["nStatus9Links"] += (pPQ->getLinkStatus(chipid, 2) == calPixelQualityLM::Masked) ? 1 : 0;
  }
  return payloadStats;
}


void plotHistograms(string filename) {
  TFile *pFile = TFile::Open(filename.c_str());
  TH1D *h1GoodLinks = (TH1D*)pFile->Get("h1GoodLinks");
  TH1D *h1NoisyPixels = (TH1D*)pFile->Get("h1NoisyPixels");
  TH1D *h1Status4Links = (TH1D*)pFile->Get("h1Status4Links");
  TH1D *h1Status5Links = (TH1D*)pFile->Get("h1Status5Links");
  TH1D *h1Status9Links = (TH1D*)pFile->Get("h1Status9Links");

  TH2D *hGoodLinks = (TH2D*)pFile->Get("hGoodLinks");
  TH2D *hNoisyPixels = (TH2D*)pFile->Get("hNoisyPixels");
  TH2D *hStatus4Links = (TH2D*)pFile->Get("hStatus4Links");
  TH2D *hStatus5Links = (TH2D*)pFile->Get("hStatus5Links");
  TH2D *hStatus9Links = (TH2D*)pFile->Get("hStatus9Links");

  TCanvas *c1 = new TCanvas("c1", "c1", 1000, 1000);
  c1->cd(1);
  c1->SetRightMargin(0.20);
  hGoodLinks->SetStats(0);
  hGoodLinks->GetZaxis()->SetTitleOffset(1.5);
  hGoodLinks->Draw("colz");
  c1->SaveAs("hGoodLinks.pdf");

  hNoisyPixels->SetStats(0);
  hNoisyPixels->GetZaxis()->SetTitleOffset(1.5);
  hNoisyPixels->Draw("colz");
  c1->SaveAs("hNoisyPixels.pdf");

  hStatus4Links->SetStats(0);
  hStatus4Links->GetZaxis()->SetTitleOffset(1.5);
  hStatus4Links->Draw("colz");
  c1->SaveAs("hStatus4Links.pdf");

  hStatus5Links->SetStats(0);
  hStatus5Links->GetZaxis()->SetTitleOffset(1.5);
  hStatus5Links->Draw("colz");
  c1->SaveAs("hStatus5Links.pdf");

  hStatus9Links->SetStats(0);
  hStatus9Links->GetZaxis()->SetTitleOffset(1.5);
  hStatus9Links->Draw("colz");
  c1->SaveAs("hStatus9Links.pdf");


  c1->SetRightMargin(0.05);
  h1GoodLinks->SetStats(0);
  h1GoodLinks->GetXaxis()->SetTitle("Run");
  h1GoodLinks->Draw("hist");
  c1->SaveAs("h1GoodLinks.pdf");

  h1NoisyPixels->SetStats(0);
  h1NoisyPixels->GetXaxis()->SetTitle("Run");
  h1NoisyPixels->Draw("hist");
  c1->SaveAs("h1NoisyPixels.pdf");

  h1Status4Links->SetStats(0);
  h1Status4Links->GetXaxis()->SetTitle("Run");
  h1Status4Links->Draw("hist");
  c1->SaveAs("h1Status4Links.pdf");

  h1Status5Links->SetStats(0);
  h1Status5Links->GetXaxis()->SetTitle("Run");
  h1Status5Links->Draw("hist");
  c1->SaveAs("h1Status5Links.pdf");

  h1Status9Links->SetStats(0);
  h1Status9Links->GetXaxis()->SetTitle("Run");
  h1Status9Links->Draw("hist");
  c1->SaveAs("h1Status9Links.pdf");

  double MINDIFF(10);
  for (int i = 0; i < h1GoodLinks->GetNbinsX(); i++) {
    if (TMath::Abs(h1GoodLinks->GetBinContent(i)) > MINDIFF) {
      cout << "run = " << static_cast<int>(h1GoodLinks->GetBinCenter(i)) << " Diff(good links) = " << static_cast<int>(h1GoodLinks->GetBinContent(i)) << endl;
    }
  }

}


// ----------------------------------------------------------------------
int main(int argc, char *argv[]) {

  string firstGlob("unset"), secondGlob("unset"), sRunList("unset");
  int plot(0);
  // -- command line parsing 
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-a")) {
      firstGlob = string(argv[++i]);
    }
    if (!strcmp(argv[i], "-b")) {
      secondGlob = string(argv[++i]);
    }

    if (!strcmp(argv[i], "-p")) {
      plot = 1;
    }

    if (!strcmp(argv[i], "-r")) {
      sRunList = string(argv[++i]);
    }
  }

  if (plot) {
    plotHistograms("diffPayloads.root");
    return 0;
  }

  vector<int> vRuns;
  if (sRunList != "unset") {
    stringstream ss(sRunList);
    string token;
    while (getline(ss, token, ',')) {
      vRuns.push_back(stoi(token));
    }
  }


  string sFirstPayloadDir = firstGlob.substr(0, firstGlob.find_last_of("/")+1);
  string sSecondPayloadDir = secondGlob.substr(0, secondGlob.find_last_of("/")+1);
  cout << "sFirstPayloadDir = " << sFirstPayloadDir << " sSecondPayloadDir = " << sSecondPayloadDir << endl;
  vector<string> vFirstPayloads = glob(firstGlob);
  vector<string> vSecondPayloads = glob(secondGlob);

  // -- histograms
  string hash1 = vFirstPayloads[0].substr(vFirstPayloads[0].find_last_of("/")+1);
  replaceAll(hash1, "_iov", "");
  hash1 = hash1.substr(0, hash1.find_last_of("_"));
  replaceAll(hash1, "tag_pixelqualitylm_", "");
  string hash2 = vSecondPayloads[0].substr(vSecondPayloads[0].find_last_of("/")+1);
  replaceAll(hash2, "_iov", "");
  hash2 = hash2.substr(0, hash2.find_last_of("_"));
  replaceAll(hash2, "tag_pixelqualitylm_", "");
  TFile *pFile = new TFile("diffPayloads.root", "RECREATE");
  TH2D *hGoodPixels = new TH2D("hGoodPixels", "Number of good pixels", 100, 0, 6.9e6, 100, 0, 6.9e6);
  TH2D *hNoisyPixels = new TH2D("hNoisyPixels", "Number of noisy pixels", 100, 0, 0.7e5, 100, 0, 0.7e5);
  int nBinsLinks(82);
  TH2D *hGoodLinks = new TH2D("hGoodLinks", "Number of good links", nBinsLinks, 0, 328, nBinsLinks, 0, 328);
  TH2D *hStatus4Links = new TH2D("hStatus4Links", "Number of Status 4 links", nBinsLinks, 0, 328, nBinsLinks, 0, 328);
  TH2D *hStatus5Links = new TH2D("hStatus5Links", "Number of Status 5 links", nBinsLinks, 0, 328, nBinsLinks, 0, 328);
  TH2D *hStatus9Links = new TH2D("hStatus9Links", "Number of Status 9 links", nBinsLinks, 0, 328, nBinsLinks, 0, 328);

  TH1D *h1GoodLinks  = new TH1D("h1GoodLinks", Form("Good links: %s - %s", hash1.c_str(), hash2.c_str()), 3000, 3300, 6300);
  TH1D *h1NoisyPixels  = new TH1D("h1NoisyPixels", Form("Noisy pixels: %s - %s", hash1.c_str(), hash2.c_str()), 3000, 3300, 6300);
  TH1D *h1Status4Links  = new TH1D("h1Status4Links", Form("Status 4 links: %s - %s", hash1.c_str(), hash2.c_str()), 3000, 3300, 6300);
  TH1D *h1Status5Links  = new TH1D("h1Status5Links", Form("Status 5 links: %s - %s", hash1.c_str(), hash2.c_str()), 3000, 3300, 6300);
  TH1D *h1Status9Links  = new TH1D("h1Status9Links", Form("Status 9 links: %s - %s", hash1.c_str(), hash2.c_str()), 3000, 3300, 6300);

  hGoodPixels->SetStats(0);
  hNoisyPixels->SetStats(0);
  hGoodLinks->SetStats(0);
  hStatus4Links->SetStats(0);
  hStatus5Links->SetStats(0);
  hStatus9Links->SetStats(0);

  hGoodPixels->GetXaxis()->SetTitle(hash1.c_str());
  hGoodPixels->GetYaxis()->SetTitle(hash2.c_str());
  hGoodPixels->GetZaxis()->SetTitle("Good pixels");
  hNoisyPixels->GetXaxis()->SetTitle(hash1.c_str());
  hNoisyPixels->GetYaxis()->SetTitle(hash2.c_str());
  hNoisyPixels->GetZaxis()->SetTitle("Noisy pixels");
  hGoodLinks->GetXaxis()->SetTitle(hash1.c_str());
  hGoodLinks->GetYaxis()->SetTitle(hash2.c_str());
  hGoodLinks->GetZaxis()->SetTitle("Good links");

  hStatus4Links->GetXaxis()->SetTitle(hash1.c_str());
  hStatus4Links->GetYaxis()->SetTitle(hash2.c_str());
  hStatus4Links->GetZaxis()->SetTitle("Status 4 links");
  hStatus5Links->GetXaxis()->SetTitle(hash1.c_str());
  hStatus5Links->GetYaxis()->SetTitle(hash2.c_str());
  hStatus5Links->GetZaxis()->SetTitle("Status 5 links");
  hStatus9Links->GetXaxis()->SetTitle(hash1.c_str());
  hStatus9Links->GetYaxis()->SetTitle(hash2.c_str());
  hStatus9Links->GetZaxis()->SetTitle("Status 9 links");



  for (auto it: vRuns) {
    string sRun = "_iov_" + to_string(it);
    auto firstPayload = find_if(vFirstPayloads.begin(), vFirstPayloads.end(), 
                                [&sRun](const string& s) { return s.find(sRun) != string::npos; }); 
    auto secondPayload = find_if(vSecondPayloads.begin(), vSecondPayloads.end(), 
                                [&sRun](const string& s) { return s.find(sRun) != string::npos; });
    if ((firstPayload != vFirstPayloads.end()) && (secondPayload != vSecondPayloads.end())) {
    //  cout << "run = " << it << " with payloads " << *firstPayload << " and " << *secondPayload << endl;
    } else {
      cout << "run = " << it << " not found" << endl;
      continue;
    }  

    calPixelQualityLM *pPQ1 = new calPixelQualityLM(); 
    string sFirstPayloadHash = (*firstPayload).substr((*firstPayload).find_last_of("/")+1);
    pPQ1->readPayloadFromFile(sFirstPayloadHash, sFirstPayloadDir);
    pPQ1->calculate(sFirstPayloadHash);
    pPQ1->resetIterator();
    map<string, int> payloadStats1 = getPayloadStats(pPQ1);

    calPixelQualityLM *pPQ2 = new calPixelQualityLM(); 
    string sSecondPayloadHash = (*secondPayload).substr((*secondPayload).find_last_of("/")+1);
    pPQ2->readPayloadFromFile(sSecondPayloadHash, sSecondPayloadDir);
    pPQ2->calculate(sSecondPayloadHash);
    pPQ2->resetIterator();
    map<string, int> payloadStats2 = getPayloadStats(pPQ2);

    cout << "run = " << it << " payload stats 1: " << payloadStats1["nGoodPixels"] << " good pixels, " << payloadStats1["nNoisyPixels"] << " noisy pixels, " 
         << payloadStats1["nGoodLinks"] << " good links, " << payloadStats1["nStatus4Links"] << " status4 links, " << payloadStats1["nStatus5Links"] << " status5 links, " << payloadStats1["nStatus9Links"] << " status9 links" << endl;
    cout << "run = " << it << " payload stats 2: " << payloadStats2["nGoodPixels"] << " good pixels, " << payloadStats2["nNoisyPixels"] << " noisy pixels, " 
         << payloadStats2["nGoodLinks"] << " good links, " << payloadStats2["nStatus4Links"] << " status4 links, " << payloadStats2["nStatus5Links"] << " status5 links, " << payloadStats2["nStatus9Links"] << " status9 links" << endl;
  
    hGoodPixels->Fill(static_cast<double>(payloadStats1["nGoodPixels"]), static_cast<double>(payloadStats2["nGoodPixels"]));
    hNoisyPixels->Fill(static_cast<double>(payloadStats1["nNoisyPixels"]), static_cast<double>(payloadStats2["nNoisyPixels"]));
    hGoodLinks->Fill(static_cast<double>(payloadStats1["nGoodLinks"]), static_cast<double>(payloadStats2["nGoodLinks"]));
    hStatus4Links->Fill(static_cast<double>(payloadStats1["nStatus4Links"]), static_cast<double>(payloadStats2["nStatus4Links"]));
    hStatus5Links->Fill(static_cast<double>(payloadStats1["nStatus5Links"]), static_cast<double>(payloadStats2["nStatus5Links"]));
    hStatus9Links->Fill(static_cast<double>(payloadStats1["nStatus9Links"]), static_cast<double>(payloadStats2["nStatus9Links"]));

    h1GoodLinks->Fill(static_cast<int>(it), static_cast<double>(payloadStats1["nGoodLinks"] - payloadStats2["nGoodLinks"]));
    h1NoisyPixels->Fill(static_cast<int>(it), static_cast<double>(payloadStats1["nNoisyPixels"] - payloadStats2["nNoisyPixels"]));
    h1Status4Links->Fill(static_cast<int>(it), static_cast<double>(payloadStats1["nStatus4Links"] - payloadStats2["nStatus4Links"]));
    h1Status5Links->Fill(static_cast<int>(it), static_cast<double>(payloadStats1["nStatus5Links"] - payloadStats2["nStatus5Links"]));
    h1Status9Links->Fill(static_cast<int>(it), static_cast<double>(payloadStats1["nStatus9Links"] - payloadStats2["nStatus9Links"]));

    delete pPQ1;
    delete pPQ2;
  }

  pFile->Write();
  pFile->Close();

  plotHistograms("diffPayloads.root");

  return 0;
}
