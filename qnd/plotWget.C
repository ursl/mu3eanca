#include <iostream>
#include <fstream>
#include <string>

void draw(string filename = "bla") {
  ifstream INS;
  INS.open(filename);
  string sline; 
  TH1D *h1 = new TH1D("bandwidth", "bandwidth (hepgrid11.ph.liv.ac.uk - merlin)", 100, 0., 100.);
  setTitles(h1, "bandwidth [MB/s]", "jobs", 0.05, 1.0, 0.9); 
  while (getline(INS, sline)) {
    double rval = ::stod(sline);
    h1->Fill(rval); 
  }
  h1->Draw();
    
}
