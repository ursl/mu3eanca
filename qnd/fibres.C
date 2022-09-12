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
  c0.SaveAs("zPosition-smbPosZ.pdf");


  for (int i = 0; i < 12; ++i) {
    TH1D *h1 = (TH1D*)gDirectory->Get(Form("SmbNegZ_%d", i));
    h1->Draw();
    c0.cd(i+2);
  }
  c0.SaveAs("zPosition-smbNegZ.pdf");

  
}
