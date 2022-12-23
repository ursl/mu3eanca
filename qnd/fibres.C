// ----------------------------------------------------------------------
// -- r ~/data/mu3e/mu3e-dev-smb/run-combined.root
// -- .L fibres.C
// -- zPosition()
//
// NOTE: For the rphi displays you MUST use a single-run file.
//       Else you get the SUM of the IDs ...
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
void rphiPosition(string dir = "FibreSmb") {
  gFile->cd(Form("stat/%s", dir.c_str()));
  string hname = "Plane";
  tl->SetTextSize(0.04);
  if (string::npos != dir.find("MuTrig")) {
    hname = "SmbMuTrigPlane";
    tl->SetTextSize(0.02);
  }

  gStyle->SetOptStat(0);
  
  c0.SetCanvasSize(1200, 600);
  zone(2, 1);
  c0.cd(2);
  TH2D *h2 = (TH2D*)gDirectory->Get(Form("%sPosZ", hname.c_str()));
  h2->Draw("axis");
  tl->SetTextColor(kBlack);
  tl->SetNDC(kFALSE);
  for (int ix = 1; ix <= h2->GetNbinsX(); ++ix) {
    for (int iy = 1; iy <= h2->GetNbinsY(); ++iy) {
      if (h2->GetBinContent(ix, iy) > -1) {
        double phi = TVector2(ix - 0.5*h2->GetNbinsX(), iy - 0.5*h2->GetNbinsY()).Phi()*57.; 
        cout << "x/y = " << h2->GetXaxis()->GetBinLowEdge(ix)
             << "/" << h2->GetYaxis()->GetBinLowEdge(iy)
             << " SMB = " << Form("%d", static_cast<int>(h2->GetBinContent(ix, iy)))
             << " phi = " << phi
             << endl;
        tl->SetTextAngle(phi);
            
        tl->DrawLatex(h2->GetXaxis()->GetBinLowEdge(ix),
                      h2->GetYaxis()->GetBinLowEdge(iy),
                      Form("%d", static_cast<int>(h2->GetBinContent(ix, iy))));
      }
    }
  }

  c0.cd(1);
  TH2D *h3 = (TH2D*)gDirectory->Get(Form("%sNegZ", hname.c_str()));
  h3->Draw("axis");
  for (int ix = 1; ix <= h3->GetNbinsX(); ++ix) {
    for (int iy = 1; iy <= h3->GetNbinsY(); ++iy) {
      if (h3->GetBinContent(ix, iy) > -1) {
        double phi = TVector2(ix - 0.5*h2->GetNbinsX(), iy - 0.5*h2->GetNbinsY()).Phi()*57.; 
        cout << "x/y = " << h3->GetXaxis()->GetBinLowEdge(ix)
             << "/" << h3->GetYaxis()->GetBinLowEdge(iy)
             << " SMB = " << Form("%d", static_cast<int>(h3->GetBinContent(ix, iy)))
             << " phi = " << phi
             << endl;
        tl->SetTextAngle(phi);
        tl->DrawLatex(h3->GetXaxis()->GetBinLowEdge(ix),
                      h3->GetYaxis()->GetBinLowEdge(iy),
                      Form("%d", static_cast<int>(h3->GetBinContent(ix, iy))));
      }
    }
  }

  c0.SaveAs(Form("fibres-%s-rphiPosition.pdf", dir.c_str()));

  
}


// ----------------------------------------------------------------------
void addMuTRIG(string hname, string opt = "") {
  gFile->cd("stat/FibreSmbMuTrig");

  c0.SetCanvasSize(1800, 900);
  zone(2,1);
  gStyle->SetOptStat(0);
  shrinkPad(0.1, 0.1, 0.15);
  
  TH1D *h1(0), *hSumDS(0), *hSumUS(0);
  for (int i = 0; i < 12; ++i) {
    h1 = (TH1D*)gDirectory->Get(Form("DS_SMB%d_%s", i, hname.c_str()));
    if (0 == i) {
      hSumDS = h1;
      hSumDS->SetTitle(Form("DS global %s position for MuTRIGs", hname.c_str()));
    } else {
      hSumDS->Add(h1);
    }
  }

  hSumDS->Draw(opt.c_str());

  c0.cd(2);
  shrinkPad(0.1, 0.1, 0.15);
  for (int i = 0; i < 12; ++i) {
    h1 = (TH1D*)gDirectory->Get(Form("US_SMB%d_%s", i, hname.c_str()));
    if (0 == i) {
      hSumUS = h1;
      hSumUS->SetTitle(Form("US global %s position for MuTRIGs", hname.c_str()));
    } else {
      hSumUS->Add(h1);
    }
  }
  hSumUS->Draw(opt.c_str());

  c0.SaveAs(Form("fibres-mutrig-%s.pdf", hname.c_str()));
  
}

// ----------------------------------------------------------------------
void addMuTRIG() {
  addMuTRIG("gz", "");
  addMuTRIG("lxy", "colz");
  addMuTRIG("edep", "");
}



