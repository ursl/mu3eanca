// ----------------------------------------------------------------------
// -- r ~/data/mu3e/mu3e-dev-smb/run-combined.root
// -- .L fibres.C
// -- zPosition()
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
void getRange(TH1D *h, double &zmin, double &zmax) {
  double con(0.), min(0.), max(0.);
  for (Int_t i = 0; i <= h->GetNbinsX()+1; ++i) {
    con = h->GetBinContent(i);
    if (con > 0.) {
      min = h->GetBinLowEdge(i);
      max = h->GetBinLowEdge(i);
      if (min < zmin) zmin = min; 
      if (max > zmax) zmax = max; 
    }
  }
}
 


// ----------------------------------------------------------------------
void zPosition() {
  gFile->cd("stat/FibreSmb");

  c0.SetCanvasSize(1800, 900);
  zone(2,1);
  gStyle->SetOptStat(0);
  TH1D *h1 = (TH1D*)gDirectory->Get("SmbNegZ");
  h1->Draw();
  double zmin(999.), zmax(-999.);
  zmin = 900, zmax = -900;
  getRange(h1, zmin, zmax);
  cout << "zmin = " << zmin << " zmax = " << zmax << " difference = " << (zmax-zmin) << endl;
  tl->DrawLatexNDC(0.35, 0.8, Form("%4.2f .. %4.2f", zmin, zmax));
  //  c0.SaveAs("fibres-zPosition-smbPosZ.pdf");

  c0.cd(2);
  h1 = (TH1D*)gDirectory->Get("SmbPosZ");
  h1->Draw();
  zmin = 999., zmax = -999.;
  getRange(h1, zmin, zmax);
  cout << "zmin = " << zmin << " zmax = " << zmax << " difference = " << (zmax-zmin) << endl;
  tl->DrawLatexNDC(0.35, 0.8, Form("%4.2f .. %4.2f", zmin, zmax));
  c0.SaveAs("fibres-zPositions.pdf");
  
  
  c0.SetCanvasSize(1800, 700);
  zone(6,2);
  for (int i = 0; i < 12; ++i) {
    h1 = (TH1D*)gDirectory->Get(Form("SmbPosZ_%d", i));
    h1->Draw();
    c0.cd(i+2);
  }
  c0.SaveAs("fibres-zPosition-all-smbPosZ.pdf");


  c0.cd(1);
  for (int i = 0; i < 12; ++i) {
    h1 = (TH1D*)gDirectory->Get(Form("SmbNegZ_%d", i));
    h1->Draw();
    c0.cd(i+2);
  }
  c0.SaveAs("fibres-zPosition-all-smbNegZ.pdf");

  
}


// ----------------------------------------------------------------------
void rphiPosition() {
  gFile->cd("stat/FibreSmb");

  gStyle->SetOptStat(0);
  
  c0.SetCanvasSize(1200, 600);
  zone(2, 1);
  c0.cd(2);
  TH2D *h2 = (TH2D*)gDirectory->Get("PlanePosZ");
  h2->Draw("axis");
  tl->SetTextSize(0.04);
  tl->SetTextColor(kBlack);
  tl->SetNDC(kFALSE);
  for (int ix = 1; ix <= h2->GetNbinsX(); ++ix) {
    for (int iy = 1; iy <= h2->GetNbinsY(); ++iy) {
      if (h2->GetBinContent(ix, iy) > -1) {
        cout << "x/y = " << h2->GetXaxis()->GetBinLowEdge(ix)
             << "/" << h2->GetYaxis()->GetBinLowEdge(iy)
             << " SMB = " << Form("%d", static_cast<int>(h2->GetBinContent(ix, iy)))
             << endl;
        tl->DrawLatex(h2->GetXaxis()->GetBinLowEdge(ix),
                      h2->GetYaxis()->GetBinLowEdge(iy),
                      Form("%d", static_cast<int>(h2->GetBinContent(ix, iy))));
      }
    }
  }

  c0.cd(1);
  TH2D *h3 = (TH2D*)gDirectory->Get("PlaneNegZ");
  h3->Draw("axis");
  for (int ix = 1; ix <= h3->GetNbinsX(); ++ix) {
    for (int iy = 1; iy <= h3->GetNbinsY(); ++iy) {
      if (h3->GetBinContent(ix, iy) > -1) {
        cout << "x/y = " << h3->GetXaxis()->GetBinLowEdge(ix)
             << "/" << h3->GetYaxis()->GetBinLowEdge(iy)
             << " SMB = " << Form("%d", static_cast<int>(h3->GetBinContent(ix, iy)))
             << endl;
        tl->DrawLatex(h3->GetXaxis()->GetBinLowEdge(ix),
                      h3->GetYaxis()->GetBinLowEdge(iy),
                      Form("%d", static_cast<int>(h3->GetBinContent(ix, iy))));
      }
    }
  }

  c0.SaveAs("fibres-rphiPosition.pdf");

  
}
