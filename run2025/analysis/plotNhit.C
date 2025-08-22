#include <TH1.h>
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

vector<int> gLayer1 = {1,2,3,4,5,6,
  33, 34, 35, 36, 37, 38,
  65, 66, 67, 68, 69, 70,
  97, 98, 99, 100, 101, 102,
  129, 130, 131, 132, 133, 134,
  161, 162, 163, 164, 165, 166,
  193, 194, 195, 196, 197, 198,
  225, 226, 227, 228, 229, 230};

  vector<int> gLayer2 = {1025, 1026, 1027, 1028, 1029, 1030,
  1057, 1058, 1059, 1060, 1061, 1062,
  1089, 1090, 1091, 1092, 1093, 1094,
  1121, 1122, 1123, 1124, 1125, 1126,
  1153, 1154, 1155, 1156, 1157, 1158,
  1185, 1186, 1187, 1188, 1189, 1190,
  1217, 1218, 1219, 1220, 1221, 1222,
  1249, 1250, 1251, 1252, 1253, 1254,
  1281, 1282, 1283, 1284, 1285, 1286,
  1313, 1314, 1315, 1316, 1317, 1318};


// ---------------------------------------------------------------------
void plotVtxLayer(string filename = "results/frameTree_run6286.default.root", string batch = "vtx2D_burstGood", int layer = 1, string opt = "hist") {
  TFile *f = new TFile(filename.c_str());
  vector<int> layer2Plot; 
  c0.Clear();
  gStyle->SetOptStat(0);
  if (layer == 1) {
    layer2Plot = gLayer2;
    c0.Divide(6,8);
  } else {
    layer2Plot = gLayer1;
    c0.Divide(6,10);
  }

  for (int i = 0; i < layer2Plot.size(); ++i) {
    c0.cd(i+1);
    TH1 *h = (TH1*)f->Get(Form("vtx/%s_%d", batch.c_str(), layer2Plot[i]));
    h->Draw(opt.c_str());
  }

  c0.SaveAs(Form("vtx_layer%d_%s.pdf", layer, batch.c_str()));
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
  hh->GetYaxis()->SetTitle("Number of hits");
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