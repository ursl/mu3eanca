#include <string>
#include <TFile.h>
#include <TH1D.h>
#include <TLegend.h>
#include <TCanvas.h>

using namespace std;

string runType(int irun) {
  if (irun == 6244) return "beam 1e6 (Vertex: Tune 2)";
  if (irun == 6285) return "beam 1e5 (Vertex: Tune 2)";
  if (irun == 6286) return "beam 1e5 (Vertex: Tune 2)";
  if (irun == 6287) return "beam 1e5 (Vertex: Tune 2)";
  if (irun == 6292) return "no beam (Vertex: Tune 2)";
  if (irun == 6295) return "no beam (Vertex: Tune 2)";
  return "unknown";
}



// ---------------------------------------------------------------------
void plotNhitVsFrameNumber(string filename = "results/frameTree_run6286.default.root", bool save = true) {
  TFile *f = new TFile(filename.c_str());
  int nrun(-1);
  if (filename.find("run") != string::npos) {
    nrun = stoi(filename.substr(filename.find("run") + 3));
  }

  TH1D *hh = (TH1D*)f->Get("nHitVsFrameNumber");

  string runstring = "Run " + to_string(nrun) + ": " + runType(nrun) + "";
  hh->SetTitle(runstring.c_str());
  hh->SetTitleOffset(1.2);
  hh->SetTitleSize(0.04);
  hh->SetTitleFont(42);

  // -- set the x-axis title
  hh->GetXaxis()->SetTitle("Frame (sequential) number");
  hh->GetXaxis()->SetTitleOffset(1.2);
  hh->GetXaxis()->SetTitleSize(0.04);
  hh->GetXaxis()->SetTitleFont(42);

  // -- set the y-axis title
  hh->GetYaxis()->SetTitle("Hit multiplicity");
  hh->GetYaxis()->SetTitleOffset(1.2);
  hh->GetYaxis()->SetTitleSize(0.04);
  hh->GetYaxis()->SetTitleFont(42);

  // -- set the minimum of the histogram
  hh->SetMinimum(0.5);
  hh->DrawCopy("e");

  gPad->SetLogy();
  gStyle->SetOptStat(0);

  // -- draw the histograms
  TH1D *h = (TH1D*)f->Get("nHitVsFrameNumber");
  h->SetLineColor(kBlack);
  TH1 *hAll = h->DrawCopy("histsame");

  h = (TH1D*)f->Get("nGoodHitVsFrameNumber");
  h->SetLineColor(kGreen);
  TH1 *hGood = h->DrawCopy("histsame");

  h = (TH1D*)f->Get("nBadHitVsFrameNumber");
  h->SetLineColor(kRed);
  TH1 *hBad = h->DrawCopy("histsame");

  h = (TH1D*)f->Get("nInvalidHitVsFrameNumber");
  h->SetLineColor(kMagenta);
  TH1 *hInvalid = h->DrawCopy("histsame");

  TLegend *leg = new TLegend(0.7, 0.82, 0.96, 0.92);
  leg->AddEntry(hAll, "All hits", "l");
  leg->AddEntry(hGood, "Good hits (neither bad nor invalid)", "l");
  leg->AddEntry(hBad, "Bad hits (noisy pixels)", "l");
  leg->AddEntry(hInvalid, "Invalid hits (in trirec)", "l");
  leg->Draw();

  if (save) {
    string outfile = "nhitVsFrameNumber_run" + to_string(nrun) + ".pdf";
    c0.SaveAs(outfile.c_str());
  }

  delete f;
}

// ---------------------------------------------------------------------
void plotAll() {
  c0.Divide(2,3);
  c0.cd(1);
  plotNhitVsFrameNumber("results/frameTree_run6244.default.root", false);
  c0.cd(2);
  plotNhitVsFrameNumber("results/frameTree_run6285.default.root", false);
  c0.cd(3);
  plotNhitVsFrameNumber("results/frameTree_run6286.default.root", false);
  c0.cd(4);
  plotNhitVsFrameNumber("results/frameTree_run6287.default.root", false);
  c0.cd(5);
  plotNhitVsFrameNumber("results/frameTree_run6292.default.root", false);
  c0.cd(6);
  plotNhitVsFrameNumber("results/frameTree_run6295.default.root", false);

  c0.SaveAs("nhitVsFrameNumber-allRuns.pdf");
}