#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <string.h>
#include <chrono>

#include "Mu3eConditions.hh"
#include "cdbUtil.hh"
#include "calPixelQuality.hh"
#include "calPixelQualityV.hh"
#include "calPixelQualityM.hh"

#include "cdbJSON.hh"
#include "base64.hh"

#include "TCanvas.h"
#include "TStyle.h"
#include "TFile.h"
#include "TH2F.h"
#include "TMath.h"
#include "TKey.h"
#include "TROOT.h"

#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>

using namespace std;

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::sub_array;
using bsoncxx::builder::basic::sub_document;
using bsoncxx::builder::basic::make_document;

// ----------------------------------------------------------------------
// pcrPixelQuality
// ---------------
//
// Examples: 
// bin/pcrPixelQuality pcr-run00265.root
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
struct pixhit {
  unsigned int ichip;
  int icol, irow;
  int tot;
  int status;
};

// ----------------------------------------------------------------------
void createPayload(string , calAbs *, map<unsigned int, vector<double> >);


// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {

  // -- command line arguments
  int verbose(0), mode(1);
  // note: mode = 1 PixelQuality, 2 PixelQualityV, 3 PixelQualityM
  string filename("nada.root");
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i], "-f"))      {filename = argv[++i];}
    if (!strcmp(argv[i], "-m"))      {mode = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-v"))      {verbose = atoi(argv[++i]);}
  }
  
  string gt("mdc2023");
  cdbAbs *pDB = new cdbJSON(gt, "json", verbose);

  calAbs *cpq(0);
  if (1 == mode) {
    cpq = new calPixelQuality();
  } else if (2 == mode) {
    cpq = new calPixelQualityV();
  } else if (3 == mode) {
    cpq = new calPixelQualityM();
  }
  
  map<unsigned int, vector<double> > mdet{};
  
  string hash("tag_pixelquality_mdc2023_iov_1");

  TFile *f = TFile::Open(filename.c_str()); 

  // -- read in all chipids
  TIter next(gDirectory->GetListOfKeys());
  TKey *key(0);
  vector<unsigned int> vchipid;
  while ((key = (TKey*)next())) {
    if (gROOT->GetClass(key->GetClassName())->InheritsFrom("TDirectory")) continue;
    TH1 *sig = (TH1*)key->ReadObj();
    TString hname(sig->GetName());
    if (hname.Contains("chip")) {
      string  sChip = hname.Data();
      cout << sChip << endl;
      replaceAll(sChip, "chip", ""); 
      int ichip(-1);
      ichip = ::stoi(sChip);
      if (ichip > -1) vchipid.push_back(ichip); 
    }
  }

  // -- loop over all chipids and determine noisy pixels VERY naively
  for (unsigned int i = 0; i < vchipid.size(); ++i) {
    unsigned int chipID = vchipid[i];
    TH2F *h2 = (TH2F*)(f->Get(Form("chip%d", static_cast<int>(chipID))));
    int chipCnt = h2->GetSumOfWeights();
    int npix(0);
    for (int ix = 1; ix <= h2->GetNbinsX(); ++ix) {
      for (int iy = 1; iy <= h2->GetNbinsY(); ++iy) {
        if (h2->GetBinContent(ix,iy) > 0) ++npix;
      }
    }
    cout << "chipID = " << chipID << " nhits =  " << chipCnt << " for npix = " << npix << endl;

    double NSIGMA(10.0); 
    double meanHits  = (npix > 0? static_cast<double>(chipCnt)/npix : 0.);
    // -- this is WRONG!
    double meanHitsE = TMath::Sqrt(meanHits);
    double noiseThr  = meanHits + NSIGMA*meanHitsE; 
    if (meanHits > 0.) cout << "meanHits = " << meanHits << " +/- " << meanHitsE << " noise threshold = " << noiseThr << endl;
    vector<double> vchip; 
    
    for (int ix = 1; ix <= h2->GetNbinsX(); ++ix) {
      for (int iy = 1; iy <= h2->GetNbinsY(); ++iy) {
        float nhits = h2->GetBinContent(ix,iy);
        if (nhits > noiseThr) {
          vchip.push_back(static_cast<double>(ix-1)); 
          vchip.push_back(static_cast<double>(iy-1)); 
          vchip.push_back(static_cast<double>(1)); 
          cout << "  noisy pixel at icol/irow = " << ix-1 << "/" << iy-1  << " nhits = " << nhits << endl;
        }
      }
    }
    mdet.insert(make_pair(chipID, vchip));
  }
  
  
  createPayload(hash, cpq, mdet);
}

// ----------------------------------------------------------------------  
void createPayload(string hash, calAbs *a, map<unsigned int, vector<double> > mdet) {
 
  string sblob = a->makeBLOB(mdet);
  
  payload pl;
  pl.fHash = hash;
  pl.fComment = "testing";
  pl.fBLOB = sblob;
  a->writePayloadToFile(hash, ".", pl); 
}


