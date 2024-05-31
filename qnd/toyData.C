#include "/Users/ursl/macros/ana/util/initFunc.hh"

#define MMUON      105.7
#define MELECTRON  0.5

// ----------------------------------------------------------------------
double f_p1(double *x, double *par) {
  return par[0] + par[1]*x[0];
}

// ----------------------------------------------------------------------
double f_gauss(double *x, double *par) {
  // par[0] -> const
  // par[1] -> mean
  // par[2] -> sigma

  if (par[2] > 0.) {
    Double_t arg = (x[0] - par[1]) / par[2];
    Double_t fitval =  par[0]*TMath::Exp(-0.5*arg*arg);
    return fitval;
  }
  else {
    return -1.;
  }
}


// ----------------------------------------------------------------------
double f_argus(double *x, double *par) {
  //   par[0] = normalization of argus
  //   par[1] = exponential factor of argus
  //   par[2] = endpoint

  // --- If tail is small then Gauss
  //  double ebeam = 10.5795/2;
  double ebeam = par[2];
  double ebeam2 = ebeam*ebeam;
  double background = 0.;
  double x2 = x[0]*x[0];
  if (x2/ebeam2 < 1.) {
    background = par[0]*x[0] * sqrt(1. - (x2/ebeam2)) * exp(par[1] * (1. - (x2/ebeam2)));
  } else {
    background = 0.;
  }
  return background;
}


// ----------------------------------------------------------------------
double f_cb(double *x, double *par) {
  // par[0]:  mean
  // par[1]:  sigma
  // par[2]:  alpha, crossover point
  // par[3]:  n, length of tail
  // par[4]:  N, normalization

  Double_t cb = 0.0;
  Double_t exponent = 0.0;

  if (x[0] > par[0] - par[2]*par[1]) {
    exponent = (x[0] - par[0])/par[1];
    cb = TMath::Exp(-exponent*exponent/2.);
  } else {
    double nenner  = TMath::Power(par[3]/par[2], par[3])*TMath::Exp(-par[2]*par[2]/2.);
    double zaehler = (par[0] - x[0])/par[1] + par[3]/par[2] - par[2];
    zaehler = TMath::Power(zaehler, par[3]);
    cb = nenner/zaehler;
  }

  if (par[4] > 0.) {
    cb *= par[4];
  }

  return cb;
}


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


// ----------------------------------------------------------------------
// -- the simplest most stupid setup for Klaus' slide in Filzbach
void toy1(int nsig = 10, double p0 = 10., double p1 = 3.) {

  TH1D *hm = new TH1D("hm", "", 40, 95., 115.);
  hm->SetBinErrorOption(TH1::kPoisson);
  hm->SetMarkerStyle(20);
  hm->SetLineColor(kBlack);
  hm->SetLineWidth(2);
  setTitles(hm, "#it{m}_{#it{e#kern[-0.9]{ }e#kern[-0.9]{ }e}} [MeV]", "Entries/Bin");

  TF1 *F1 = new TF1("F1", f_gauss, 95., 115., 3);
  TF1 *F2 = new TF1("F2", f_p1, 95., 115., 2);
  TF1 *F3 = new TF1("F3", f_argus, 95., 115., 3);
  TF1 *F4 = new TF1("F4", f_cb, 95., 115., 5);
  F4->SetFillColor(kRed);
  F4->SetFillStyle(3365);

  F3->SetLineColor(kBlack);
  F3->SetLineStyle(kDashed);
  F4->SetLineWidth(3);

  F1->SetParameters(1, 115., 2.); // detector not aligned!
  F2->SetParameters(1, 0.);
  F3->SetParameters(0.001, 10., 115.);
  F3->SetLineWidth(3);

  F4->SetParameters(105.0, 2., 0.5, 10., 1.);

  for (int i = 0; i < nsig; ++i) {
    double m = F3->GetRandom();
    hm->Fill(m);
  }

  c0.SetLogy(0);
  gStyle->SetOptStat(0);
  shrinkPad(0.12, 0.12);
  hm->SetMinimum(0.01);
  hm->Draw("ex0p");

  F4->Draw("same");
  F3->Draw("same");


  TLegend *tl = newLegend(0.4, 0.7, 0.8, 0.85);
  tl->SetHeader("wishful thinking");
  tl->AddEntry(hm, "2024 Mu3e Data", "ep");
  tl->AddEntry(F4, "#it{#mu}^{#kern[0.3]{#plus}} #rightarrow e^{#kern[0.3]{#plus}}#kern[-0.9]{ }e^{#kern[0.3]{#minus}}#kern[-0.9]{ }e^{#kern[0.3]{#plus}} (#kern[-0.5]{ }#times#kern[-0.9]{ }10^{#kern[0.3]{50}})", "f");
  tl->Draw();

  //  F4->Draw("");

  c0.SaveAs("mu3e-toydata-2024.pdf");
}