// ----------------------------------------------------------------------
void dose(double inTargetTotal = 19200000, double muonStopsPhase1 = 1.e15) {
  TH1F *h1 = (TH1F*) gFile->Get("stat/FibreSmbMuTrig/hFibreSmbDose"); 
  h1->GetXaxis()->SetTitle("");
  h1->GetYaxis()->SetTitle("Dose [Gy]");
  gStyle->SetOptStat(0);
  zone();
  shrinkPad(0.2, 0.15);
  h1->Draw();

  float dt(0.);
  sscanf(h1->GetTitle(), "Fibre Smb Dose (time = %f)", &dt);
  int NFIBERS(96);
  double njobs = h1->GetEntries()/NFIBERS;
  cout << "dt = " << dt << " total time = " << njobs*dt << endl;

  // -- remove most of the bin labels
  for (int ibin = 1; ibin <= h1->GetNbinsX(); ++ibin) {
    h1->SetBinContent(ibin, h1->GetBinContent(ibin)*(njobs*dt));
    if (1 == ibin%4) {
      string sname = h1->GetXaxis()->GetBinLabel(ibin);
      if (string::npos != sname.find("US")){
        replaceAll(sname, "US", "DS");
      } else {
        replaceAll(sname, "DS", "US");
      }
      h1->GetXaxis()->SetBinLabel(ibin, sname.c_str());
    }

    if (0 == ibin%4) h1->GetXaxis()->SetBinLabel(ibin, "");
    if (2 == ibin%4) h1->GetXaxis()->SetBinLabel(ibin, "");
    if (3 == ibin%4) h1->GetXaxis()->SetBinLabel(ibin, "");
  }

  h1->SetTitle(Form("Fibre MuTRIG Dose (time = %f sec)", njobs*dt));
  h1->Draw();

  c0.SaveAs("fibres-FibreSmbMuTrig-dose-sumjobs.pdf");

  // -- scale up to 1e15 muon stops
  double sf = muonStopsPhase1/inTargetTotal;
  h1->Scale(sf);

  double n = muonStopsPhase1; 
  double expoN  = TMath::Log10(n);
  cout << "original expoN = " << expoN << endl;

  double mantN  = n / TMath::Power(10, static_cast<int>(expoN));
  cout << "original mantN = " << mantN << endl;

  h1->SetTitle(Form("Fibre MuTRIG Dose (n_{#mu}^{stop} = %1.0fE%2.0f)", mantN, expoN));
  h1->Draw("hist");
  
  c0.SaveAs("fibres-FibreSmbMuTrig-dose-phase1.pdf");

}

// ----------------------------------------------------------------------
void dose2(double inTargetTotal = 25600000, double muonStopsPhase1 = 2.6e15) {
  TH1F *h1 = (TH1F*) gFile->Get("stat/FibreSmbMuTrig/hFibreSmbDose2"); 
  h1->GetXaxis()->SetTitle("");
  h1->GetYaxis()->SetTitle("Dose [Gy]");
  gStyle->SetOptStat(0);
  zone();
  shrinkPad(0.2, 0.15);
  h1->Draw();

  // -- remove most of the bin labels
  for (int ibin = 1; ibin <= h1->GetNbinsX(); ++ibin) {
    if (1 == ibin%4) {
      string sname = h1->GetXaxis()->GetBinLabel(ibin);
      h1->GetXaxis()->SetBinLabel(ibin, sname.c_str());
    }

    if (0 == ibin%4) h1->GetXaxis()->SetBinLabel(ibin, "");
    if (2 == ibin%4) h1->GetXaxis()->SetBinLabel(ibin, "");
    if (3 == ibin%4) h1->GetXaxis()->SetBinLabel(ibin, "");
  }

  h1->SetTitle("Fibre MuTRIG Dose");
  h1->Draw();

  c0.SaveAs("fibres-FibreSmbMuTrig-dose2-sumjobs.pdf");

  // -- scale up to 1e15 muon stops
  double sf = muonStopsPhase1/inTargetTotal;
  h1->Scale(sf);

  double n = muonStopsPhase1; 
  double expoN  = TMath::Log10(n);
  cout << "original expoN = " << expoN << endl;

  double mantN  = n / TMath::Power(10, static_cast<int>(expoN));
  cout << "original mantN = " << mantN << endl;

  h1->SetTitle(Form("Fibre MuTRIG Dose (n_{#mu}^{stop} = %1.1fE%2.0f)", mantN, expoN));
  h1->SetLineWidth(2);
  h1->SetAxisRange(0., 95., "X");
  h1->SetMinimum(0.);
  h1->GetXaxis()->SetNdivisions(-24, false);
  h1->Draw("hist");
  
  c0.SaveAs("fibres-FibreSmbMuTrig-dose2-phase1.pdf");

}

