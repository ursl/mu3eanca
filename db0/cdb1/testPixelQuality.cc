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
#include "TH1D.h"
#include "TGraph.h"
#include "TRandom3.h"
#include "TMath.h"

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
// testPixelQuality
// ----------------
//
// Examples: 
// bin/testPixelQuality -n 10 -noisy1 100 -noisy2 1100 -nrec1 1000
// bin/testPixelQuality -n 10 -noisy1 100 -noisy2 1100 -nrec1 500
// bin/testPixelQuality -n 10 -noisy1 100 -noisy2 1100 -nrec1 50
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
struct pixhit {
  unsigned int ichip;
  int icol, irow;
  int tot;
  int status;
};

// ----------------------------------------------------------------------
void dumpBlob(string);
string writeBlob(string filename, int nchip, int nnoisy);
void createPayload(string , calAbs *, int, int);
void createRandomHits(map<unsigned int, vector<pixhit> > &, int, int);


// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {

  int NCHIPS(1), NNOISY(2), NRECCHIPS(NCHIPS), NRECHITS(200);
  
  // -- command line arguments
  int verbose(0), mode(1), nevts(2);
  unsigned long int rseed(123456);
  // note: mode = 1 PixelQuality, 2 PixelQualityV, 3 PixelQualityM
  int nchips(NCHIPS);
  int noisy1(0), noisy2(NNOISY);
  int nrec1(NRECHITS), nrec2(NRECHITS);
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i], "-v"))      {verbose = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-m"))      {mode = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-n"))      {nevts = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-r"))      {rseed = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-nchips")) {nchips = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-noisy1")) {noisy1 = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-noisy2")) {noisy2 = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-nrec1"))  {nrec1 = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-nrec2"))  {nrec2 = atoi(argv[++i]);}
  }

  gRandom->SetSeed(rseed);
  
  string gt("mcideal");
  cdbAbs *pDB = new cdbJSON("json", verbose);
  
  Mu3eConditions *pDC = Mu3eConditions::instance(gt, pDB);
  pDC->setVerbosity(verbose);

  int nstep (10);
  
  TCanvas c1;
  long long totalTime(0);
  TH1D *hTime;
  if (1 == mode) {
    hTime = new TH1D("hTime", "hTime", 1000, 0., 1000.);
  } else if (2 == mode) {
    hTime = new TH1D("hTime", "hTime", 1000, 0., 10000.);
  } else if (3 == mode) {
    hTime = new TH1D("hTime", "hTime", 1000, 0., 20000.);
  }
  TH1D *hSetup = new TH1D("hSetup", "hSetup", 1000, 0., 20000.);
  TH1D *hSetup1 = new TH1D("hSetup1", "hSetup 1", 1000, 0., 20000.);
  TH1D *hSetup2 = new TH1D("hSetup2", "hSetup 2", 1000, 0., 20000.);
  TH1D *hSetup3 = new TH1D("hSetup3", "hSetup 3", 1000, 0., 20000.);

  auto grNoise = new TGraph();
  grNoise->SetTitle(Form("Processing time for N(rechit) = %d/chip", nrec1));
  grNoise->GetXaxis()->SetTitle("noisy pixels");
  grNoise->GetYaxis()->SetTitle("time [ms]");
  grNoise->SetMarkerStyle(20);
  grNoise->SetMarkerSize(1.5);
  auto grSetup = new TGraph();
  grSetup->SetTitle("Setup time (payload preparation/reading)");
  grSetup->GetXaxis()->SetTitle("noisy pixels");
  grSetup->GetYaxis()->SetTitle("time [ms]");
  grSetup->SetMarkerStyle(20);
  grSetup->SetMarkerSize(1.5);

  calAbs *cpq(0); 
  int inc = (noisy2-noisy1)/nstep;
  for (int inoise = noisy1; inoise <= noisy2; inoise += inc) {
    cout << "######################################################################" << endl;
    cout << "### inoise = " << inoise
         << " noisy1 = " << noisy1
         << " noisy2 = " << noisy2
         << " inc = " << inc
         << " nstep = " << nstep
         << endl;

    hTime->Reset();
    hSetup->Reset();
    for (int ievt = 0; ievt < nevts; ++ievt) {
      cout << "####### evt " << ievt << endl;
      
      if (1 == mode) {
        cpq = new calPixelQuality();
      } else if (2 == mode) {
        cpq = new calPixelQualityV();
      } else if (3 == mode) {
        cpq = new calPixelQualityM();
      }
      // -- create payload
      auto sbegin = std::chrono::high_resolution_clock::now();
      string hash("tag_pixelquality_mcideal_iov_1");
      createPayload(hash, cpq, nchips, inoise);
      
      cpq->readPayloadFromFile(hash, ".");
      cpq->calculate(hash);

      map<unsigned int, vector<pixhit>> detHits; 
      createRandomHits(detHits, NRECCHIPS, nrec1);
      auto send = std::chrono::high_resolution_clock::now();
      long long duss = chrono::duration_cast<chrono::milliseconds>(send-sbegin).count();
      hSetup->Fill(static_cast<double>(duss));
      
      // -- now loop over all rec hits
      auto tbegin = std::chrono::high_resolution_clock::now();
      for (auto it: detHits) {
        vector<pixhit> v = it.second; 
        //        cout << "it.first = " << it.first << " npix = " << v.size() << endl;
        for (auto ipix: v) {
          if (cpq->getStatus(ipix.ichip, ipix.icol, ipix.irow)) {
            if (0) cout << "skip noisy pixel " << ipix.ichip << "/" << ipix.icol << "/" << ipix.irow << endl;
          }
        }
      }
      auto tend = std::chrono::high_resolution_clock::now();
      long long  dus = chrono::duration_cast<chrono::milliseconds>(tend-tbegin).count();
      auto dura = chrono::duration_cast<chrono::milliseconds>(tend-tbegin).count();
      totalTime += dus;
      hTime->Fill(static_cast<double>(dus));
      cout << "##timing: " << dura << " dus = " << dus << endl;
      delete cpq;
    }
    cout << "##timing/evt: " << totalTime/nevts
         << " TH1D = " << hTime->GetMean() << " +/- " << hTime->GetMeanError() << " (RMS = " << hTime->GetRMS() << ")"
         << " # NCHIPS/NNOISY/NRECHITS = "
         << nchips << "/" << inoise << "/" << nrec1 << " (nrec2 = " << nrec2 << ")"
         << endl;
    
    hTime->SetTitle(Form("timing mode%d-nchips%d-nnoise%d-nrec%d", mode, nchips, inoise, nrec1));
    hTime->Draw();
    c1.SaveAs(Form("hTime-mode%d-nchips%d-nnoise%d-nrec%d.pdf", mode, nchips, inoise, nrec1));

    hSetup->SetTitle(Form("setup mode%d-nchips%d-nnoise%d-nrec%d", mode, nchips, inoise, nrec1));
    hSetup->Draw();
    c1.SaveAs(Form("hSetup-mode%d-nchips%d-nnoise%d-nrec%d.pdf", mode, nchips, inoise, nrec1));
    
    grSetup->AddPoint(inoise, hSetup->GetMean());
    grNoise->AddPoint(inoise, hTime->GetMean());
  }

  c1.Clear();
  grNoise->Draw("alp");
  c1.SaveAs(Form("hNoise-mode%d-nchips%d-nrec%d-noisemax%d.pdf", mode, nchips, nrec1, noisy2));

  c1.Clear();
  grSetup->Draw("alp");
  c1.SaveAs(Form("hSetup-mode%d-nchips%d-nrec%d-noisemax%d.pdf", mode, nchips, nrec1, noisy2));

  cout << "gRandom->Rndm() = " << gRandom->Rndm() << endl;
}


