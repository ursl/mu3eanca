#include "plotHitDataPixel.hh"

#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>


#include "TROOT.h"
#include "TStyle.h"
#include "TKey.h"
#include "TMath.h"
#include "TMarker.h"
#include "TPad.h"
#include "TRandom3.h"
#include "TString.h"
#include "TCanvas.h"
#include "TLorentzVector.h"
#include "TPad.h"
#include "TF1.h"
#include "THStack.h"
#include "TFitResult.h"
#include "TTimeStamp.h"

#include "util/dataset.hh"
#include "util/util.hh"

ClassImp(plotHitDataPixel)

using namespace std;

// ----------------------------------------------------------------------
plotHitDataPixel::plotHitDataPixel(string dir, string files, string cuts, string setup):
  plotClass(dir, files, cuts, setup) {
  plotClass::loadFiles(files);
  plotHitDataPixel::loadFiles(files);

  changeSetup(dir, "plotHitDataPixel", setup);
}


// ----------------------------------------------------------------------
plotHitDataPixel::~plotHitDataPixel() {
  cout << "plotHitDataPixel destructor" << endl;
  resetHistograms(true);
  cout << "done with histogram deletion" << endl;
}

// ----------------------------------------------------------------------
void plotHitDataPixel::resetHistograms(bool deleteThem) {

}

// ----------------------------------------------------------------------
void plotHitDataPixel::makeAll(string what) {
  fHistFile = TFile::Open(fHistFileName.c_str(), "RECREATE");
  
  play();
  closeHistFile();
}


// ----------------------------------------------------------------------
void plotHitDataPixel::play() {
  TTree *t = getTree("pixelData");
  t->Print();
  gFile->ls();
  TH2D *h2 = new TH2D("hitmap", "", 256, 0., 256., 250, 0., 250.);
  //  TTree*t = (TTree*)
}

// ----------------------------------------------------------------------
void plotHitDataPixel::loopOverTree(TTree *t, int ifunc, int nevts, int nstart) {
  int nentries = Int_t(t->GetEntries());
  int nbegin(0), nend(nentries);
  if (nevts > 0 && nentries > nevts) {
    nentries = nevts;
    nbegin = 0;
    nend = nevts;
  }
  if (nevts > 0 && nstart > 0) {
    nentries = nstart + nevts;
    nbegin = nstart;
    if (nstart + nevts < t->GetEntries()) {
      nend = nstart + nevts;
    } else {
      nend = t->GetEntries();
    }
  }

  nentries = nend - nstart;

  int step(1000000);
  if (nentries < 5000000)  step = 500000;
  if (nentries < 1000000)  step = 100000;
  if (nentries < 100000)   step = 10000;
  if (nentries < 10000)    step = 1000;
  if (nentries < 1000)     step = 100;
  cout << "==> plotHitDataPixel::loopOverTree> loop over dataset " << fCds->fName << " in file "
       << t->GetDirectory()->GetName()
       << " with " << nentries << " entries"
       << " nbegin = " << nbegin << " nend = " << nend
       << endl;

  // -- setup loopfunction through pointer to member functions
  //    (this is the reason why this function is NOT in plotClass!)
  void (plotHitDataPixel::*pF)(void);
  if (ifunc == 1) pF = &plotHitDataPixel::loopFunction1;
  if (ifunc == 2) pF = &plotHitDataPixel::loopFunction2;

  // ----------------------------
  // -- the real loop starts here
  // ----------------------------
  for (int jentry = nbegin; jentry < nend; jentry++) {
    t->GetEntry(jentry);
    if (jentry%step == 0) {
      TTimeStamp ts;
      cout << Form(" .. evt = %d", jentry)
           << ", time now: " << ts.AsString("lc")
           << endl;
    }

    candAnalysis();
    (this->*pF)();
  }
}

// ----------------------------------------------------------------------
void plotHitDataPixel::loopFunction1() {

}


// ----------------------------------------------------------------------
void plotHitDataPixel::loopFunction2() {

}



// ----------------------------------------------------------------------
void plotHitDataPixel::loadFiles(string afiles) {

  string files = fDirectory + string("/") + afiles;
  cout << "==> plotHitDataPixel::loadFile loading files listed in " << files << endl;

  char buffer[1000];
  ifstream is(files.c_str());
  while (is.getline(buffer, 1000, '\n')) {
    if (buffer[0] == '#') {continue;}
    if (buffer[0] == '/') {continue;}

    string sbuffer = string(buffer);
    replaceAll(sbuffer, " ", "");
    replaceAll(sbuffer, "\t", "");
    if (sbuffer.size() < 1) continue;

    string::size_type m1 = sbuffer.find("lumi=");
    string stype = sbuffer.substr(5, m1-5);

    string::size_type m2 = sbuffer.find("file=");
    string slumi = sbuffer.substr(m1+5, m2-m1-6);
    string sfile = sbuffer.substr(m2+5);
    string sname("nada"), sdecay("nada");

    TFile *pF(0);
    dataset *ds(0);

    if (string::npos != stype.find("Data")) {
      // -- Charmonium
      pF = loadFile(sfile);
      ds = new dataset();
      ds->fSize = 1.2;
      ds->fWidth = 2;
      if (string::npos != stype.find("blablabla")) {
        sname = "bmmCharmonium";
        sdecay = "bmm";
        ds->fColor = kBlack;
        ds->fSymbol = 20;
        ds->fF      = pF;
        ds->fBf     = 1.;
        ds->fMass   = 1.;
        ds->fFillStyle = 3365;
      }
    }
  }
}
