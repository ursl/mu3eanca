void plot1() {
  TTree *t = (TTree*)gFile->Get("events");

  TH1D *h1 = new TH1D("h1", "", 32, 0., 256.);
  setTitles(h1, "tot", "entries/bin", 0.05, 1.1, 1.6);
  h1->SetMinimum(0.);

  TH1D *hhits = new TH1D("hhits", "", 100, 0., 100.);
  setTitles(hhits, "n(Hit)", "entries/bin", 0.05, 1.1, 1.6);
  hhits->SetMinimum(0.5);

  TH1D *hchip = new TH1D("hchip", "", 120, 0., 120.);
  setTitles(hchip, "chipID", "entries/bin", 0.05, 1.1, 1.6);
  hchip->SetMinimum(0.);

  TH2D *h2 = new TH2D("h2", "", 256, 0., 256., 250, 0., 250.);
  setTitles(h2, "col", "row", 0.05, 1.1, 1.6);

  TH2D *h2tot = new TH2D("h2tot", "", 120, 0., 120., 32, 0., 256.);
  setTitles(h2tot, "chipID", "tot", 0.05, 1.1, 1.6);

  shrinkPad(0.15, 0.15, 0.15);
  gStyle->SetOptStat(0);

  h1->SetTitle("all chips");
  t->Draw("tot>>h1", "", "");
  c0.SaveAs("goodhits_tot.pdf");

  gPad->SetLogy(1);
  hhits->SetTitle("hits/chip, all chips");
  t->Draw("nchip>>hhits", "", "");
  c0.SaveAs("goodhits_nchip.pdf");
  
  gPad->SetLogy(1);
  hhits->SetTitle("hits/event, all chips");
  t->Draw("nevt>>hhits", "", "");
  c0.SaveAs("goodhits_nevt.pdf");

  gPad->SetLogy(0);
  h1->SetTitle("good hits");
  t->Draw("chipID>>hchip", "", "");
  c0.SaveAs("goodhits_chipID.pdf");

  h2->SetTitle("good hits, all runs");
  t->Draw("row:col>>h2", "", "colz");
  c0.SaveAs("goodhits_hitmap.pdf");

  
  h1->SetTitle("all chips (20 pixels away from edge)");
  t->Draw("tot>>h1", "col>20 && col < 230 && row >20 && row<230", "");
  c0.SaveAs("tot_inner20.pdf");

  h1->SetTitle("all chips (50 pixels away from edge)");
  t->Draw("tot>>h1", "col>50 && col < 200 && row >50 && row<200", "");
  c0.SaveAs("tot_inner50.pdf");

  h1->SetTitle("all chips (top part)");
  t->Draw("tot>>h1", "col>20 && col < 230 && row >125 && row<230", "");
  c0.SaveAs("tot_top.pdf");

  h1->SetTitle("all chips (bottom part)");
  t->Draw("tot>>h1", "col>20 && col < 230 && row >20 && row<230", "");
  c0.SaveAs("tot_bottom.pdf");

  h1->SetTitle("all chips (left part)");
  t->Draw("tot>>h1", "col>20 && col < 125 && row >20 && row<230", "");
  c0.SaveAs("tot_left.pdf");

  h1->SetTitle("all chips (right part)");
  t->Draw("tot>>h1", "col>125 && col < 230 && row >20 && row<230", "");
  c0.SaveAs("tot_right.pdf");
  
  h1->SetTitle("edge pixels (5 pixels to edge, not chips 38,43,44,45,79,80,108,109,110)");
  t->Draw("tot>>h1", "(col<5 || col > 250 || row <5 || row>245) && !(chipID==38||chipID==43||chipID==44||chipID==45||chipID==79||chipID==80||chipID==108||chipID==109||chipID==110)", "");
  c0.SaveAs("tot_edge5.pdf");

  h1->SetTitle("chips 38,43,44,45,108,109,110");
  t->Draw("tot>>h1", "(chipID==38||chipID==43||chipID==44||chipID==45||chipID==108||chipID==109||chipID==110)", "");
  c0.SaveAs("tot_specialChips.pdf");

  h2->SetTitle("chips 38,43,44,45,108,109,110");
  t->Draw("row:col>>h2", "(chipID==38||chipID==43||chipID==44||chipID==45||chipID==108||chipID==109||chipID==110)", "colz");
  c0.SaveAs("hitmap_specialChips.pdf");

  
  h2tot->SetTitle("all chips");
  t->Draw("tot:chipID>>h2tot", "", "colz");
  c0.SaveAs("tot_chipID.pdf");


  h2->SetTitle("tot<32");
  t->Draw("row:col>>h2", "tot<32", "colz");
  c0.SaveAs("tot_smaller32.pdf");

  h2->SetTitle("tot>32");
  t->Draw("row:col>>h2", "tot>32", "colz");
  c0.SaveAs("tot_larger32.pdf");

  h2->SetTitle("tot<32 (chipID == 43,44,45)");
  t->Draw("row:col>>h2", "tot<32 && (chipID==43 || chipID==44 || chipID==45)", "colz");
  c0.SaveAs("tot_smaller32_chips43-45.pdf");

  for (int i = 0; i < 120; ++i) {
    h1->Reset();
    h1->SetTitle(Form("chipID == %d", i));
    t->Draw("tot>>h1", Form("chipID == %d", i), "");
    if (0 == h1->GetEntries()) continue;
    c0.SaveAs(Form("tot-chipID%d.pdf", i));


    h2->SetTitle(Form("chipID == %d", i));
    t->Draw("row:col>>h2", Form("chipID==%d && (tot>80 && tot<200)", i), "colz");
    c0.SaveAs(Form("hitmap_chipID%d.pdf", i));

  }
  
  
  h1->SetTitle("!(chipID==79 || chipID==80)");
  t->Draw("tot>>h1", "!(chipID==79||chipID==80)", "");
  c0.SaveAs("tot_notChips79-80.pdf");

  h1->SetTitle("chipID==79 || chipID==80");
  t->Draw("tot>>h1", "(chipID==79||chipID==80)", "");
  c0.SaveAs("tot_Chip79-80.pdf");

  h2->SetTitle("chipID==79 || chipID==80");
  t->Draw("row:col>>h2", "(chipID==79 || chipID==80)", "colz");
  c0.SaveAs("hitmap_chips79-80.pdf");

  h2->SetTitle("!(chipID==79 || chipID==80)");
  t->Draw("row:col>>h2", "!(chipID==79 || chipID==80)", "colz");
  c0.SaveAs("hitmap_notChips79-80.pdf");

}