// ----------------------------------------------------------------------
string writeBlob(string filename, int nchip, int nnoisy) {
  map<unsigned int, vector<double> > mdet;
  
  long unsigned int header(0xdeadface);
  stringstream ONS;
  if (1) {
    ONS << dumpArray(uint2Blob(header));
  }
  
  if (1) cout << " nchip = " << nchip
              << " nnoisy = " << nnoisy
              << endl;
  for (int i = 0; i < nchip; ++i) {
    vector<double> vchip{};
    int nnoisypix(nnoisy);
    ONS << dumpArray(uint2Blob(i)) 
        << dumpArray(int2Blob(nnoisypix));
    vchip.push_back(static_cast<double>(nnoisypix));
    
    for (int ipix = 0; ipix < nnoisypix; ++ipix) {
      int icol = 100 + 50*gRandom->Rndm();
      int irow = 120 + 50*gRandom->Rndm();
      char iqual = 1; 
      ONS << dumpArray(int2Blob(icol))
          << dumpArray(int2Blob(irow))
          << dumpArray(uint2Blob(static_cast<unsigned int>(iqual)));
      vchip.push_back(static_cast<double>(icol));
      vchip.push_back(static_cast<double>(irow));
      vchip.push_back(static_cast<double>(iqual));

      if (0) cout << "icol/irow = " << icol << "/" << irow
                  << " qual = " << static_cast<unsigned int>(iqual) 
                  << endl;
    }
    mdet.insert(make_pair(i, vchip));
  }

  return ONS.str();
}


