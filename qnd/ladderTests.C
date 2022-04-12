#include <stdio.h>
#include <stdlib.h>

#include "TGraphErrors.h"

#include "../../analyzer/analyzer/utility/json.h"

using namespace std;

// -- <HLDR, quantity<chip 0, chip 1, chip2> > 
map<int, vector<double> > gLayout; 


// ----------------------------------------------------------------------
string readFromJson(string filename, vector<string> what) {
  ifstream INS(filename);
  string sline;
  vector<bool> found;
  int cnt(0); 
  for (unsigned int i = 0; i < what.size(); ++i) {
    found.push_back(false);
  }
  while (getline(INS, sline)) {
    //    cout << sline << endl;
    string search = string("\"") + what[cnt] + string("\"");
    if (string::npos != sline.find(search)) {
      ++cnt;
    }
    if (cnt == what.size()) {
      cout << "==return> " << sline << endl;
      break;
    }
  }
  return sline; 
}

// ----------------------------------------------------------------------
vector<pair<double, double> > combine2Lines(string l1, string l2) {
  vector<pair<double, double> > result;

  l1 = l1.substr(l1.find("[") + 2);
  l1 = l1.substr(0, l1.rfind("]") -1);
  l2 = l2.substr(l2.find("[") + 2);
  l2 = l2.substr(0, l2.rfind("]") -1);

  cout << "l1 ->" << l1 << "<-" << endl;
  cout << "l2 ->" << l2 << "<-" << endl;
  
  // -- start reading until
  double val1, val2;
  string comma;
  istringstream sl1(l1);
  istringstream sl2(l2);
  sl1 >> val1 >> comma;
  sl2 >> val2 >> comma;
  cout << "val1(" << val1 << "), val2(" << val2 << ")" << endl;
  result.push_back(make_pair(val1, val2));
  
  while (1) {
    sl1 >> val1 >> comma;
    sl2 >> val2 >> comma;
    if (val1 < 1) break;
    result.push_back(make_pair(val1, val2));
    cout << result.size() << ": val1(" << val1 << "), val2(" << val2 << ")" << endl;
  }
  
  
  return result;
  
}

// ----------------------------------------------------------------------
TGraphErrors* makeGraph(vector<pair<double, double> > result, int mcolor, int mstyle, double msize) {
  int nsize(result.size());

  // -- sort vectors
  vector<double> x, y;
  x.push_back(result[0].first);
  y.push_back(result[0].second); 
  for (unsigned int i = 1; i < result.size(); ++i) {
    for (unsigned int ix = 0; ix < x.size(); ++ix) {
      if ((result[i].first < x[ix]) && (ix < x.size())) {
        auto it = x.insert(x.begin() + ix, result[i].first);
        auto iu = y.insert(y.begin() + ix, result[i].second);
        break;  
      }
      if (ix + 1 == x.size()) {
        x.push_back(result[i].first);
        y.push_back(result[i].second);
        break;  
      }
    }
  }

  double vx[result.size()];
  double vy[result.size()];
  double vxe[result.size()];
  double vye[result.size()];
  for (unsigned int i = 0; i < nsize; ++i) {
    vx[i] = x[i];
    vy[i] = y[i];

    vxe[i] = 0.;
    vye[i] = 0.;
  }
  
  TGraphErrors *gr = new TGraphErrors(nsize, vx, vy, vxe, vye); 
  gr->SetMarkerStyle(mstyle);
  gr->SetMarkerSize(msize);
  gr->SetMarkerColor(mcolor);
  gr->SetLineColor(mcolor);
  gr->SetLineWidth(3);
  return gr; 
}


