#include <fstream>
#include <string>

// ----------------------------------------------------------------------
void chipIDSpecBook(int chipid, int &station, int &layer, int &phi, int &z) {
  station = chipid/(0x1<<12);
  layer   = chipid/(0x1<<10) % 4 + 1;
  int p   = chipid/(0x1<<5) % (0x1<<5) + 1;

  int zp(-99);

  if ((layer == 1) && (p>3)) {
    cout << "(layer == 1) && (p>3)), p = " << p << endl;
    phi = p - 3;
  } else if ((layer == 2) && (p>3)) {
    phi = p;
  } else {
    phi = p + 1;
  }
  zp  = chipid % (0x1<<5);

  if (layer == 3) {
    z = zp - 7;
  } else if (layer == 4) {
    z = zp - 6;
  } else {
    z = zp;
  }

}


// ----------------------------------------------------------------------
void chipIDFromSimulation(string filename) {

  vector<string> allChips;
  // fill it

  string sline;
  ifstream INS(filename);
  while (getline(INS, sline)) {
    allChips.push_back(sline);
  }
  INS.close();

  TH2D *hLayer1a= new TH2D("hLayer1a", "Layer 1 (alignment/sensors)", 6, -42., 63., 8, -180., 180.);
  TH2D *hLayer1i= new TH2D("hLayer1i", "", 6, -42., 63., 8, -180., 180.);
  setTitles(hLayer1a, "z [mm]", "#phi", 0.05, 1.0, 1.0);

  TH2D *hLayer2a= new TH2D("hLayer2a", "Layer 2 (alignment/sensors)", 6, -42., 63., 10, -180., 180.);
  TH2D *hLayer2i= new TH2D("hLayer2i", "", 6, -42., 63., 10, -180., 180.);
  setTitles(hLayer2a, "z [mm]", "#phi", 0.05, 1.0, 1.0);

  TH2D *hLayer31= new TH2D("hLayer31", "Layer 3/Station 1 (alignment/sensors)", 200, -600.,-200., 192, -185., 180.);
  setTitles(hLayer31, "z [mm]", "#phi", 0.05, 1.0, 0.6);
  TH2D *hLayer32= new TH2D("hLayer32", "Layer 3/Station 2 (alignment/sensors)", 200, -200., 200., 192, -185., 180.);
  setTitles(hLayer32, "z [mm]", "#phi", 0.05, 1.0, 0.6);
  TH2D *hLayer33= new TH2D("hLayer33", "Layer 3/Station 3 (alignment/sensors)", 200,  200., 600., 192, -185., 180.);
  setTitles(hLayer33, "z [mm]", "#phi", 0.05, 1.0, 0.6);
  TH2D *hLayer3i= new TH2D("hLayer3i", "", 200,  200., 600., 24, -180., 180.);

  TH2D *hLayer41= new TH2D("hLayer41", "Layer 4/Station 1 (alignment/sensors)", 200, -600.,-200., 224, -185., 180.);
  setTitles(hLayer41, "z [mm]", "#phi", 0.05, 1.0, 0.6);
  TH2D *hLayer42= new TH2D("hLayer42", "Layer 4/Station 2 (alignment/sensors)", 200, -200., 200., 224, -185., 180.);
  setTitles(hLayer42, "z [mm]", "#phi", 0.05, 1.0, 0.6);
  TH2D *hLayer43= new TH2D("hLayer43", "Layer 4/Station 3 (alignment/sensors)", 200,  200., 600., 224, -185., 180.);
  setTitles(hLayer43, "z [mm]", "#phi", 0.05, 1.0, 0.6);
  TH2D *hLayer4i= new TH2D("hLayer4i", "", 200,  200., 600., 28, -180., 180.);

  for (auto it: allChips) {
    vector<string> numbers = split(it, ',');
    int chipID = atoi(numbers[0].c_str());
    if (chipID > 15000) {
      cout << it << endl;
    }
    double x   = atof(numbers[1].c_str());
    double y   = atof(numbers[2].c_str());
    double z   = atof(numbers[3].c_str());
    double r   = TMath::Sqrt(x*x + y*y);
    double f   = TMath::RadToDeg()*TMath::ATan2(y,x);

    int layer(0);

    int iphi, iz, il, is;
    chipIDSpecBook(chipID, is, il, iphi, iz);

    if (r < 26.) {
      layer = 1;
      hLayer1a->Fill(z, f, chipID);
      hLayer1i->Fill(z, f, iphi);
    } else if (r < 32.) {
      layer = 2;
      hLayer2a->Fill(z, f, chipID);
      hLayer2i->Fill(z, f, iphi);
    } else if (r < 73.) {
      layer = 3;
      if (z < -200) {
        hLayer31->Fill(z, f, chipID);
      } else if (z > 200) {
        hLayer33->Fill(z, f, chipID);
      } else {
        hLayer32->Fill(z, f, chipID);
      }
      hLayer3i->Fill(z, f, iphi);
    } else {
      layer = 4;
      if (z < -200) {
        hLayer41->Fill(z, f, chipID);
      } else if (z > 200) {
        hLayer43->Fill(z, f, chipID);
      } else {
        hLayer42->Fill(z, f, chipID);
      }
      hLayer4i->Fill(z, f, iphi);
    }
  }

  c0.SetCanvasSize(600, 600);
  shrinkPad(0.11, 0.1);
  gStyle->SetOptStat(0);
  hLayer1i->Draw("colz");
  hLayer1a->Draw("textsame");
  c0.SaveAs("hLayer1.pdf");

  hLayer2i->Draw("colz");
  hLayer2a->Draw("textsame");
  c0.SaveAs("hLayer2.pdf");

  c0.SetCanvasSize(1200, 600);
  shrinkPad(0.11, 0.07);
  hLayer3i->Draw("colz");
  hLayer31->Draw("textsame");
  c0.SaveAs("hLayer31.pdf");
  hLayer3i->Draw("colz");
  hLayer32->Draw("textsame");
  c0.SaveAs("hLayer32.pdf");
  hLayer3i->Draw("colz");
  hLayer33->Draw("textsame");
  c0.SaveAs("hLayer33.pdf");

  hLayer4i->Draw("colz");
  hLayer41->Draw("textsame");
  c0.SaveAs("hLayer41.pdf");
  hLayer4i->Draw("colz");
  hLayer42->Draw("textsame");
  c0.SaveAs("hLayer42.pdf");
  hLayer4i->Draw("colz");
  hLayer43->Draw("textsame");
  c0.SaveAs("hLayer43.pdf");
}
