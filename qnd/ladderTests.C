#include <stdio.h>
#include <stdlib.h>

#include "TGraphErrors.h"

#include "../../analyzer/analyzer/utility/json.h"

using namespace std;

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
  for (unsigned int i = 0; i < files.size(); ++i) {
    opts.push_back("p");
    string title = files[i];
    replaceAll(title, ".json", "");
    replaceAll(title, "qc_", "");
    replaceAll(title, "_", " ");
    titles.push_back(title);
  }
  
  vector<TGraphErrors*> hists; 
  for (unsigned int i = 0; i < files.size(); ++i) {
    hists.push_back(ivTest(files[i], cols[i]));
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

// // ----------------------------------------------------------------------
// int main() {
//   iv();
//   return 0;
// }