// ----------------------------------------------------------------------
TGraphErrors* ivTest(string filename= "qc_ladder_0.json", int color = 1) {
  cout << "reading json from file ->" << filename << "<-" << endl;
  vector<string> ivstrings;
  ivstrings.push_back("IV");
  ivstrings.push_back("Output");
  ivstrings.push_back("Voltage");
  string voltage = readFromJson(filename, ivstrings); 
  ivstrings.clear();
  ivstrings.push_back("IV");
  ivstrings.push_back("Output");
  ivstrings.push_back("Current");
  string current = readFromJson(filename, ivstrings); 

  vector<pair<double, double> > result = combine2Lines(voltage, current); 
  
  TGraphErrors *grG  = makeGraph(result, color, 20, 1.);
  grG->Draw("l");
  return grG; 
}


// ----------------------------------------------------------------------
void ivTests(int layer = 1) {
  vector<string> files;
  vector<int> cols;
  
  TH1D *h1 = new TH1D("h1", "h1", 30, 0., 30.);
  h1->SetMaximum(0.5);
  h1->SetMinimum(-12.0);
  h1->GetXaxis()->SetTitle("Bias voltage [-V]");
  h1->GetYaxis()->SetTitle("Current [#muA]");
  h1->GetYaxis()->SetTitleOffset(1.1);
  
  h1->Draw();

  
  if (0 == layer) {
    files.push_back("qc_ladder_0.json"); cols.push_back(kRed+1);      
    files.push_back("qc_ladder_1.json"); cols.push_back(kRed+2);      
    files.push_back("qc_ladder_2.json"); cols.push_back(kBlue+1);     
    files.push_back("qc_ladder_3.json"); cols.push_back(kBlue+3);     
    files.push_back("qc_ladder_4.json"); cols.push_back(kGreen+1);     
    files.push_back("qc_ladder_6.json"); cols.push_back(kGreen+3);     
    files.push_back("qc_ladder_7.json"); cols.push_back(kYellow+1);     
  };                                     
  
  if (1 == layer) {
    files.push_back("qc_ladder_10.json"); cols.push_back(kRed+1);    
    files.push_back("qc_ladder_11.json"); cols.push_back(kRed+2);     
    files.push_back("qc_ladder_12.json"); cols.push_back(kBlue+1);      
    files.push_back("qc_ladder_13.json"); cols.push_back(kBlue+3);     
    files.push_back("qc_ladder_14.json"); cols.push_back(kGreen+1);  
    files.push_back("qc_ladder_15.json"); cols.push_back(kGreen+3);  
    files.push_back("qc_ladder_16.json"); cols.push_back(kYellow+1);   
    files.push_back("qc_ladder_17.json"); cols.push_back(kYellow+3);   
  };
  vector<string> opts;
  vector<string> titles;
  vector<int> ladderNumbers;
  for (unsigned int i = 0; i < files.size(); ++i) {
    opts.push_back("p");
    string title = files[i];
    replaceAll(title, ".json", "");
    replaceAll(title, "qc_", "");
    replaceAll(title, "_", " ");
    titles.push_back(title);
    
    replaceAll(title, "ladder", "");
    replaceAll(title, " ", "");
    ladderNumbers.push_back(atoi(title.c_str()));
    cout << "ladderNumber " << ladderNumbers[i] << endl;
  }
  
  vector<TGraphErrors*> hists; 
  for (unsigned int i = 0; i < files.size(); ++i) {
    TGraphErrors *gr = ivTest(files[i], cols[i]);
    hists.push_back(gr);
    // -- determine V at which compliance (<-9.9 uA) is reached
    bool foundCompliance(false);
    for (int ix = 0; ix < gr->GetN(); ++ix) {
      if (gr->GetPointY(ix) < -9.9) {
        cout << "hldr = " << ladderNumbers[i]
             << " compliance " << gr->GetPointY(ix) << " at " << gr->GetPointX(ix) << endl;
        vector<double> vchips = {gr->GetPointX(ix), gr->GetPointX(ix), gr->GetPointX(ix)};
        gLayout.insert(make_pair(ladderNumbers[i], vchips));
        foundCompliance = true; 
        break;
      }
    }
    if (!foundCompliance) {
      vector<double> vchips = {30., 30., 30.};
      gLayout.insert(make_pair(ladderNumbers[i], vchips));
      cout << "hldr = " << ladderNumbers[i]
           << " no compliance until 25" << endl;
    }
  }

  gStyle->SetOptStat(0); 
  gStyle->SetOptTitle(0); 
  
  for (unsigned int i = 0; i < hists.size(); ++i) {
    hists[i]->Draw("pl");
  }

  TLegend *tll = newLegend(0.7, 0.2, 0.88, 0.5);
  tll->SetTextSize(0.03);
  if (0 == layer) {
    tll->SetHeader("Layer 0");
  } else if (1 == layer) {
    tll->SetHeader("Layer 1");
  }    
  for (unsigned int i = 0; i < files.size(); ++i) {
    TLegendEntry *tle = tll->AddEntry(hists[i], titles[i].c_str(), "p");
    tle->SetTextColor(cols[i]);
  }
  
  tll->Draw();

  tl->SetTextColor(kGray);
  tl->SetTextSize(0.001);
  tl->DrawLatexNDC(0.102, 0.102, "UL");
  
  if (1 == layer) {
    c0.SaveAs("iv-layer1.pdf");
  }
  if (0 == layer) {
    c0.SaveAs("iv-layer0.pdf");
  }
  
}



