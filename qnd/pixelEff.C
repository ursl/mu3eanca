 
// ----------------------------------------------------------------------
double f_pow18(double *x, double *par) {
  return TMath::Power(x[0], 18);
}

// ----------------------------------------------------------------------
double f_pow15(double *x, double *par) {
  return TMath::Power(x[0], 15);
}

// ----------------------------------------------------------------------
double f_pow9(double *x, double *par) {
  return TMath::Power(x[0], 9);
}
 

// ----------------------------------------------------------------------
void pixelEff() {
  c0.Clear();
//  c0.SetCanvasSize(1200, 600);

  TF1 *F1 = new TF1("F1", f_pow18, 0., 1., 0);
  F1->SetLineColor(kBlack);
  F1->SetLineWidth(3);
  F1->SetNpx(10000);

  TF1 *F2 = new TF1("F2", f_pow15, 0., 1., 0);
  F2->SetLineColor(kGreen+1);
  F2->SetLineWidth(3);
  F2->SetNpx(10000);

  TF1 *F3 = new TF1("F3", f_pow9, 0., 1., 0);
  F3->SetLineColor(kBlue+1);
  F3->SetLineWidth(3);
  F3->SetNpx(10000);
  
  TH1D *h = new TH1D("h", "", 100, 0.9, 1.);
    
  setTitles(h, "avg. pixel eff.", "overall eff. scale factor", 0.05, 0.95, 1.2, 0.035);
  h->SetNdivisions(-10410, "X");

  gStyle->SetOptStat(0);
  gPad->SetGridx(1);
  gPad->SetGridy(1);
  shrinkPad(0.1, 0.15, 0.02, 0.02);
  h->Draw("");
  h->SetAxisRange(0.001, 1., "Y");
  F1->DrawCopy("same");
  F2->DrawCopy("same");
  F3->DrawCopy("same");

  tl->SetTextColor(kBlack);
  tl->DrawLatexNDC(0.38, 0.30, "#approx#kern[0.2]{#varepsilon}^{18}");

  tl->SetTextColor(kGreen+1);
  tl->DrawLatexNDC(0.30, 0.39, "#approx#kern[0.2]{#varepsilon}^{15}");

  tl->SetTextColor(kBlue+1);
  tl->DrawLatexNDC(0.31, 0.56, "#approx#kern[0.2]{#varepsilon}^{9}");
  

  TH1D *i = new TH1D("i", "", 100, 0.98, 1.);
  setTitles(i, "avg pixel eff.", "overall eff. scale factor", 4, 1.2, 1.2, 0.06);

  TPad *p1 = new TPad("p1","p1", .152, .66, .56, .976);
  p1->Draw();
  p1->cd();
  gPad->SetBottomMargin(0.15);
  gPad->SetLeftMargin(0.1);
  gPad->SetRightMargin(0.03);
  gPad->SetTopMargin(0.03);


  p1->SetGridx(1);
  p1->SetGridy(1);
  
  i->Draw("");
  i->SetAxisRange(0.36, 1., "Y");
  F1->SetLineWidth(2);
  F1->DrawCopy("same");

  F2->SetLineWidth(2);
  F2->DrawCopy("same");

  F3->SetLineWidth(2);
  F3->DrawCopy("same");

  c0.SaveAs("pixel-global-Eff.pdf");
  c0.SaveAs("pixel-global-Eff.png");

}
