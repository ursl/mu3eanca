#include "TCanvas.h"
#include "TEfficiency.h"
#include "TRandom3.h"
#include "TMath.h"

TEfficiency* eff(int nEvents = 10000, string name = "eff") {  
   TEfficiency* pEff = new TEfficiency(name.c_str(),Form("%s;x;#epsilon",name.c_str()),20,0,10);
   TRandom3 rand3;
 
   bool bPassed;
   double x;
   for (int i=0; i<nEvents; ++i) {
      //simulate events with variable under investigation
      x = rand3.Uniform(10);
      //check selection: bPassed = DoesEventPassSelection(x)
      bPassed = rand3.Rndm() < TMath::Gaus(x,5,4);
      pEff->Fill(bPassed,x);
   }
 
//   pEff->Draw("AP");

  return pEff;
}

void effAsymmetry(int nEvents1 = 10000, int nEvents2 = 1000000) {
  TEfficiency* pEff = eff(nEvents1, "eff1");
  TEfficiency* pEff2 = eff(nEvents2, "eff2");

  TH1D *hAsym = new TH1D("hAsym", "Asymmetry = (error low - error high) / (error low + error high)", 20, 0., 10.);
  hAsym->SetLineColor(kRed);
  TH1D *hAsym2 = new TH1D("hAsym2", "Asymmetry = (error low - error high) / (error low + error high)", 20, 0., 10.);
  hAsym2->SetLineColor(kBlack);
  hAsym->SetLineWidth(2);
  hAsym2->SetLineWidth(2);

  for (int iBin = 1; iBin <= pEff->GetTotalHistogram()->GetNbinsX(); ++iBin) {
    double effErrLow = pEff->GetEfficiencyErrorLow(iBin);
    double effErrHigh = pEff->GetEfficiencyErrorUp(iBin);

    double eff2ErrLow = pEff2->GetEfficiencyErrorLow(iBin);
    double eff2ErrHigh = pEff2->GetEfficiencyErrorUp(iBin);
    hAsym->SetBinContent(iBin, (effErrLow - effErrHigh) / (effErrLow + effErrHigh));
    hAsym2->SetBinContent(iBin, (eff2ErrLow - eff2ErrHigh) / (eff2ErrLow + eff2ErrHigh));
    cout << "bin " << iBin << " Asymmetry = " << (effErrLow - effErrHigh) / (effErrLow + effErrHigh);
    cout << " Asymmetry2 = " << (eff2ErrLow - eff2ErrHigh) / (eff2ErrLow + eff2ErrHigh) << endl;
  } 
  TCanvas *c0 = new TCanvas("c0", "c0", 1000, 500);
  c0->Clear();
  c0->Divide(2,1);

  gStyle->SetOptStat(0);

  c0->cd(1);
  pEff->SetLineColor(kBlack);
  pEff->Draw("AP");
  gPad->Update();
  pEff->GetPaintedGraph()->GetXaxis()->SetRangeUser(0., 10.);
  pEff->SetLineColor(kRed);
  pEff2->Draw("Psame");

  c0->cd(2);
  hAsym->Draw();
  hAsym2->Draw("samehist");


  TLegend *tl = new TLegend(0.7, 0.7, 0.9, 0.9);
  tl->AddEntry(hAsym, Form("Asymmetry (N=%d)", nEvents1), "l");
  tl->AddEntry(hAsym2, Form("Asymmetry (N=%d)", nEvents2), "l");
  tl->Draw();



  
  c0->SaveAs("effAsymmetry.pdf");
  c0->SaveAs("effAsymmetry.png");

}