// ----------------------------------------------------------------------
void mapIV() {
  TH2D *hl0 = new TH2D("L0", "L0", 6, 0., 6.,  8, 0., 8.);
  hl0->SetMaximum(30.);
  hl0->SetNdivisions(600, "X");
  hl0->SetNdivisions(800, "Y");
  hl0->GetZaxis()->SetTitle("V_{B}");
  for (int i = 0; i < 8; ++i) {
    hl0->SetBinLabel(i, Form("%d", i));
  }
  TH2D *hl1 = new TH2D("L1", "L1", 6, 0., 6., 10, 0., 10.);
  hl1->SetNdivisions(600, "X");
  hl1->SetNdivisions(1000, "Y");
  hl1->SetMaximum(30.);

  // -- fixed coloring
  Int_t    colors[] = {kRed, kRed-9, kBlue-6, kBlue-4, kGreen+1, kGreen+2};
  Double_t levels[] = {0.,   5.,     10.,     15.,     20.,      25.,       30.};
  gStyle->SetPalette((sizeof(colors)/sizeof(Int_t)), colors);
  
  hl0->SetContour((sizeof(levels)/sizeof(Double_t)), levels);
  hl1->SetContour((sizeof(levels)/sizeof(Double_t)), levels);
  
  if (gLayout.size() < 2) {
    ivTests(0); 
    ivTests(1); 
  }

  map<int, vector<double> >::iterator it;
  for (it = gLayout.begin(); it != gLayout.end(); ++it) {
    int hldr = it->first; 
    int cmpl = it->second[0]; 
    cout << "HLDR " << hldr << " cmpl = " << cmpl << endl;
    // -- FIXME!!!
    if (hldr < 8) {
      hl0->SetBinContent(1, hldr+1, cmpl);
      hl0->SetBinContent(2, hldr+1, cmpl);
      hl0->SetBinContent(3, hldr+1, cmpl);
    } else {
      int bias(9);
      hl1->SetBinContent(1, hldr+1-bias, cmpl);
      hl1->SetBinContent(2, hldr+1-bias, cmpl);
      hl1->SetBinContent(3, hldr+1-bias, cmpl);
    }
  }
  
  c0.SetWindowSize(800, 400); 
  zone(2,1);
  gPad->SetRightMargin(0.2);
  gPad->SetGridx(1);
  gPad->SetGridy(1);
  hl0->Draw("colz");

  gPad->Update();
  TPaletteAxis *palette = (TPaletteAxis*)hl0->GetListOfFunctions()->FindObject("palette");
  palette->SetX1NDC(0.88);
  palette->SetX2NDC(0.92);
  gPad->Modified();
  gPad->Update();

  c0.cd(2);
  gPad->SetGridx(1);
  gPad->SetGridy(1);
  hl1->Draw("colz");
}
