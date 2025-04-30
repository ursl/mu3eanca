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

using namespace std;


// ----------------------------------------------------------------------
// pcrPixelQuality
// ---------------
//
// Examples:
// bin/pcrPixelQuality pcr-run00265.root
// ----------------------------------------------------------------------

#define MDCJSONDIR "/psi/home/langenegger/data/mdc2023/json"

// ----------------------------------------------------------------------
struct pixhit {
  unsigned int ichip;
  int icol, irow;
  int tot;
  int status;
};

// ----------------------------------------------------------------------
void createPayload(string, calAbs *, map<unsigned int, vector<double> >, string);


// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {

  // -- command line arguments
  int verbose(0), mode(1);
  // note: mode = 1 PixelQuality, 2 PixelQualityV, 3 PixelQualityM
  string jsondir(MDCJSONDIR), filename("nada.root");
  string gt("intrun");
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-f"))      {filename = argv[++i];}
    if (!strcmp(argv[i], "-g"))      {gt = argv[++i];}
    if (!strcmp(argv[i], "-j"))      {jsondir = argv[++i];}
    if (!strcmp(argv[i], "-m"))      {mode = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-v"))      {verbose = atoi(argv[++i]);}
  }
  
  cdbAbs *pDB = new cdbJSON(gt, jsondir, verbose);
  
  calAbs *cpq(0);
  if (1 == mode) {
    cpq = new calPixelQuality();
  } else if (2 == mode) {
    cpq = new calPixelQualityV();
  } else if (3 == mode) {
    cpq = new calPixelQualityM();
  }
  
  map<unsigned int, vector<double> > mdet{};
  
  int run(-1);
  string sbla(filename);
  if (string::npos != filename.find_last_of("/")) {
    string dir = filename.substr(0, filename.find_last_of("/")+1);
    cout << "dir ->" << dir << "<-" << endl;
    replaceAll(sbla, dir, "");
  }
  // -- extract the runnumber. It follows directly after "run".
  replaceAll(sbla, ".root", "");
  size_t rIdx = sbla.find("run")+3;
  sbla = sbla.substr(rIdx);
  cout << "sbla ->" << sbla << "<-" << endl;
  run = ::stoi(sbla);
  cout << "run = " << run << endl;
  
  string hash = string("tag_pixelquality_") + gt + string("_iov_") + to_string(run);
  
  TFile *f = TFile::Open(filename.c_str());
  
  // -- read in all chipids
  TIter next(gDirectory->GetListOfKeys());
  TKey *key(0);
  vector<unsigned int> vchipid;
  cout << "pcrPixelQuality look at chipIDs: ";
  while ((key = (TKey*)next())) {
    if (gROOT->GetClass(key->GetClassName())->InheritsFrom("TDirectory")) continue;
    TH1 *sig = (TH1*)key->ReadObj();
    TString hname(sig->GetName());
    if (hname.Contains("chip")) {
      string  sChip = hname.Data();
      replaceAll(sChip, "chip", "");
      int ichip(-1);
      ichip = ::stoi(sChip);
      if (ichip > -1) {
        cout << ichip << " ";
        vchipid.push_back(ichip);
      }
    }
  }
  cout << endl;
  
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
    if (verbose > 0) cout << "chipID = " << chipID << " nhits =  " << chipCnt << " for npix = " << npix << endl;
    
    vector<double> vchip{};
    if (npix > 0) {
      double NSIGMA(10.0);
      double meanHits  = static_cast<double>(chipCnt)/npix;
      // -- this is WRONG!
      double meanHitsE = TMath::Sqrt(meanHits);
      double noiseThr  = meanHits + NSIGMA*meanHitsE;
      if (meanHits > 0.) {
        if (verbose > 0) cout << "meanHits = " << meanHits << " +/- " << meanHitsE << " noise threshold = " << noiseThr << endl;
      }
      int nNoisyPix(0);
      for (int ix = 1; ix <= h2->GetNbinsX(); ++ix) {
        for (int iy = 1; iy <= h2->GetNbinsY(); ++iy) {
          float nhits = h2->GetBinContent(ix,iy);
          if (nhits > noiseThr) {
            ++nNoisyPix;
            vchip.push_back(static_cast<double>(ix-1));
            vchip.push_back(static_cast<double>(iy-1));
            vchip.push_back(static_cast<double>(1));
            if (verbose > 0) {
              cout << "  noisy pixel at icol/irow = " << ix-1 << "/" << iy-1  << " nhits = " << nhits << endl;
            }
          }
        }
      }
      if (verbose > 0) cout << "  -> nNoisyPixel = " << nNoisyPix << endl;
    }
    mdet.insert(make_pair(chipID, vchip));
  }
  
  createPayload(hash, cpq, mdet, jsondir + string("/payloads"));
  
  // -- validate written payload
  cpq->readPayloadFromFile(hash, jsondir + string("/payloads"));
  //  cout << "######################################################################" << endl;
  //  cout << "### validate payload" << endl;
  string sblob = pDB->getPayload(hash).fBLOB;
  //  cpq->printBLOB(sblob);
  cpq->printBLOB(sblob, 0);
  
  
}

// ----------------------------------------------------------------------
void createPayload(string hash, calAbs *a, map<unsigned int, vector<double> > mdet, string jsondir) {

  string sblob = a->makeBLOB(mdet);
  
  payload pl;
  pl.fHash = hash;
  pl.fComment = "testing";
  pl.fBLOB = sblob;
  //  cout << "######################################################################" << endl;
  //  cout << "### createPayload" << endl;
  //  a->printBLOB(sblob);
  
  a->writePayloadToFile(hash, jsondir, pl);
}