// ----------------------------------------------------------------------
void dumpBlob(string filename, int nchip, int nnoisy) {
  long unsigned int header(0xdeadface);
  ofstream ONS;
  ONS.open(filename);
  if (1) {
    ONS << dumpArray(uint2Blob(header));
  }
  
  if (1) cout << " nchip = " << nchip
              << " nnoisy = " << nnoisy
              << endl;
  for (int i = 0; i < nchip; ++i) {
    int nnoisypix(nnoisy);
    ONS << dumpArray(uint2Blob(i)) 
        << dumpArray(int2Blob(nnoisypix));
    for (int ipix = 0; ipix < nnoisypix; ++ipix) {
      int icol = 100 + 50*gRandom->Rndm();
      int irow = 120 + 50*gRandom->Rndm();
      char iqual = 1; 
      ONS << dumpArray(int2Blob(icol))
          << dumpArray(int2Blob(irow))
          << dumpArray(uint2Blob(static_cast<unsigned int>(iqual)));
      if (0) cout << "icol/irow = " << icol << "/" << irow
                  << " qual = " << static_cast<unsigned int>(iqual) 
                  << endl;
    }
  }

  ONS.close();
}


// ----------------------------------------------------------------------  
void createPayload(string hash, calAbs *a, int nchips, int nnoisy) {
  string tmpfilename("bla");

  if (0) {
    dumpBlob(tmpfilename, nchips, nnoisy);
  } else {
    string blobs = writeBlob(tmpfilename, nchips, nnoisy);
    ofstream ONS;
    ONS.open(tmpfilename);
    ONS << blobs;
    ONS.close();
    
    map<unsigned int, vector<double> > mdet = a->decodeBLOB(blobs);
    string sdet = a->makeBLOB(mdet);
    cout << " a->makeBLOB(mdet): " << endl;
  }
  
  std::ifstream file;
  file.open(tmpfilename);
  vector<char> buffer(std::istreambuf_iterator<char>(file), {});
  string sblob("");
  for (unsigned int i = 0; i < buffer.size(); ++i) sblob.push_back(buffer[i]);

  payload pl;
  pl.fHash = hash;
  pl.fComment = "testing";
  pl.fBLOB = sblob;
  a->writePayloadToFile(hash, ".", pl); 
}


// ----------------------------------------------------------------------
void createRandomHits(map<unsigned int, vector<pixhit> > &dethits, int nrecchips, int nrechits) {

  dethits.clear();
  if (1) cout << " nrecchips = " << nrecchips
              << " nrechits = " << nrechits
              << endl;
  
  pixhit a;
  for (int ic = 0; ic < nrecchips; ++ic) {
    vector<pixhit> v; 
    for (int ih = 0; ih < nrechits; ++ih) {
      a.ichip = ic;
      a.icol = 100 + 60*gRandom->Rndm();
      a.irow = 120 + 50*gRandom->Rndm();
      a.tot  = 50  + 6*gRandom->Rndm();
      a.status = 0;
      v.push_back(a);
    }
    dethits.insert(make_pair(ic, v));
  }
}
