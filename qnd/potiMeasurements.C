#include <TVector.h>

// ----------------------------------------------------------------------
void potiMeasurements(double p0 = -1., double p1 = -1.) {
  TF1 *f1 = new TF1("f1", "pol1", 0., 2.);

  vector<double> errX = {0.001, 0.001, 0.001, 0.001, 0.001, 0.001};
  vector<double> errM = {0.004, 0.004, 0.004, 0.004, 0.004, 0.004};
  vector<double> errP = {0.05,  0.05,  0.05,  0.05,  0.05,  0.05};
  vector<double> errT = {0.004, 0.004, 0.004, 0.004, 0.004, 0.004};
  TVectorD tex(errX.size(), &errX[0]);  
  TVectorD tem(errM.size(), &errM[0]);  
  TVectorD tep(errP.size(), &errP[0]);  
  TVectorD tet(errT.size(), &errT[0]);  

  // -- M1 Measurement 1 with 10%
  vector<double> x1 = {0.122, 0.24, 0.344, 0.561, 0.83, 1.05};
  vector<double> M1 = {0.05,  0.05, 0.05,  0.05,  0.05, 0.08};
  vector<double> P1 = {0.3,    0.4, 0.55,  0.68,  0.9,  1.34};
  vector<double> T1 = {0.24,  0.25, 0.27,  0.29,  0.32, 0.35};

  TVectorD tx1(x1.size(), &x1[0]);  
  TVectorD tm1(M1.size(), &M1[0]);  
  TVectorD tp1(P1.size(), &P1[0]);  
  TVectorD tt1(T1.size(), &T1[0]);  

  TGraphErrors *gm1 = new TGraphErrors(tx1, tm1, tex, tem); setGraph(gm1, kBlack, 20, 1., 2.);
  TGraphErrors *gp1 = new TGraphErrors(tx1, tp1, tex, tep); setGraph(gp1, kBlue, 20, 1., 2.);
  TGraphErrors *gt1 = new TGraphErrors(tx1, tt1, tex, tet); setGraph(gt1, kRed, 20, 1., 2.);

  // -- M2 Measurement 2 with 5%
  vector<double> x2 = {0.07, 0.233, 0.435, 0.669, 0.816, 1.002};
  vector<double> M2 = {0.05, 0.05, 0.05, 0.05, 0.05, 0.07};
  vector<double> P2 = {0.25, 0.425, 0.6, 0.8, 1.0, 1.2};
  vector<double> T2 = { 0.23, 0.25, 0.27, 0.30, 0.32, 0.34};

  TVectorD tx2(x2.size(), &x2[0]);  
  TVectorD tm2(M2.size(), &M2[0]);  
  TVectorD tp2(P2.size(), &P2[0]);  
  TVectorD tt2(T2.size(), &T2[0]);  

  TGraphErrors *gm2 = new TGraphErrors(tx2, tm2, tex, tem); setGraph(gm2, kBlack, 21, 1., 2., kDashed);
  TGraphErrors *gp2 = new TGraphErrors(tx2, tp2, tex, tep); setGraph(gp2, kBlue, 21, 1., 2., kDashed);
  TGraphErrors *gt2 = new TGraphErrors(tx2, tt2, tex, tet); setGraph(gt2, kRed, 21, 1., 2., kDashed);


  // -- M3 Measurement 3 with 0%
  vector<double> x3 = {0.248, /*0.345,*/ 0.452, 0.554, 0.711, 0.88, 1.124};
  vector<double> M3 = { 0.05, /*0.05,*/ 0.05, 0.05, 0.05, 0.05, 0.08};
  vector<double> P3 = { 0.4, /*0.6,*/ 0.7, 0.8, 0.9, 1.1, 1.3};
  vector<double> T3 = {0.25, /*0.26,*/ 0.27, 0.28, 0.30, 0.32, 0.35};


  TVectorD tx3(x3.size(), &x3[0]);  
  TVectorD tm3(M3.size(), &M3[0]);  
  TVectorD tp3(P3.size(), &P3[0]);  
  TVectorD tt3(T3.size(), &T3[0]);  

  TGraphErrors *gm3 = new TGraphErrors(tx3, tm3, tex, tem); setGraph(gm3, kBlack, 22, 1., 2., kDotted);
  TGraphErrors *gp3 = new TGraphErrors(tx3, tp3, tex, tep); setGraph(gp3, kBlue, 22, 1., 2., kDotted);
  TGraphErrors *gt3 = new TGraphErrors(tx3, tt3, tex, tet); setGraph(gt3, kRed, 22, 1., 2., kDotted);


  // -- M4 Measurement 4 with -5%
  vector<double> x4 = {0.221, 0.39, 0.482, /*0.62,*/ 0.782, 0.973, 1.165};
  vector<double> M4 = { 0.05, 0.05, 0.05, /*0.05,*/ 0.05, 0.05, 0.08};
  vector<double> P4 = {0.5, 0.6, 0.7, /*0.9,*/ 1.1, 1.2, 1.4};
  vector<double> T4 = {0.24, 0.26, 0.27, /*0.29,*/ 0.30, 0.33, 0.35};

  TVectorD tx4(x4.size(), &x4[0]);  
  TVectorD tm4(M4.size(), &M4[0]);  
  TVectorD tp4(P4.size(), &P4[0]);  
  TVectorD tt4(T4.size(), &T4[0]);  

  TGraphErrors *gm4 = new TGraphErrors(tx4, tm4, tex, tem); setGraph(gm4, kBlack, 23, 1., 2., 5);
  TGraphErrors *gp4 = new TGraphErrors(tx4, tp4, tex, tep); setGraph(gp4, kBlue, 23, 1., 2., 5);
  TGraphErrors *gt4 = new TGraphErrors(tx4, tt4, tex, tet); setGraph(gt4, kRed, 23, 1., 2., 5);



  // -- M5 Measurement 5 with -9%
  vector<double> x5 = {0.244, 0.384, 0.462,  0.64, 0.824, 1.185};
  vector<double> M5 = { 0.05, 0.05, 0.05, 0.05, 0.05, 0.05, 0.07};
  vector<double> P5 = {0.5, 0.65, 0.7, 0.8, 1.1, 1.5};
  vector<double> T5 = {0.25, 0.26, 0.27, 0.29, 0.31, 0.34};

  TVectorD tx5(x5.size(), &x5[0]);  
  TVectorD tm5(M5.size(), &M5[0]);  
  TVectorD tp5(P5.size(), &P5[0]);  
  TVectorD tt5(T5.size(), &T5[0]);  

  TGraphErrors *gm5 = new TGraphErrors(tx5, tm5, tex, tem); setGraph(gm5, kBlack, 24, 1., 2., 6);
  TGraphErrors *gp5 = new TGraphErrors(tx5, tp5, tex, tep); setGraph(gp5, kBlue, 24, 1., 2., 6);
  TGraphErrors *gt5 = new TGraphErrors(tx5, tt5, tex, tet); setGraph(gt5, kRed, 24, 1., 2., 6);


  // -- Plot things
  gm1->SetTitle("bla");
  gm1->GetXaxis()->SetLimits(0., 1.2);
  gm1->GetYaxis()->SetLimits(0., 1.5);

  zone();
  shrinkPad(0.12, 0.12);

  TH1F *h = c0.DrawFrame(0., 0., 1.2, 1.5, "");
  setTitles(h, "DMM current [A]", "current [A]", 0.05, 1.1, 1.2);


  gm1->Draw("l");
  gp1->Draw("l");
  gt1->Draw("l");

  gm2->Draw("l");
  gp2->Draw("l");
  gt2->Draw("l");

  gm3->Draw("l");
  gp3->Draw("l");
  gt3->Draw("l");

  gm4->Draw("l");
  gp4->Draw("l");
  gt4->Draw("l");

  gm5->Draw("l");
  gp5->Draw("l");
  gt5->Draw("l");

  tl->SetTextSize(0.03); 
  tl->SetTextColor(kBlue); 
  tl->DrawLatexNDC(0.2, 0.80, "PDCC current");
  tl->SetTextColor(kRed); 
  tl->DrawLatexNDC(0.2, 0.75, "TDK current");
  tl->SetTextColor(kBlack); 
  tl->DrawLatexNDC(0.2, 0.70, "MPDC current");

  TLegend *leg = newLegend(0.7, 0.4, 0.96, 0.6);
  leg->AddEntry(gm1, "+10%", "l");
  leg->AddEntry(gm2, "+5%", "l");
  leg->AddEntry(gm3, "0%", "l");
  leg->AddEntry(gm4, "-5%", "l");
  leg->AddEntry(gm5, "-9%", "l");

  leg->Draw();

  c0.SaveAs("potiMeasurements.pdf");

  // -- Fit TDK individually
  c0.Clear();
  gStyle->SetOptFit(111);
  zone(2,3);

  shrinkPad(0.12, 0.12);
  h = c0.DrawFrame(0., 0.2, 1.2, 0.4, "+10%");
  setTitles(h, "DMM current [A]", "TDK current [A]", 0.05, 1.0, 1.1);
  gt1->Draw("p");
  if (p0 > 0) f1->FixParameter(0, p0);
  if (p1 > 0) f1->FixParameter(1, p1);
  gt1->Fit("f1");
  gt1->GetFunction("f1")->SetLineStyle(gt1->GetLineStyle());

  c0.cd(2);shrinkPad(0.12, 0.12);
  h = c0.DrawFrame(0., 0.2, 1.2, 0.4, "+5%");
  setTitles(h, "DMM current [A]", "TDK current [A]", 0.05, 1.0, 1.1);
  gt2->Draw("p");
  if (p0 > 0) f1->FixParameter(0, p0);
  if (p1 > 0) f1->FixParameter(1, p1);
  gt2->Fit("f1");
  gt2->GetFunction("f1")->SetLineStyle(gt2->GetLineStyle());

  c0.cd(3);shrinkPad(0.12, 0.12);
  h = c0.DrawFrame(0., 0.2, 1.2, 0.4, "0%");
  setTitles(h, "DMM current [A]", "TDK current [A]", 0.05, 1.0, 1.1);
  gt3->Draw("p");
  if (p0 > 0) f1->FixParameter(0, p0);
  if (p1 > 0) f1->FixParameter(1, p1);
  gt3->Fit("f1");
  gt3->GetFunction("f1")->SetLineStyle(gt3->GetLineStyle());

  c0.cd(4);shrinkPad(0.12, 0.12);
  h = c0.DrawFrame(0., 0.2, 1.2, 0.4, "-5%");
  setTitles(h, "DMM current [A]", "TDK current [A]", 0.05, 1.0, 1.1);
  gt4->Draw("p");
  if (p0 > 0) f1->FixParameter(0, p0);
  if (p1 > 0) f1->FixParameter(1, p1);
  gt4->Fit("f1");
  gt4->GetFunction("f1")->SetLineStyle(gt4->GetLineStyle());

  c0.cd(5);shrinkPad(0.12, 0.12);
  h = c0.DrawFrame(0., 0.2, 1.2, 0.4, "-9%");
  setTitles(h, "DMM current [A]", "TDK current [A]", 0.05, 1.0, 1.1);
  gt5->Draw("p");
  if (p0 > 0) f1->FixParameter(0, p0);
  if (p1 > 0) f1->FixParameter(1, p1);
  gt5->Fit("f1");
  gt5->GetFunction("f1")->SetLineStyle(gt5->GetLineStyle());

  c0.cd(6);shrinkPad(0.12, 0.12);
  tl->SetTextColor(kBlack);
  tl->SetTextSize(0.06);
  tl->DrawLatex(0.2, 0.2, Form("Error on TDK current: %5.3f A", errT[0]));
  tl->DrawLatex(0.2, 0.14, "(arbitrary)");

  c0.SaveAs("potMeasurementsFitTDK.pdf");


  // -- PDCC Fit individually
  c0.Clear();
  gStyle->SetOptFit(111);
  zone(2,3);

  shrinkPad(0.12, 0.12);
  h = c0.DrawFrame(0., 0., 1.2, 1.6, "+10%");
  setTitles(h, "DMM current [A]", "PDCC current [A]", 0.05, 1.0, 1.1);
  gp1->Draw("p");
  if (p0 > 0) f1->FixParameter(0, p0);
  if (p1 > 0) f1->FixParameter(1, p1);
  gp1->Fit("f1");
  gp1->GetFunction("f1")->SetLineStyle(gp1->GetLineStyle());
  gp1->GetFunction("f1")->SetLineColor(kBlue);

  c0.cd(2);shrinkPad(0.12, 0.12);
  h = c0.DrawFrame(0., 0., 1.2, 1.6, "+5%");
  setTitles(h, "DMM current [A]", "PDCC current [A]", 0.05, 1.0, 1.1);
  gp2->Draw("p");
  if (p0 > 0) f1->FixParameter(0, p0);
  if (p1 > 0) f1->FixParameter(1, p1);
  gp2->Fit("f1");
  gp2->GetFunction("f1")->SetLineStyle(gp2->GetLineStyle());
  gp2->GetFunction("f1")->SetLineColor(kBlue);

  c0.cd(3);shrinkPad(0.12, 0.12);
  h = c0.DrawFrame(0., 0., 1.2, 1.6, "0%");
  setTitles(h, "DMM current [A]", "PDCC current [A]", 0.05, 1.0, 1.1);
  gp3->Draw("p");
  if (p0 > 0) f1->FixParameter(0, p0);
  if (p1 > 0) f1->FixParameter(1, p1);
  gp3->Fit("f1");
  gp3->GetFunction("f1")->SetLineStyle(gp3->GetLineStyle());
  gp3->GetFunction("f1")->SetLineColor(kBlue);

  c0.cd(4);shrinkPad(0.12, 0.12);
  h = c0.DrawFrame(0., 0., 1.2, 1.6, "-5%");
  setTitles(h, "DMM current [A]", "PDCC current [A]", 0.05, 1.0, 1.1);
  gp4->Draw("p");
  if (p0 > 0) f1->FixParameter(0, p0);
  if (p1 > 0) f1->FixParameter(1, p1);
  gp4->Fit("f1");
  gp4->GetFunction("f1")->SetLineStyle(gp4->GetLineStyle());
  gp4->GetFunction("f1")->SetLineColor(kBlue);

  c0.cd(5); shrinkPad(0.12, 0.12);
  h = c0.DrawFrame(0., 0., 1.2, 1.6, "-9%");
  setTitles(h, "DMM current [A]", "PDCC current [A]", 0.05, 1.0, 1.1);
  gp5->Draw("p");
  if (p0 > 0) f1->FixParameter(0, p0);
  if (p1 > 0) f1->FixParameter(1, p1);
  gp5->Fit("f1");
  gp5->GetFunction("f1")->SetLineStyle(gp5->GetLineStyle());
  gp5->GetFunction("f1")->SetLineColor(kBlue);


  c0.cd(6); 
  tl->SetTextColor(kBlack);
  tl->SetTextSize(0.06);
  tl->DrawLatex(0.2, 0.2, Form("Error on PDCC current: %5.3f A", errP[0]));
  tl->DrawLatex(0.2, 0.14, "(arbitrary)");
  c0.SaveAs("potMeasurementsFitPDCC.pdf");

}
