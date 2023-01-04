#include "TFile.h"

int NFILE(21);

// ----------------------------------------------------------------------
void plotFeeAsicID() { 
  gFile ->cd("stat");
  
  TH1F* PlanePosZ = (TH1F*)gDirectory->Get("PlanePosZ");
  PlanePosZ->Scale(1./NFILE);
  PlanePosZ->GetXaxis()->SetTitle("x [mm]");
  PlanePosZ->GetYaxis()->SetTitle("y [mm]");
  PlanePosZ->GetYaxis()->SetTitleOffset(1.3);

  gStyle->SetOptStat(0);
  
  PlanePosZ->Draw("text");

  c0.SaveAs("PlanePosZ.pdf");
  
  TH1F* PlaneNegZ = (TH1F*)gDirectory->Get("PlaneNegZ");
  PlaneNegZ->Scale(1./NFILE);
  PlaneNegZ->GetXaxis()->SetTitle("x [mm]");
  PlaneNegZ->GetYaxis()->SetTitle("y [mm]");
  PlaneNegZ->GetYaxis()->SetTitleOffset(1.3);

  gStyle->SetOptStat(0);
  
  PlaneNegZ->Draw("text");
  c0.SaveAs("PlaneNegZ.pdf");

  
}


// ----------------------------------------------------------------------
void plotFeeDose() { 
  gFile ->cd("stat");
  
  TH1F* h1 = (TH1F*)gDirectory->Get("hFibreFEEDose");
  h1->Scale(1./NFILE);
  h1->SetTitle("Fibre FEE Dose (normalization?)");

  gStyle->SetOptStat(0);
  
  h1->Draw("hist");
  c0.SaveAs("fibreFeeDose.pdf");

}


// ----------------------------------------------------------------------
void plotFeeImpact() { 
  gFile ->cd("stat");
  
  TH2F* h2 = (TH2F*)gDirectory->Get("RadialOutElmz2");

  gPad->SetRightMargin(0.15);
  
  gStyle->SetStatX(0.5);
  gStyle->SetStatY(0.5);
  
  h2->Draw("colz");
  c0.SaveAs("fibreFeeImpact-global.pdf");


  h2 = (TH2F*)gDirectory->Get("RadialOutElmz1");


  h2->Draw("colz");
  c0.SaveAs("fibreFeeImpact-local.pdf");

}
