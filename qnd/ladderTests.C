#include <stdio.h>
#include <stdlib.h>
#include <algorithm>

#include "TGraphErrors.h"

using namespace std;

// -- <HLDR, quantity<chip 0, chip 1, chip2> > 
map<int, vector<double> > gLayout; 


void drawChipNumbering() {

  TH2D *h1 = new TH2D("h2", "", 6, 0., 6., 1, 0., 1.);
  h1->SetNdivisions(0, "X");
  h1->SetNdivisions(0, "Y");

  c0.SetWindowSize(800, 100); 


  gStyle->SetOptStat(0);

  
  h1->Draw("");


  pl->SetLineWidth(3);
  for (int i = 0; i <= 6; ++i) {
    pl->DrawLine(i, 0., i, 1.);
  }
  pl->DrawLine(0., 0., 6., 0.);
  pl->DrawLine(0., 1., 6., 1.);
  

  tl->SetTextSize(0.6);
  tl->SetTextColor(kBlack);
  tl->SetNDC(kFALSE);

  double ypos(0.3);
  tl->DrawLatex(0.4, ypos, "2");
  tl->DrawLatex(1.4, ypos, "1");
  tl->DrawLatex(2.4, ypos, "0");

  tl->DrawLatex(3.4, ypos, "2");
  tl->DrawLatex(4.4, ypos, "1");
  tl->DrawLatex(5.4, ypos, "0");

  tl->SetNDC(kTRUE);

  c0.SaveAs("laddertests-chipnumbering.pdf");
  
}

// ----------------------------------------------------------------------
void drawChipGrid(int imax) {
  for (int i = 0; i <= imax; ++i) {
    pl->DrawLine(0., i, 6., i);
  }

  for (int i = 0; i <= 6; ++i) {
    pl->DrawLine(i, 0., i, imax);
  }

}

// ----------------------------------------------------------------------
void drawLadderGrid(int imax) {
  for (int i = 0; i <= imax; ++i) {
    pl->DrawLine(0., i, 6., i);
  }

  pl->DrawLine(0, 0., 0, imax);
  pl->DrawLine(3, 0., 3, imax);
  pl->DrawLine(6, 0., 6, imax);

}


// ----------------------------------------------------------------------
// -- return <BIN, US=0|DS=1> pair, given HLDR number
pair<int, int> hldrBin(int hldr) {
  // bin numbering starts at 1!
  if (hldr >= 0 && hldr <= 7) {
    return make_pair(hldr+1, 0); 
  } else if (hldr >= 18 && hldr <= 25) {
    return make_pair(hldr-17, 1);
  } else if (hldr >= 8 && hldr <= 17) {
    return make_pair(hldr-7, 0);
  } else if (hldr >= 26 && hldr <= 35) {
    return make_pair(hldr-25, 1);
  }
  return make_pair(-1, -1);
}


// ----------------------------------------------------------------------
// -- return layer number based on hldr
int hldrLayer(int hldr) {
  if (hldr >= 0 && hldr <= 7) {
    return 0;
  } else if (hldr >= 18 && hldr <= 25) {
    return 0;
  }

  return 1;
}