// ----------------------------------------------------------------------
void rphiMuTrig() {
  gStyle->SetOptStat(0);
  zone();
  shrinkPad(0.15, 0.15);
  TH2F *hu = (TH2F*)gFile->Get("stat/FibreSmbMuTrig/SmbMuTrigPlaneNegZ");
  TH2F *hd = (TH2F*)gFile->Get("stat/FibreSmbMuTrig/SmbMuTrigPlanePosZ");
  
  hu->SetAxisRange(-60., 60., "X");
  hu->SetAxisRange(-60., 60., "Y");
  hu->GetXaxis()->SetTitle("x [mm]");
  hu->GetXaxis()->SetTitleSize(0.05);
  hu->GetYaxis()->SetTitle("y [mm]");
  hu->GetYaxis()->SetTitleSize(0.05);
  hu->SetNdivisions(505, "Z");
  hu->SetMinimum(-0.01);

  hd->SetAxisRange(-60., 60., "X");
  hd->SetAxisRange(-60., 60., "Y");
  hd->GetXaxis()->SetTitle("x [mm]");
  hd->GetXaxis()->SetTitleSize(0.05);
  hd->GetYaxis()->SetTitle("y [mm]");
  hd->GetYaxis()->SetTitleSize(0.05);
  hd->SetMinimum(-0.01);
  hd->SetNdivisions(505, "Z");

  hu->Draw("axis");
  tl->SetNDC(kFALSE);
  tl->SetTextSize(0.03);
  for (int ix = 1; ix <= hu->GetNbinsX(); ++ix) {
    for (int iy = 1; iy <= hu->GetNbinsY(); ++iy) {
      if (hu->GetBinContent(ix, iy) > -1)
        tl->DrawLatex(hu->GetXaxis()->GetBinCenter(ix),
                      hu->GetYaxis()->GetBinCenter(iy),
                      Form("%d", static_cast<int>(hu->GetBinContent(ix, iy))));
    }
  }
  
  c0.SaveAs("fibres-us-rphi-MuTrig.pdf");
  
  hd->Draw("axis");
  tl->SetNDC(kFALSE);
  tl->SetTextSize(0.03);
  for (int ix = 1; ix <= hd->GetNbinsX(); ++ix) {
    for (int iy = 1; iy <= hd->GetNbinsY(); ++iy) {
      if (hd->GetBinContent(ix, iy) > -1)
        tl->DrawLatex(hd->GetXaxis()->GetBinCenter(ix),
                      hd->GetYaxis()->GetBinCenter(iy),
                      Form("%d", static_cast<int>(hd->GetBinContent(ix, iy))));
    }
  }
  c0.SaveAs("fibres-ds-rphi-MuTrig.pdf");
}


// ----------------------------------------------------------------------
void rphiSmb() {
  gStyle->SetOptStat(0);
  zone();
  shrinkPad(0.15, 0.15);
  TH2F *hu = (TH2F*)gFile->Get("stat/FibreSmb/PlaneNegZ");
  TH2F *hd = (TH2F*)gFile->Get("stat/FibreSmb/PlanePosZ");
  
  hu->SetAxisRange(-60., 60., "X");
  hu->SetAxisRange(-60., 60., "Y");
  hu->GetXaxis()->SetTitle("x [mm]");
  hu->GetXaxis()->SetTitleSize(0.05);
  hu->GetYaxis()->SetTitle("y [mm]");
  hu->GetYaxis()->SetTitleSize(0.05);
  hu->SetMinimum(-0.01);

  hd->SetAxisRange(-60., 60., "X");
  hd->SetAxisRange(-60., 60., "Y");
  hd->GetXaxis()->SetTitle("x [mm]");
  hd->GetXaxis()->SetTitleSize(0.05);
  hd->GetYaxis()->SetTitle("y [mm]");
  hd->GetYaxis()->SetTitleSize(0.05);
  hd->SetMinimum(-0.01);

  hu->Draw("axis");
  tl->SetNDC(kFALSE);
  tl->SetTextSize(0.05);
  for (int ix = 1; ix <= hu->GetNbinsX(); ++ix) {
    for (int iy = 1; iy <= hu->GetNbinsY(); ++iy) {
      if (hu->GetBinContent(ix, iy) > -1)
        tl->DrawLatex(hu->GetXaxis()->GetBinCenter(ix),
                      hu->GetYaxis()->GetBinCenter(iy),
                      Form("%d", static_cast<int>(hu->GetBinContent(ix, iy))));
    }
  }
  c0.SaveAs("fibres-us-rphi-SMB.pdf");

  hd->Draw("axis");
  tl->SetNDC(kFALSE);
  tl->SetTextSize(0.05);
  for (int ix = 1; ix <= hd->GetNbinsX(); ++ix) {
    for (int iy = 1; iy <= hd->GetNbinsY(); ++iy) {
      if (hd->GetBinContent(ix, iy) > -1)
        tl->DrawLatex(hd->GetXaxis()->GetBinCenter(ix),
                      hd->GetYaxis()->GetBinCenter(iy),
                      Form("%d", static_cast<int>(hd->GetBinContent(ix, iy))));
    }
  }
  c0.SaveAs("fibres-ds-rphi-SMB.pdf");
}


// ----------------------------------------------------------------------
void allGeometryPlots() {
  rphiSmb();
  rphiMuTrig();  
}
