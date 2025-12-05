#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TGraph.h"
#include "TLine.h"
#include "TText.h"
#include "TBox.h"
#include "TArrow.h"
#include "TMarker.h"
#include "TKey.h"
#include "TList.h"

// ----------------------------------------------------------------------
void plotFakeTracks(string cuts = "(TMath::Abs(p)<100) && nhit==6", 
                    string pdfname = "fakeTracks.pdf",
                    string filenameN = "run5736-normal-trirec.root", 
                    string filenameR = "run5736-random-trirec.root") {
  TFile *fN = TFile::Open(filenameN.c_str());
  TFile *fR = TFile::Open(filenameR.c_str());
  TFile *fMC = TFile::Open("mc-conf8-normal-trirec.root");
  TTree *tN = (TTree*)fN->Get("frames");
  TTree *tR = (TTree*)fR->Get("frames");
  TTree *tMC = (TTree*)fMC->Get("frames");

  TH1D *np = new TH1D("np", "p", 40, -80., 80.);
  np->SetMarkerStyle(20);
  TH1D *rp = new TH1D("rp", "p", 40, -80., 80.);
  rp->SetMarkerStyle(25);
  TH1D *dp = new TH1D("dp", "", 40, -80., 80.);
  setFilledHist(dp, kBlack, kYellow, 1000, 2);
  setTitles(dp, "p [MeV]", "Entries", 0.05, 0.95, 1.1, 0.035);

  TH1D *mp = new TH1D("mp", "p", 40, -80., 80.);
  mp->SetLineWidth(2);

  TH1D *nc = new TH1D("nc", "#chi^{2}", 30, 0., 150.);
  nc->SetMarkerStyle(20);
  TH1D *rc = new TH1D("rc", "#chi^{2}", 30, 0., 150.);
  rc->SetMarkerStyle(25);
  TH1D *dc = new TH1D("dc", "", 30, 0., 150.);
  setFilledHist(dc, kBlack, kYellow, 1000, 2);
  setTitles(dc, "#chi^{2}", "Entries", 0.05, 0.95, 1.1, 0.035);

  TH1D *mc = new TH1D("mc", "#chi^{2}", 30, 0., 150.);
  mc->SetLineWidth(2);

  tN->Draw("p>>np", cuts.c_str());
  tR->Draw("p>>rp", cuts.c_str());
  tMC->Draw("p>>mp", cuts.c_str());

  tN->Draw("chi2>>nc", cuts.c_str());
  tR->Draw("chi2>>rc", cuts.c_str());
  tMC->Draw("chi2>>mc", cuts.c_str());

  dp->Add(np, rp, 1., -1.);
  dc->Add(nc, rc, 1., -1);

  mp->Scale(dp->Integral() / mp->Integral());
  mc->Scale(dp->Integral() / mc->Integral());


  TCanvas *c = new TCanvas("c", "c", 1000, 1000);
  gStyle->SetOptStat(0);
  c->Divide(1, 2);
  c->cd(1);
  dp->SetMaximum(np->GetMaximum() * 1.1);
  dp->SetMinimum(0.);
  dp->Draw("hist");
  np->Draw("esame");
  rp->Draw("esame");
  mp->Draw("histsame");

  TLegend *tl = new TLegend(0.7, 0.7, 0.9, 0.9);
  tl->AddEntry(np, Form("Normal (N = %5.0f)", np->Integral()), "ep");
  tl->AddEntry(rp, Form("Random (N = %5.0f)", rp->Integral()), "ep");
  tl->AddEntry(dp, "Difference", "f");
  tl->AddEntry(mp, "MC (conf8)", "l");
  tl->Draw();

  c->cd(2);
  dc->SetMaximum(mc->GetMaximum() * 1.1);
  dc->SetMinimum(0.);

  dc->Draw("hist");
  nc->Draw("esame");
  rc->Draw("esame");
  mc->Draw("histsame");

  c->SaveAs(pdfname.c_str());
}
