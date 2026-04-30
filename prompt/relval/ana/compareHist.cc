#include "compareHist.hh"

#include <iostream>
#include <filesystem>

#include <TCanvas.h>
#include <TLatex.h>
#include <TPave.h>
#include <TPaveText.h>

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

  fHistDecos = {
    {kSolid, kBlue, 1, 20, 1},
    {kDashed, kRed, 1, 21, 1}
  };


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
    {"hp", nullptr, nullptr, false, false, true},
    {"hperr2", nullptr, nullptr, false, false, true},
    {"hchi2", nullptr, nullptr, false, false, true},
    {"hlam01", nullptr, nullptr, false, false, true},
    {"htan01", nullptr, nullptr, false, false, true},
    {"hchi2", nullptr, nullptr, false, false, true},
    {"hlam01", nullptr, nullptr, false, false, true},
    {"hnhit", nullptr, nullptr, false, false, true},
    {"httype", nullptr, nullptr, false, false, true},
    {"hn_shared_hits", nullptr, nullptr, false, false, true},
    {"hn_shared_segs", nullptr, nullptr, false, false, true},
    {"hx0", nullptr, nullptr, false, false, true},
    {"hy0", nullptr, nullptr, false, false, true},
    {"hz0", nullptr, nullptr, false, false, true},
    {"hn", nullptr, nullptr, false, true, true},
    {"hn4", nullptr, nullptr, false, true, true},
    {"hn6", nullptr, nullptr, false, true, true}
  };

  for (auto plot : plots) {
    plot.h1 = getHist(fInFile1, plot.name);
    plot.h2 = getHist(fInFile2, plot.name);
    if (plot.h1 && plot.h2) {
      plot.valid = true;
      fPlots[plot.name] = plot;
    }
  }

  TH1 *hinfo1 = getHist(fInFile1, "hinfo");
  TH1 *hinfo2 = getHist(fInFile2, "hinfo");
  if (hinfo1 && hinfo2) {
    fEvents1 = hinfo1->GetBinContent(2);
    fEvents2 = hinfo2->GetBinContent(2);
    fHiTracks1 = hinfo1->GetBinContent(3);
    fHiTracks2 = hinfo2->GetBinContent(3);
    fSetup1 = hinfo1->GetXaxis()->GetBinLabel(1);
    fSetup2 = hinfo2->GetXaxis()->GetBinLabel(1);
  }
  cout << "compareHist::setupHists() SETUP1 = " << fSetup1
       << " SETUP2 = " << fSetup2
       << " fEvents1 = " << fEvents1
       << " fEvents2 = " << fEvents2
       << " fHiTracks1 = " << fHiTracks1
       << " fHiTracks2 = " << fHiTracks2 << endl;

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
  }
  cout << "compareHist::run() done" << endl;
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
  fCanvas->Clear();
  decorateHist(plot.h1, fHistDecos[0]);
  decorateHist(plot.h2, fHistDecos[1]);
  if (plot.logY) {
    fCanvas->SetLogy();
    plot.h1->SetMinimum(0.5);
    plot.h2->SetMinimum(0.5);
  } else {
    fCanvas->SetLogy(0);
    plot.h1->SetMinimum(0.);
    plot.h2->SetMinimum(0.);
  }

  if (plot.h1->GetMaximum() < plot.h2->GetMaximum()) {
    plot.h1->SetMaximum(1.1*plot.h2->GetMaximum());
  }

  plot.h1->SetStats(0);
  plot.h1->Draw();
  plot.h2->Draw("same");
  if (plot.statsBox) addDoubleStatsBox(plot);
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


// ----------------------------------------------------------------------
void compareHist::makeSummaryPDF(std::string dirname) {

}

// ----------------------------------------------------------------------
void compareHist::decorateHist(TH1 *h, const histDeco &deco) {
  h->SetLineStyle(deco.lineStyle);
  h->SetLineColor(deco.lineColor);
  h->SetLineWidth(deco.lineWidth);
  h->SetMarkerStyle(deco.markerStyle);
  h->SetMarkerSize(deco.markerSize);
}

