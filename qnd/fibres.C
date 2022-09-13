// ----------------------------------------------------------------------
// -- r data/mu3e_run_000780.root
// -- .L fibres.C
// -- zPosition()
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
void zPosition() {
  gFile->cd("stat/FibreSmb");

  c0.SetCanvasSize(1800, 700);
  zone(6,2);
  for (int i = 0; i < 12; ++i) {
    TH1D *h1 = (TH1D*)gDirectory->Get(Form("SmbPosZ_%d", i));
    h1->Draw();
    c0.cd(i+2);
  }
  c0.SaveAs("fibres-zPosition-smbPosZ.pdf");


  for (int i = 0; i < 12; ++i) {
    TH1D *h1 = (TH1D*)gDirectory->Get(Form("SmbNegZ_%d", i));
    h1->Draw();
    c0.cd(i+2);
  }
  c0.SaveAs("fibres-zPosition-smbNegZ.pdf");

  
}


// ----------------------------------------------------------------------
void rphiPosition() {
  gFile->cd("stat/FibreSmb");

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
        cout << "x/y = " << h2->GetXaxis()->GetBinLowEdge(ix) << "/" << h2->GetYaxis()->GetBinLowEdge(iy)
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
        cout << "x/y = " << h3->GetXaxis()->GetBinLowEdge(ix) << "/" << h3->GetYaxis()->GetBinLowEdge(iy)
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