// ----------------------------------------------------------------------
void loadFiles(vector<string> &files, vector<int> &cols,
               vector<string> &titles, vector<int> &ladderNumbers,
               int layer) {
  if (0 == layer) {
    files.push_back("qc_ladder_0.json"); cols.push_back(kRed+1);      
    files.push_back("qc_ladder_1.json"); cols.push_back(kRed+2);      
    files.push_back("qc_ladder_2.json"); cols.push_back(kRed-4);     
    files.push_back("qc_ladder_3.json"); cols.push_back(kMagenta+1);     
    files.push_back("qc_ladder_4.json"); cols.push_back(kMagenta-6);     
    files.push_back("qc_ladder_6.json"); cols.push_back(kYellow+4);     
    files.push_back("qc_ladder_7.json"); cols.push_back(kYellow-2);     
    
    files.push_back("qc_ladder_18.json"); cols.push_back(kBlue+1);      
    files.push_back("qc_ladder_19.json"); cols.push_back(kBlue-7);      
    files.push_back("qc_ladder_20.json"); cols.push_back(kBlue-10);     
    files.push_back("qc_ladder_21.json"); cols.push_back(kCyan+3);     
    files.push_back("qc_ladder_22.json"); cols.push_back(kCyan+1);     
    files.push_back("qc_ladder_23.json"); cols.push_back(kGreen+1);     
    files.push_back("qc_ladder_24.json"); cols.push_back(kGreen+3);     
    files.push_back("qc_ladder_25.json"); cols.push_back(kGreen-6);     
  };                                     
  
  if (1 == layer) {
    files.push_back("qc_ladder_10.json"); cols.push_back(kRed+1);    
    files.push_back("qc_ladder_11.json"); cols.push_back(kRed+2);     
    files.push_back("qc_ladder_12.json"); cols.push_back(kRed-4);      
    files.push_back("qc_ladder_13.json"); cols.push_back(kMagenta+1);     
    files.push_back("qc_ladder_14.json"); cols.push_back(kMagenta-6);  
    files.push_back("qc_ladder_15.json"); cols.push_back(kYellow+4);  
    files.push_back("qc_ladder_16.json"); cols.push_back(kYellow-2);   
    files.push_back("qc_ladder_17.json"); cols.push_back(kYellow-8);   

    files.push_back("qc_ladder_26.json"); cols.push_back(kBlue+1);    
    files.push_back("qc_ladder_27.json"); cols.push_back(kBlue-7);     
    files.push_back("qc_ladder_28.json"); cols.push_back(kBlue-10);      
    files.push_back("qc_ladder_29.json"); cols.push_back(kCyan+3);     
    files.push_back("qc_ladder_30.json"); cols.push_back(kCyan+1);  
    files.push_back("qc_ladder_31.json"); cols.push_back(kCyan-8);  
    files.push_back("qc_ladder_32.json"); cols.push_back(kGreen+1);   
    files.push_back("qc_ladder_33.json"); cols.push_back(kGreen+3);   
    files.push_back("qc_ladder_34.json"); cols.push_back(kGreen-6);   
    files.push_back("qc_ladder_35.json"); cols.push_back(kGreen-8);   
  };

  for (unsigned int i = 0; i < files.size(); ++i) {
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

  
}

// ----------------------------------------------------------------------
string readFromJson(string filename, vector<string> what) {
  cout << "***** " << filename << endl;
  ifstream INS(filename);
  string sline;
  vector<bool> found;
  int cnt(0); 
  for (unsigned int i = 0; i < what.size(); ++i) {
    found.push_back(false);
  }
  while (getline(INS, sline)) {
    string search = string("\"") + what[cnt] + string("\"");
    if (string::npos != sline.find(search)) {
      // cout << "found ->" << search << "<- in " << sline << endl;
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
  
  size_t nComma = std::count(l1.begin(), l1.end(), ',');
  int iComma(0);
  
  while (1) {
    sl1 >> val1 >> comma;
    sl2 >> val2 >> comma;
    if (iComma > nComma+1) break;
    result.push_back(make_pair(val1, val2));
    cout << result.size() << ": val1(" << val1 << "), val2(" << val2 << ")" << endl;
    ++iComma;
  }
    
  return result;
  
}

// ----------------------------------------------------------------------
vector<double> split1Line(string l1) {
  vector<double> result;

  l1 = l1.substr(l1.find("[") + 2);
  l1 = l1.substr(0, l1.rfind("]") -1);

  cout << "l1 ->" << l1 << "<-" << endl;
  
  // -- start reading until
  double val1;
  string comma;
  istringstream sl1(l1);
  sl1 >> val1 >> comma;
  cout << "val1(" << val1 << ")" << endl;
  result.push_back(val1);
  
  for (int i = 1; i < 3; ++i) {
    sl1 >> val1 >> comma;
    result.push_back(val1);
    cout << result.size() << ": val1(" << val1 << ")" << endl;
  }

  if (result.size() > 3) {
    result.erase(result.end() - 1);     
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
    cout << "i = " << i << " x = " << vx[i] << " y = " << vy[i] << endl;
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
vector<TGraphErrors *> linkqualiTest(string filename= "qc_ladder_0.json", int color = 1) {
  cout << "reading json from file ->" << filename << "<-" << endl;
  vector<string> vcostrings;
  
  vector<vector<pair<double, double> > > allResults;
  
  for (int ic = 0; ic < 3; ++ic) {
    vcostrings.clear();
    vcostrings.push_back("LINKQUALIcheck");
    vcostrings.push_back("Output");
    vcostrings.push_back(Form("%d", ic));
    vcostrings.push_back("Scan");
    vcostrings.push_back("VPVCO");
    string voltage = readFromJson(filename, vcostrings); 
    vcostrings.clear();
    vcostrings.push_back("LINKQUALIcheck");
    vcostrings.push_back("Output");
    vcostrings.push_back(Form("%d", ic));
    vcostrings.push_back("Scan");
    vcostrings.push_back("error_rate");
    string current = readFromJson(filename, vcostrings); 
    
    vector<pair<double, double> > result = combine2Lines(voltage, current); 
    allResults.push_back(result);
  }
       
  vector<TGraphErrors *> vg; 
  for (unsigned int i = 0; i < allResults.size(); ++i) {
    vg.push_back(makeGraph(allResults[i], color, 24+i, 1.));
  }
  return vg;
}


// ----------------------------------------------------------------------
void linkqualiTests(int parno = 0, int layer = 1) {
  vector<string> files, titles;
  vector<int> cols, ladderNumbers;

  files.clear();
  cols.clear();
  loadFiles(files, cols, titles, ladderNumbers, layer); 

  TCanvas c1;
  c1.SetLogy(1);
  TH1D *h1 = new TH1D("h1", "", 10, 0., 100.);
  h1->SetMaximum(1.e10);
  h1->SetMinimum(1.e-3);
  h1->GetXaxis()->SetTitle("VPVCO [DAC]");
  h1->GetYaxis()->SetTitle("error rate"); 
  h1->GetYaxis()->SetTitleOffset(1.1);
  gStyle->SetOptStat(0);
  
  h1->Draw();

  for (unsigned int i = 0; i < files.size(); ++i) {
    cout << "*** i = " << i << " " << files[i] << endl;
    vector<TGraphErrors *> gr = linkqualiTest(files[i], kBlue);
    int hldr = ladderNumbers[i];
    vector<double>  vchips; 
    vector<int> colors  = {kBlue-2, kGreen+2, kRed-3};
    // -- fit pol1 to each of the three graphs
    if (0 == parno) {
      c1.cd();
      c1.Clear();
      h1->Draw();
    }
    for (unsigned int ig = 0; ig < gr.size(); ++ig) {
      gr[ig]->SetLineWidth(1);
      double minErr(1.e99), minVCO(0.), val(0.);
      for (int ix = 0; ix < gr[ig]->GetN(); ++ix) {
        if ((gr[ig]->GetPointY(ix) > 0.) && gr[ig]->GetPointY(ix) < minErr) {
          minVCO = gr[ig]->GetPointX(ix);
          minErr = gr[ig]->GetPointY(ix);
        }
      }
      if (0 == parno) {
        if (minErr > 1.e98) {
          val = 1.e-6;
        } else {
          val = minErr;
        }            
      } else if (1 == parno) {
        val = minVCO;
      }
      vchips.push_back(val);
      cout << "ig = " << ig << " minErr = " << minErr << " minVCO = " << minVCO << " parno = " << parno << endl;

      if (0 == parno) {
        gr[ig]->SetMarkerColor(colors[ig]);
        gr[ig]->SetLineColor(colors[ig]);
        gr[ig]->SetLineStyle(kDashed);
        gr[ig]->Draw("lp");
      }
    }
    
    if (0 == parno) {
      c1.SaveAs(Form("linkquali-%d.pdf", hldr));
    }
    gLayout.insert(make_pair(hldr, vchips));
      
  }

  return;
}


// ----------------------------------------------------------------------
vector<TGraphErrors *> dacscanTest(string dacname = "VPDAC", string filename= "qc_ladder_0.json", int color = 1) {
  cout << "reading json from file ->" << filename << "<-" << endl;
  vector<string> ivstrings;
  
  vector<vector<pair<double, double> > > allResults;
  
  for (int ic = 0; ic < 3; ++ic) {
    ivstrings.clear();
    ivstrings.push_back("DACScan");
    ivstrings.push_back("Output");
    ivstrings.push_back(dacname);
    ivstrings.push_back(Form("%d", ic));
    ivstrings.push_back(dacname + "_values");
    string voltage = readFromJson(filename, ivstrings); 
    ivstrings.clear();
    ivstrings.push_back("DACScan");
    ivstrings.push_back("Output");
    ivstrings.push_back(dacname);
    ivstrings.push_back(Form("%d", ic));
    ivstrings.push_back(dacname + "_current");
    string current = readFromJson(filename, ivstrings); 
    
    vector<pair<double, double> > result = combine2Lines(voltage, current); 

    // -- remove trailing 0
    vector<pair<double, double> >::iterator it;
    for (it = result.begin() + 1; it != result.end(); ++it) {
      if (0 == it->first) break;
    }
    result.erase(it, result.end());
    
    allResults.push_back(result);
  }
       
  vector<TGraphErrors *> vg; 
  for (unsigned int i = 0; i < allResults.size(); ++i) {
    vg.push_back(makeGraph(allResults[i], color, 24+i, 1.));
  }
  return vg;
}




// ----------------------------------------------------------------------
void dacscanTests(string dacname = "VPDAC", int parno = 0, int layer = 1) {
  vector<string> files, titles;
  vector<int> cols, ladderNumbers;

  files.clear();
  cols.clear();
  loadFiles(files, cols, titles, ladderNumbers, layer); 

  TCanvas c1;
  TH1D *h1(0);
  if (string::npos != dacname.find("VPDAC")) {
    h1 = new TH1D("h1", "h1", 10, 0., 10.);
  } else if (string::npos != dacname.find("ref_Vss")) {
    h1 = new TH1D("h1", "h1", 10, 0., 300.);
  } else {
    h1 = new TH1D("h1", "h1", 10, 0., 100.);
  }
  h1->SetMaximum(2.);
  h1->SetMinimum(0.);
  h1->GetXaxis()->SetTitle(Form("%s [DAC]", dacname.c_str()));
  h1->GetYaxis()->SetTitle("Current [A]");
  h1->GetYaxis()->SetTitleOffset(1.1);
  
  h1->Draw();

  for (unsigned int i = 0; i < files.size(); ++i) {
    cout << "*** i = " << i << " " << files[i] << endl;
    vector<TGraphErrors *> gr = dacscanTest(dacname, files[i], kBlue);
    int hldr = ladderNumbers[i];
    vector<double>  vchips; 
    vector<int> colors  = {kBlue-2, kGreen+2, kRed-3};
    // -- fit pol1 to each of the three graphs
    if (0 == parno) {
      c1.cd();
      c1.Clear();
      h1->Draw();
    }
    for (unsigned int ig = 0; ig < gr.size(); ++ig) {
      gr[ig]->SetLineWidth(1);
      int fitstatus = gr[ig]->Fit("pol1", "w");
      double offset(0.), slope(0.), val(0.);
      if (0 == fitstatus) {
        TF1 *f = (TF1*)gr[ig]->GetFunction("pol1");
        f->SetLineColor(colors[ig]);
        offset = f->GetParameter(0);
        slope = f->GetParameter(1);
        if (0 == parno) {
          val = offset;
        } else if (1 == parno) {
          val = slope;
        }
      } else {
        val = -99.;
      }
      vchips.push_back(val);
      cout << "ig = " << ig << " offset = " << offset << " slope = " << slope << " parno = " << parno
           << " fit status = " << fitstatus
           << endl;

      if (0 == parno) {
        gr[ig]->SetMarkerColor(colors[ig]);
        gr[ig]->SetLineColor(colors[ig]);
        gr[ig]->SetLineStyle(kDashed);
        gr[ig]->Draw("lp");
      }
    }
    
    if (0 == parno) {
      c1.SaveAs(Form("dacscan-%s-%d.pdf", dacname.c_str(), hldr));
    }
    gLayout.insert(make_pair(hldr, vchips));
      
  }

  return;
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
  vector<string> files, titles;
  vector<int> cols, ladderNumbers;

  files.clear();
  cols.clear();
  loadFiles(files, cols, titles, ladderNumbers, layer); 
  
  TH1D *h1 = new TH1D("h1", "h1", 30, 0., 30.);
  h1->SetMaximum(0.5);
  h1->SetMinimum(-12.0);
  h1->GetXaxis()->SetTitle("Bias voltage [-V]");
  h1->GetYaxis()->SetTitle("Current [#muA]");
  h1->GetYaxis()->SetTitleOffset(1.1);
  
  h1->Draw();
  
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

  TLegend *tll = newLegend(0.7, 0.45, 0.88, 0.75);
  if (0 == layer) {
    tll->SetTextSize(0.025);
    tll->SetHeader("Layer 0");
  } else if (1 == layer) {
    tll->SetTextSize(0.020);
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
void lvTest(string filename= "qc_ladder_0.json", int hldr = 0) {
  cout << "reading json from file ->" << filename << "<-" << endl;
  vector<string> ivstrings;
  ivstrings.push_back("LVPowerOn");
  ivstrings.push_back("Output");
  ivstrings.push_back("current_increase");
  string currentIncrease = readFromJson(filename, ivstrings); 
  
  vector<double> result = split1Line(currentIncrease); 
  for (unsigned int i = 0; i < result.size(); ++i) {
    cout << "hldr = " << hldr << " i = " << i << " result = " << result[i] << endl;
  }
  gLayout.insert(make_pair(hldr, result));
}


// ----------------------------------------------------------------------
void lvTests(int layer = 0) {
  vector<string> files, titles;
  vector<int> cols, ladderNumbers;
  
  files.clear();
  cols.clear();
  loadFiles(files, cols, titles, ladderNumbers, layer); 
 
  for (unsigned int i = 0; i < files.size(); ++i) {
    lvTest(files[i], ladderNumbers[i]);
    cout << "gLayout.size() = " << gLayout.size() << endl;
  }
}

// ----------------------------------------------------------------------
void displayMap(TH2D *hl0, TH2D *hl1, int logz = 0) {
  c0.SetWindowSize(800, 400); 

  // -- Layer 0
  zone(2,1);
  gPad->SetLeftMargin(0.2);
  gPad->SetRightMargin(0.2);
  gPad->SetLogz(logz);
  hl0->Draw("colz");

  gPad->Update();

  TPaletteAxis *palette = (TPaletteAxis*)hl0->GetListOfFunctions()->FindObject("palette");
  palette->SetX1NDC(0.85);
  palette->SetX2NDC(0.87);

  // -- draw an axis on the right side
  TGaxis *aR0 = new TGaxis(gPad->GetUxmax(), gPad->GetUymin(),
                          gPad->GetUxmax(), gPad->GetUymax(), 18, 26, 8, "+L");
  aR0->CenterLabels();  aR0->SetTickLength(0.);
  aR0->Draw();

  // -- draw an axis on the left side
  TGaxis *aL0 = new TGaxis(gPad->GetUxmin(), gPad->GetUymin(),
                          gPad->GetUxmin(), gPad->GetUymax(), 0, 8, 8, "-R");
  aL0->CenterLabels();  aL0->SetTickLength(0.);
  aL0->Draw();

  tl->SetTextSize(0.08);
  tl->SetTextColor(kBlack);
  tl->DrawLatexNDC(0.4, 0.92, "Layer 0");
  
  tl->SetTextColor(kGray);
  tl->SetTextSize(0.002);
  tl->DrawLatexNDC(0.2, 0.8978, "UL");

  gPad->Modified();
  gPad->Update();

  // -- Layer 1
  c0.cd(2);
  gPad->SetLeftMargin(0.2);
  gPad->SetRightMargin(0.2);
  gPad->SetGridx(1);
  gPad->SetGridy(1);
  gPad->SetLogz(logz);
  hl1->Draw("colz");
  gPad->Update();

  palette = (TPaletteAxis*)hl1->GetListOfFunctions()->FindObject("palette");
  palette->SetX1NDC(0.85);
  palette->SetX2NDC(0.87);

  // -- draw an axis on the right side
  TGaxis *aR1 = new TGaxis(gPad->GetUxmax(), gPad->GetUymin(),
                          gPad->GetUxmax(), gPad->GetUymax(), 26, 36, 10, "+L");
  aR1->CenterLabels();  aR1->SetTickLength(0.);
  aR1->Draw();

  // -- draw an axis on the left side
  TGaxis *aL1 = new TGaxis(gPad->GetUxmin(), gPad->GetUymin(),
                          gPad->GetUxmin(), gPad->GetUymax(), 8, 18, 10, "-R");
  aL1->CenterLabels();  aL1->SetTickLength(0.);
  aL1->Draw();


  tl->SetTextSize(0.08);
  tl->SetTextColor(kBlack);
  tl->DrawLatexNDC(0.4, 0.92, "Layer 1");

  tl->SetTextColor(kGray);
  tl->SetTextSize(0.002);
  tl->DrawLatexNDC(0.2, 0.8978, "UL");

  gPad->Modified();
  gPad->Update();
 
}


// ----------------------------------------------------------------------
void mapIV() {
  if (gLayout.size() < 2) {
    ivTests(0); 
    ivTests(1); 
  }

  TH2D *hl0 = new TH2D("L0", "Layer 0", 6, 0., 6.,  8, 0., 8.);
  hl0->SetMaximum(30.);
  hl0->SetMinimum(-0.1);
  hl0->SetNdivisions(600, "X");
  hl0->SetNdivisions(0, "Y");
  hl0->GetZaxis()->SetTitle("V_{B}");
  hl0->GetZaxis()->SetTitleOffset(0.8);
  hl0->GetZaxis()->SetTitleSize(0.06);
  // -- initialize histogram to indicate untested hldr
  for (int ix = 0; ix < hl0->GetNbinsX(); ++ix) {
    for (int iy = 0; iy < hl0->GetNbinsY(); ++iy) {
      hl0->SetBinContent(ix+1, iy+1, -99.);
    }
  }
  
  TH2D *hl1 = new TH2D("L1", "Layer 1", 6, 0., 6., 10, 0., 10.);
  hl1->SetNdivisions(600, "X");
  hl1->SetNdivisions(0, "Y");
  hl1->SetMaximum(30.);
  hl1->SetMinimum(-0.1);
  hl1->GetZaxis()->SetTitle("V_{B}");
  hl1->GetZaxis()->SetTitleOffset(0.8);
  hl1->GetZaxis()->SetTitleSize(0.06);
  // -- initialize histogram to indicate untested hldr
  for (int ix = 0; ix < hl1->GetNbinsX(); ++ix) {
    for (int iy = 0; iy < hl1->GetNbinsY(); ++iy) {
      hl1->SetBinContent(ix+1, iy+1, -99.);
    }
  }
  
  // -- fixed coloring
  Int_t    colors[] = {kRed, kRed-9, kBlue-6, kBlue-4, kGreen+1, kGreen+2};
  Double_t levels[] = {0.,   5.,     10.,     15.,     20.,      25.,       30.};
  gStyle->SetPalette((sizeof(colors)/sizeof(Int_t)), colors);
  
  hl0->SetContour((sizeof(levels)/sizeof(Double_t)), levels);
  hl1->SetContour((sizeof(levels)/sizeof(Double_t)), levels);

  map<int, vector<double> >::iterator it;
  for (it = gLayout.begin(); it != gLayout.end(); ++it) {
    int hldr = it->first; 
    int cmpl = it->second[0]; 
    cout << "HLDR " << hldr << " cmpl = " << cmpl << endl;
    if (0 == hldrLayer(hldr)) {
      hl0->SetBinContent(3*hldrBin(hldr).second + 1, hldrBin(hldr).first, cmpl);
      hl0->SetBinContent(3*hldrBin(hldr).second + 2, hldrBin(hldr).first, cmpl);
      hl0->SetBinContent(3*hldrBin(hldr).second + 3, hldrBin(hldr).first, cmpl);
    } else {
      hl1->SetBinContent(3*hldrBin(hldr).second + 1, hldrBin(hldr).first, cmpl);
      hl1->SetBinContent(3*hldrBin(hldr).second + 2, hldrBin(hldr).first, cmpl);
      hl1->SetBinContent(3*hldrBin(hldr).second + 3, hldrBin(hldr).first, cmpl);
    }
  }
  
  displayMap(hl0, hl1);
  c0.cd(1);
  drawLadderGrid(8);
  c0.cd(2);
  drawLadderGrid(10);

  c0.SaveAs("map-iv.pdf");
}


// ----------------------------------------------------------------------
void mapLV() {
  if (gLayout.size() < 2) {
    lvTests(0); 
    lvTests(1); 
  }

  gStyle->SetOptStat(0);
  gStyle->SetOptTitle(0);
  
  TH2D *hl0 = new TH2D("L0", "Layer 0", 6, 0., 6.,  8, 0., 8.);
  hl0->SetMaximum(0.5);
  hl0->SetMinimum(-0.1);
  hl0->SetNdivisions(600, "X");
  hl0->SetNdivisions(0, "Y");
  hl0->GetZaxis()->SetTitle("#Delta(I) [A]");
  hl0->GetZaxis()->SetTitleOffset(0.8);
  hl0->GetZaxis()->SetTitleSize(0.06);
  // -- initialize histogram to indicate untested hldr
  for (int ix = 0; ix < hl0->GetNbinsX(); ++ix) {
    for (int iy = 0; iy < hl0->GetNbinsY(); ++iy) {
      hl0->SetBinContent(ix+1, iy+1, -99.);
    }
  }
  
  
  TH2D *hl1 = new TH2D("L1", "Layer 1", 6, 0., 6., 10, 0., 10.);
  hl1->SetMaximum(0.5);
  hl1->SetMinimum(-0.1);
  hl1->SetNdivisions(600, "X");
  hl1->SetNdivisions(0, "Y");
  hl1->GetZaxis()->SetTitle("#Delta(I) [A]");
  hl1->GetZaxis()->SetTitleOffset(0.8);
  hl1->GetZaxis()->SetTitleSize(0.06);
  // -- initialize histogram to indicate untested hldr
  for (int ix = 0; ix < hl1->GetNbinsX(); ++ix) {
    for (int iy = 0; iy < hl1->GetNbinsY(); ++iy) {
      hl1->SetBinContent(ix+1, iy+1, -99.);
    }
  }
  
  // -- fixed coloring
  Int_t    colors[] = {kRed, kRed-9, kBlue-6, kBlue-4, kGreen+1, kGreen+2};
  Double_t levels[] = {-0.1, 0.0,    0.1,     0.2,     0.3,      0.35,        0.5};
  gStyle->SetPalette((sizeof(colors)/sizeof(Int_t)), colors);
  
  hl0->SetContour((sizeof(levels)/sizeof(Double_t)), levels);
  hl1->SetContour((sizeof(levels)/sizeof(Double_t)), levels);

  map<int, vector<double> >::iterator it;
  for (it = gLayout.begin(); it != gLayout.end(); ++it) {
    int hldr = it->first; 
    double chip0 = it->second[0]; 
    double chip1 = it->second[1]; 
    double chip2 = it->second[2]; 
    cout << "HLDR " << hldr
         << " chip0 = " << chip0
         << " chip1 = " << chip1
         << " chip2 = " << chip2
         << endl;
    if (0 == hldrLayer(hldr)) {
      if (0 == hldrBin(hldr).second) {
        // -- US needs to be swapped because of firmware mismatch
        hl0->SetBinContent(3*hldrBin(hldr).second + 3, hldrBin(hldr).first, chip0);
        hl0->SetBinContent(3*hldrBin(hldr).second + 2, hldrBin(hldr).first, chip1);
        hl0->SetBinContent(3*hldrBin(hldr).second + 1, hldrBin(hldr).first, chip2);
      } else {
        // -- apparently DS numbering is 2 1 0 (along increasing z)
        hl0->SetBinContent(3*hldrBin(hldr).second + 3, hldrBin(hldr).first, chip0);
        hl0->SetBinContent(3*hldrBin(hldr).second + 2, hldrBin(hldr).first, chip1);
        hl0->SetBinContent(3*hldrBin(hldr).second + 1, hldrBin(hldr).first, chip2);
      }
    } else {
      if (0 == hldrBin(hldr).second) {
        // -- US needs to be swapped because of firmware mismatch
        hl1->SetBinContent(3*hldrBin(hldr).second + 3, hldrBin(hldr).first, chip0);
        hl1->SetBinContent(3*hldrBin(hldr).second + 2, hldrBin(hldr).first, chip1);
        hl1->SetBinContent(3*hldrBin(hldr).second + 1, hldrBin(hldr).first, chip2);
      } else {
        // -- apparently DS numbering is 2 1 0 (along increasing z)
        hl1->SetBinContent(3*hldrBin(hldr).second + 3, hldrBin(hldr).first, chip0);
        hl1->SetBinContent(3*hldrBin(hldr).second + 2, hldrBin(hldr).first, chip1);
        hl1->SetBinContent(3*hldrBin(hldr).second + 1, hldrBin(hldr).first, chip2);
      }
    }
  }

  displayMap(hl0, hl1);
  c0.cd(1);
  drawChipGrid(8);
  c0.cd(2);
  drawChipGrid(10);
  
  c0.SaveAs("map-lv.pdf");
}


// ----------------------------------------------------------------------
void mapDacscan(string dacname = "VPDAC", int parno = 0) {
  if (gLayout.size() < 2) {
    dacscanTests(dacname, parno, 0); 
    dacscanTests(dacname, parno, 1); 
  }
  
  gStyle->SetOptStat(0);
  gStyle->SetOptTitle(0);
  
  TH2D *hl0 = new TH2D("L0", "Layer 0", 6, 0., 6.,  8, 0., 8.);
  hl0->SetNdivisions(600, "X");
  hl0->SetNdivisions(0, "Y");
  // -- initialize histogram to indicate untested hldr
  for (int ix = 0; ix < hl0->GetNbinsX(); ++ix) {
    for (int iy = 0; iy < hl0->GetNbinsY(); ++iy) {
      hl0->SetBinContent(ix+1, iy+1, -99.);
    }
  }
  if (0 == parno) {
    hl0->GetZaxis()->SetTitle(Form("I(%s = 0) [A]", dacname.c_str()));
  } else {
    hl0->GetZaxis()->SetTitle(Form("d(I)/d(%s)", dacname.c_str()));
  }
  hl0->GetZaxis()->SetTitleOffset(1.1);
  hl0->GetZaxis()->SetTitleSize(0.06);
  
  
  TH2D *hl1 = new TH2D("L1", "Layer 1", 6, 0., 6., 10, 0., 10.);
  hl1->SetNdivisions(600, "X");
  hl1->SetNdivisions(0, "Y");
  // -- initialize histogram to indicate untested hldr
  for (int ix = 0; ix < hl1->GetNbinsX(); ++ix) {
    for (int iy = 0; iy < hl1->GetNbinsY(); ++iy) {
      hl1->SetBinContent(ix+1, iy+1, -99.);
    }
  }
  if (0 == parno) {
    hl1->GetZaxis()->SetTitle(Form("I(%s = 0) [A]", dacname.c_str()));
  } else {
    hl1->GetZaxis()->SetTitle(Form("d(I)/d(%s)", dacname.c_str()));
  }
  hl1->GetZaxis()->SetTitleOffset(1.1);
  hl1->GetZaxis()->SetTitleSize(0.06);
  
  // -- fixed coloring
  double dmin(0.), dmax(1.);
  Int_t    colors[] = {kRed, kRed-9, kBlue-6, kBlue-4, kGreen+1, kGreen+2};
  Double_t levels[sizeof(colors)/sizeof(Int_t) + 1];
  if (string::npos != dacname.find("VPDAC")) {
    if (0 == parno) {
      //      dmin = -1.e-4;
      dmin = -0.1;
      dmax = 1.0;
      levels[0] = -0.1; levels[1] = 0.1; levels[2] = 0.3; levels[3] = 0.5; levels[4] = 0.7; levels[5] = 0.9; levels[6] = 1.0;
    } else {
      dmin = -2.e-2;
      //      dmin = -0.1;
      dmax = 0.03;
      levels[0] = 1.5*dmin; levels[1] = 0.0; levels[2] = 0.005; levels[3] = 0.010; levels[4] = 0.015; levels[5] = 0.020; levels[6] = 0.025;
    }
  } else if (string::npos != dacname.find("ref_Vss")) {
    if (0 == parno) {
      dmin = -0.1;
      dmax = 1.0;
      levels[0] = -0.1; levels[1] = 0.1; levels[2] = 0.2; levels[3] = 0.3; levels[4] = 0.4; levels[5] = 0.5; levels[6] = 1.0;
    } else {
      dmin = -2.e-4;
      dmax = 0.0012;
      levels[0] = 1.5*dmin; levels[1] = 0.0; levels[2] = 0.0002; levels[3] = 0.0004; levels[4] = 0.0006; levels[5] = 0.0008; levels[6] = 0.0010;
    }
  }

  hl0->SetMaximum(dmax);
  hl0->SetMinimum(dmin);
  hl1->SetMaximum(dmax);
  hl1->SetMinimum(dmin);

  gStyle->SetPalette((sizeof(colors)/sizeof(Int_t)), colors);
  
  hl0->SetContour((sizeof(levels)/sizeof(Double_t)), levels);
  hl1->SetContour((sizeof(levels)/sizeof(Double_t)), levels);

  map<int, vector<double> >::iterator it;
  for (it = gLayout.begin(); it != gLayout.end(); ++it) {
    int hldr = it->first; 
    double chip0 = it->second[0]; 
    double chip1 = it->second[1]; 
    double chip2 = it->second[2]; 
    cout << "HLDR " << hldr
         << " chip0 = " << chip0
         << " chip1 = " << chip1
         << " chip2 = " << chip2
         << endl;
    if (0 == hldrLayer(hldr)) {
      if (0 == hldrBin(hldr).second) {
        // -- US needs to be swapped because of firmware mismatch
        hl0->SetBinContent(3*hldrBin(hldr).second + 3, hldrBin(hldr).first, chip0);
        hl0->SetBinContent(3*hldrBin(hldr).second + 2, hldrBin(hldr).first, chip1);
        hl0->SetBinContent(3*hldrBin(hldr).second + 1, hldrBin(hldr).first, chip2);
      } else {
        // -- apparently DS numbering is 2 1 0 (along increasing z)
        hl0->SetBinContent(3*hldrBin(hldr).second + 3, hldrBin(hldr).first, chip0);
        hl0->SetBinContent(3*hldrBin(hldr).second + 2, hldrBin(hldr).first, chip1);
        hl0->SetBinContent(3*hldrBin(hldr).second + 1, hldrBin(hldr).first, chip2);
      }
    } else {
      if (0 == hldrBin(hldr).second) {
        // -- US needs to be swapped because of firmware mismatch
        hl1->SetBinContent(3*hldrBin(hldr).second + 3, hldrBin(hldr).first, chip0);
        hl1->SetBinContent(3*hldrBin(hldr).second + 2, hldrBin(hldr).first, chip1);
        hl1->SetBinContent(3*hldrBin(hldr).second + 1, hldrBin(hldr).first, chip2);
      } else {
        // -- apparently DS numbering is 2 1 0 (along increasing z)
        hl1->SetBinContent(3*hldrBin(hldr).second + 3, hldrBin(hldr).first, chip0);
        hl1->SetBinContent(3*hldrBin(hldr).second + 2, hldrBin(hldr).first, chip1);
        hl1->SetBinContent(3*hldrBin(hldr).second + 1, hldrBin(hldr).first, chip2);
      }
    }
  }

  displayMap(hl0, hl1);
  c0.cd(1);
  drawChipGrid(8);
  c0.cd(2);
  drawChipGrid(10);
  
  c0.SaveAs(Form("map-%s-par%d.pdf", dacname.c_str(), parno));

  gLayout.clear();
}



// ----------------------------------------------------------------------
void mapLinkQuali(int parno = 0) {
  if (gLayout.size() < 2) {
    linkqualiTests(parno, 0); 
    linkqualiTests(parno, 1); 
  }
  
  gStyle->SetOptStat(0);
  gStyle->SetOptTitle(0);
  
  TH2D *hl0 = new TH2D("L0", "Layer 0", 6, 0., 6.,  8, 0., 8.);
  hl0->SetNdivisions(600, "X");
  hl0->SetNdivisions(0, "Y");
  // -- initialize histogram to indicate untested hldr
  for (int ix = 0; ix < hl0->GetNbinsX(); ++ix) {
    for (int iy = 0; iy < hl0->GetNbinsY(); ++iy) {
      if (0 == parno) {
        hl0->SetBinContent(ix+1, iy+1, 1.e-3);
      } else {
        hl0->SetBinContent(ix+1, iy+1, -99.);
      }
    }
  }
  if (0 == parno) {
    hl0->GetZaxis()->SetTitle("Min. error rate");
  } else {
    hl0->GetZaxis()->SetTitle("VCO(min. error rate)");
  }
  hl0->GetZaxis()->SetTitleOffset(1.1);
  hl0->GetZaxis()->SetTitleSize(0.06);
  
  
  TH2D *hl1 = new TH2D("L1", "Layer 1", 6, 0., 6., 10, 0., 10.);
  hl1->SetNdivisions(600, "X");
  hl1->SetNdivisions(0, "Y");
  // -- initialize histogram to indicate untested hldr
  for (int ix = 0; ix < hl1->GetNbinsX(); ++ix) {
    for (int iy = 0; iy < hl1->GetNbinsY(); ++iy) {
      if (0 == parno) {
        hl1->SetBinContent(ix+1, iy+1, 1.e-3);
      } else {
        hl1->SetBinContent(ix+1, iy+1, -99.);
      }        
    }
  }
  if (0 == parno) {
    hl1->GetZaxis()->SetTitle("Min. error rate");
  } else {
    hl1->GetZaxis()->SetTitle("VCO(min. error rate)");
  }
  hl1->GetZaxis()->SetTitleOffset(1.1);
  hl1->GetZaxis()->SetTitleSize(0.06);
  
  // -- fixed coloring
  double dmin(0.), dmax(1.);
  Int_t    colors[] = {kGreen+2, kGreen+1, kBlue-4, kBlue-6, kRed-9, kRed};
  Double_t levels[sizeof(colors)/sizeof(Int_t) + 1];
  if (0 == parno) {
    //      dmin = -1.e-4;
    dmin = 1.e-1;
    dmax = 1.e10;
    levels[0] = 1.e-1; levels[1] = 1.e2; levels[2] = 1.e3; levels[3] = 1.e5; levels[4] = 1.e7; levels[5] = 1.e9; levels[6] = 1.e10;
  } else {
    dmin = 0.1;
    //      dmin = -0.1;
    dmax = 40.1;
    levels[0] = 0.; levels[1] = 5.; levels[2] = 10.; levels[3] = 20.; levels[4] = 30.; levels[5] = 40.; levels[6] = 41.;
  }
  
  hl0->SetMaximum(dmax);
  hl0->SetMinimum(dmin);
  hl1->SetMaximum(dmax);
  hl1->SetMinimum(dmin);

  gStyle->SetPalette((sizeof(colors)/sizeof(Int_t)), colors);
  
  hl0->SetContour((sizeof(levels)/sizeof(Double_t)), levels);
  hl1->SetContour((sizeof(levels)/sizeof(Double_t)), levels);

  map<int, vector<double> >::iterator it;
  for (it = gLayout.begin(); it != gLayout.end(); ++it) {
    int hldr = it->first; 
    double chip0 = it->second[0]; 
    double chip1 = it->second[1]; 
    double chip2 = it->second[2]; 
    cout << "HLDR " << hldr
         << " chip0 = " << chip0
         << " chip1 = " << chip1
         << " chip2 = " << chip2
         << endl;
    if (0 == hldrLayer(hldr)) {
      if (0 == hldrBin(hldr).second) {
        // -- US needs to be swapped because of firmware mismatch
        hl0->SetBinContent(3*hldrBin(hldr).second + 3, hldrBin(hldr).first, chip0);
        hl0->SetBinContent(3*hldrBin(hldr).second + 2, hldrBin(hldr).first, chip1);
        hl0->SetBinContent(3*hldrBin(hldr).second + 1, hldrBin(hldr).first, chip2);
      } else {
        // -- apparently DS numbering is 2 1 0 (along increasing z)
        hl0->SetBinContent(3*hldrBin(hldr).second + 3, hldrBin(hldr).first, chip0);
        hl0->SetBinContent(3*hldrBin(hldr).second + 2, hldrBin(hldr).first, chip1);
        hl0->SetBinContent(3*hldrBin(hldr).second + 1, hldrBin(hldr).first, chip2);
      }
    } else {
      if (0 == hldrBin(hldr).second) {
        // -- US needs to be swapped because of firmware mismatch
        hl1->SetBinContent(3*hldrBin(hldr).second + 3, hldrBin(hldr).first, chip0);
        hl1->SetBinContent(3*hldrBin(hldr).second + 2, hldrBin(hldr).first, chip1);
        hl1->SetBinContent(3*hldrBin(hldr).second + 1, hldrBin(hldr).first, chip2);
      } else {
        // -- apparently DS numbering is 2 1 0 (along increasing z)
        hl1->SetBinContent(3*hldrBin(hldr).second + 3, hldrBin(hldr).first, chip0);
        hl1->SetBinContent(3*hldrBin(hldr).second + 2, hldrBin(hldr).first, chip1);
        hl1->SetBinContent(3*hldrBin(hldr).second + 1, hldrBin(hldr).first, chip2);
      }
    }
  }

  if (0 == parno) {
    displayMap(hl0, hl1, 1);
  } else {
    displayMap(hl0, hl1, 0);
  }
  c0.cd(1);
  drawChipGrid(8);
  c0.cd(2);
  drawChipGrid(10);
  
  c0.SaveAs(Form("map-linkQuali-par%d.pdf", parno));

  gLayout.clear();
}


// ----------------------------------------------------------------------
void ladderTests() {
  gLayout.clear();
  mapIV();
  
  gLayout.clear();
  mapLV();

  gLayout.clear();
  mapDacscan("ref_Vss", 0);
  gLayout.clear();
  mapDacscan("ref_Vss", 1);

  gLayout.clear();
  mapDacscan("VPDAC", 0);
  gLayout.clear();
  mapDacscan("VPDAC", 1);

  gLayout.clear();
  mapLinkQuali(0);
  gLayout.clear();
  mapLinkQuali(1);

}