// ----------------------------------------------------------------------
void compareHist::addDoubleStatsBox(const plotInfo &plot) {
  if (!plot.valid) return;

  // Box geometry in NDC coordinates.
  const double x1 = 0.60;
  const double y1 = 0.75;
  const double x2 = 0.95;
  const double y2 = 0.95;

  TPave *box = new TPave(x1, y1, x2, y2, 0, "NDC");
  box->SetFillColor(kWhite);
  box->SetFillStyle(0);
  box->SetLineColor(kBlack);
  box->SetLineWidth(1);
  box->SetShadowColor(0); // no shadow
  box->Draw();

  TLatex txt;
  txt.SetNDC(true);
  txt.SetTextFont(42);
  txt.SetTextSize(0.024);
  txt.SetTextAlign(12); // left aligned

  // Three rows in the stats box.
  const double y_offset = 0.03;
  const double y_entries = 0.865;
  const double y_mean = y_entries - y_offset;
  const double y_rms = y_mean - y_offset;
  const double y_overflow = y_rms - y_offset;
  const double y_underflow = y_overflow - y_offset;

  // Column anchors.
  const double x_label = 0.63;
  const double x_h1 = 0.73;
  const double x_h2 = 0.83;

  txt.SetTextColor(kBlack);
  txt.DrawLatex(x_label, y_entries, "Entries");
  txt.DrawLatex(x_label, y_mean, "Mean");
  txt.DrawLatex(x_label, y_rms, "RMS");
  txt.DrawLatex(x_label, y_overflow, "Overflow");
  txt.DrawLatex(x_label, y_underflow, "Underflow");

  txt.SetTextColor(fHistDecos[0].lineColor);
  txt.DrawLatex(x_h1, y_entries, Form("%.0f", plot.h1->GetEntries()));
  txt.DrawLatex(x_h1, y_mean, Form("%.4g", plot.h1->GetMean()));
  txt.DrawLatex(x_h1, y_rms, Form("%.4g", plot.h1->GetRMS()));
  txt.DrawLatex(x_h1, y_overflow, Form("%.0f", plot.h1->GetBinContent(plot.h1->GetNbinsX()+1)));
  txt.DrawLatex(x_h1, y_underflow, Form("%.0f", plot.h1->GetBinContent(0)));

  txt.SetTextColor(fHistDecos[1].lineColor);
  txt.DrawLatex(x_h2, y_entries, Form("%.0f", plot.h2->GetEntries()));
  txt.DrawLatex(x_h2, y_mean, Form("%.4g", plot.h2->GetMean()));
  txt.DrawLatex(x_h2, y_rms, Form("%.4g", plot.h2->GetRMS()));
  txt.DrawLatex(x_h2, y_overflow, Form("%.0f", plot.h2->GetBinContent(plot.h2->GetNbinsX()+1)));
  txt.DrawLatex(x_h2, y_underflow, Form("%.0f", plot.h2->GetBinContent(0)));

  TLatex *tl = new TLatex();
  tl->SetTextSize(0.035);
  tl->SetTextAngle(90.);
  tl->SetTextColor(fHistDecos[0].lineColor);
  tl->DrawLatexNDC(0.93, 0.1, Form("%s", fSetup1.c_str()));
  tl->SetTextColor(fHistDecos[1].lineColor);
  tl->DrawLatexNDC(0.96, 0.1, Form("%s", fSetup2.c_str()));

  tl->SetTextAngle(0.);
  tl->SetTextSize(0.02);
  tl->SetTextColor(kBlack);
  double y_events = 0.91;
  tl->DrawLatexNDC(0.2, y_events, Form("Events: "));
  tl->SetTextColor(fHistDecos[0].lineColor);
  tl->DrawLatexNDC(0.30, y_events, Form("%.0f", fEvents1));
  tl->SetTextColor(fHistDecos[1].lineColor);
  tl->DrawLatexNDC(0.35, y_events, Form("%.0f", fEvents2));

  tl->SetTextColor(kBlack);
  tl->DrawLatexNDC(0.5, y_events, Form("Hi-tracks: "));
  tl->SetTextColor(fHistDecos[0].lineColor);
  tl->DrawLatexNDC(0.60, y_events, Form("%.0f ", fHiTracks1));
  tl->SetTextColor(fHistDecos[1].lineColor);
  tl->DrawLatexNDC(0.65, y_events, Form("%.0f ", fHiTracks2));
}