#include "compareHist.hh"

#include <iostream>
#include <filesystem>

#include <TCanvas.h>

using namespace std;

// ----------------------------------------------------------------------
compareHist::compareHist(const std::string &infile1, const std::string &infile2) {
  fInFileName1 = infile1;
  fInFileName2 = infile2;

  cout << "compareHist::compareHist() infile1 = " << fInFileName1
       << " infile2 = " << fInFileName2
       << endl;

  fInFile1 = TFile::Open(fInFileName1.c_str());
  fInFile2 = TFile::Open(fInFileName2.c_str());

  fCanvas = new TCanvas("cmp", "cmp", 800, 600);
  fCanvas->cd();

}

// ----------------------------------------------------------------------
compareHist::~compareHist() {
  if (fInFile1) {
    fInFile1->Close();
    delete fInFile1;
  }
  if (fInFile2) {
    fInFile2->Close();
    delete fInFile2;
  }
}

// ----------------------------------------------------------------------
void compareHist::setupHists(string mode) {
  if (mode != "relval") {
    cout << "compareHist::setupHists() WARNING: mode " << mode
         << " not implemented, using relval defaults" << endl;
  }

  const vector<plotInfo> plots = {
    {"hp", nullptr, nullptr, false, false, false},
    {"hperr2", nullptr, nullptr, false, false, false},
    {"hx0", nullptr, nullptr, false, false, false},
    {"hy0", nullptr, nullptr, false, false, false},
    {"hz0", nullptr, nullptr, false, false, false},
    {"hn", nullptr, nullptr, false, false, false},
    {"hn4", nullptr, nullptr, false, false, false},
    {"hn6", nullptr, nullptr, false, false, false}
  };

  for (auto plot : plots) {
    plot.h1 = getHist(fInFile1, plot.name);
    plot.h2 = getHist(fInFile2, plot.name);
    if (plot.h1 && plot.h2) {
      plot.valid = true;
      fPlots[plot.name] = plot;
    }
  }

  TH1 *hnevents1 = getHist(fInFile1, "hnevents");
  TH1 *hnevents2 = getHist(fInFile2, "hnevents");
  if (hnevents1 && hnevents2) {
    fEvents1 = hnevents1->GetBinContent(1);
    fEvents2 = hnevents2->GetBinContent(1);
    fHiTracks1 = hnevents2->GetBinContent(2);
    fHiTracks2 = hnevents2->GetBinContent(2);
  }
  cout << "compareHist::setupHists() fEvents1 = " << fEvents1
       << " fEvents2 = " << fEvents2
       << " fHiTracks1 = " << fHiTracks1
       << " fHiTracks2 = " << fHiTracks2
       << endl;

}

// ----------------------------------------------------------------------
void compareHist::run(std::string dirname) {
  fDirName = dirname;
  std::error_code ec;
  if (!std::filesystem::exists(fDirName)) {
    if (!std::filesystem::create_directories(fDirName, ec)) {
      cout << "compareHist::run() ERROR: could not create directory "
           << fDirName << " (" << ec.message() << ")" << endl;
      return;
    }
  }

  for (const auto &kv : fPlots) {
    const plotInfo &plot = kv.second;
    if (!plot.valid) continue;
    plotOverlay(plot);
    plotDifference(plot);
    plotRatio(plot);
  }
}

// ----------------------------------------------------------------------
TH1 *compareHist::getHist(TFile *f, const std::string &name) {
  if (!f) return nullptr;
  TH1 *h = dynamic_cast<TH1 *>(f->Get(name.c_str()));
  if (!h) {
    cout << "compareHist::getHist() WARNING: missing histogram " << name
         << " in " << f->GetName() << endl;
    return nullptr;
  }
  return h;
}


// ----------------------------------------------------------------------
void compareHist::plotOverlay(const plotInfo &plot) {
  if (!plot.valid) return;

  fCanvas->cd();
  plot.h1->Draw();
  plot.h2->Draw("same");
  fCanvas->SaveAs((fDirName + "/" + plot.name + ".pdf").c_str());
}

// ----------------------------------------------------------------------
void compareHist::plotDifference(const plotInfo &plot) {
  if (!plot.valid) return;
  // Skeleton: to be implemented (h1-h2 plotting)
}

// ----------------------------------------------------------------------
void compareHist::plotRatio(const plotInfo &plot) {
  if (!plot.valid) return;
  // Skeleton: to be implemented (h1/h2 plotting)
}

