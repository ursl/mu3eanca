
#define MMUON      105.7
#define MELECTRON  0.5

// ----------------------------------------------------------------------
double f_michel(double *x, double *par) {
  double eps = 2.*x[0]/(MMUON);
  double result = 0.;
  if (eps < 1.) {
    result = 2*eps*eps*(3. - 2.*eps);
  }
  double integral = 52850.;

  return 1.e3*result/integral;
}

// ----------------------------------------------------------------------
double f_eAtomic(double *x, double *par) {
  // double hbarOverE = 4.1086e-3;
  // double bohrR = 4. * TMath::Pi() * 8.854187e-12 * hbarOverE * hbarOverE / MELECTRON;
  // double bohrR3 = bohrR*bohrR*bohrR;
  double r = MELECTRON/MMUON;
  double rinf = 13.61;
  double denominator  = 1. + (1 + r)*(1 + r)*x[0]/rinf;
  double denominator4 = denominator*denominator*denominator*denominator;
  double numerator    = (16./TMath::Pi())*TMath::Sqrt((1+r)*(1+r)*x[0]/rinf) * (1 + r)*(1+r)/rinf;
  double result = numerator/denominator4;

  return result;
}


// ----------------------------------------------------------------------
void eMuonic() {
  shrinkPad(0.1, 0.15);
  TF1 *fm = new TF1("fm", f_michel, 0., 60., 0); fm->SetLineColor(kBlue);
  fm->SetTitle("");
  fm->Draw();
  fm->GetXaxis()->SetTitle("E_{e}^{muonic} [MeV]");
  fm->GetYaxis()->SetTitle("Probability density");
  //  cout << fm->Integral(0., 60.) << endl;
  c0.SaveAs("muamu-eMuonic.pdf");
  TH1D *h1  = new TH1D("emuonic", "", 120, 0., 60.);
  for (int i = 0; i < 10000; ++i) h1->Fill(fm->GetRandom());
  cout << h1->GetMean() << endl;
}

// ----------------------------------------------------------------------
void eAtomic() {
  shrinkPad(0.1, 0.15);
  TF1 *fa = new TF1("fa", f_eAtomic, 0., 100., 0); fa->SetLineColor(kBlue);
  fa->SetTitle("");;
  fa->Draw();
  fa->GetXaxis()->SetTitle("E_{e}^{atomic} [eV]");
  fa->GetYaxis()->SetTitle("Probability density");
  //  cout << fm->Integral(0., 60.) << endl;
  c0.SaveAs("muamu-eAtomic.pdf");

}
