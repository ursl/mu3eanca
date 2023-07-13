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

#include "cdbJSON.hh"
#include "base64.hh"

#include "TCanvas.h"
#include "TH1D.h"
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
// 
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
struct pixhit {
  unsigned int ichip;
  int icol, irow;
  int tot;
  int status;
};

// ----------------------------------------------------------------------
void writeBlob(string);
void createPayload(string , calAbs *, int, int);
void createRandomHits(map<unsigned int, vector<pixhit> > &, int, int);


// ----------------------------------------------------------------------
void average(double &av, double &error, vector<double> &val, vector<double> &verr, double &chi2) {

  double e(0.), w8(0.), sumW8(0.), sumAve(0.);
  for (unsigned int i = 0; i < val.size(); ++i) {
    cout << i << " " << val[i] << " +/- " << verr[i];

    // -- calculate mean and error
    e = verr[i];
    if (e > 0.) {
      w8 = 1./(e*e);
      sumW8  += w8;
      sumAve += w8*val[i];
      cout << " w8 = " << w8 << " sumW8 = " << sumW8 << " sumAve = " << sumAve << endl;
    } else {
      cout << "average: Error = 0 for " << val[i] << endl;
      continue;
    }
  }
  if (sumW8 > 0.) {
    av = sumAve/sumW8;
    sumW8 = TMath::Sqrt(sumW8);
    error = 1./sumW8;
  } else {
    av = -99.;
    error = -99.;
  }
  cout << "sqrt(sumW8) = " << sumW8 << " av = " << av << " +/- " << error << endl;

  chi2 = 0;
  for (unsigned int i = 0; i < val.size(); ++i) {
    e = verr[i];
    if (e > 0.) {
      w8 = 1./(e*e);
    } else {
      w8 = 0.;
    }
    chi2 += w8*(av-val[i])*(av-val[i]);
  }
}


// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {
  
  // -- command line arguments
  int verbose(0), nevts(2);
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i], "-v"))   {verbose = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-n"))   {nevts = atoi(argv[++i]);}
  }
  
  string gt("mcideal");
  cdbAbs *pDB = new cdbJSON(gt, "json", verbose);
  
  Mu3eConditions *pDC = Mu3eConditions::instance(gt, pDB);
  pDC->setVerbosity(verbose);
  
  vector<double> vdura, vduraE;
  vdura.reserve(nevts);
  vduraE.reserve(nevts);

  long long totalTime(0);
  int NCHIPS(4), NNOISY(150), NRECCHIPS(NCHIPS), NRECHITS(200);
  TH1D *hTime = new TH1D("hTime", "hTime", 100, 0., 10000.);
  for (int ievt = 0; ievt < nevts; ++ievt) {
    auto tbegin = std::chrono::high_resolution_clock::now();
    cout << "####### evt " << ievt << endl;
    calPixelQuality a;
    // -- create payload
    string hash("tag_pixelquality_mcideal_iov_1");
    createPayload(hash, &a, NCHIPS, NNOISY);
    
    a.readPayloadFromFile(hash, ".");
    a.calculate(hash);
    
    map<unsigned int, vector<pixhit>> detHits; 
    createRandomHits(detHits, NRECCHIPS, NRECHITS);

    // -- now loop over all rec hits
    for (auto it: detHits) {
      vector<pixhit> v = it.second; 
      cout << "it.first = " << it.first << " npix = " << v.size() << endl;
      for (auto ipix: v) {
        if (a.getStatus(ipix.ichip, ipix.icol, ipix.irow)) {
          if (0) cout << "skip noisy pixel " << ipix.ichip << "/" << ipix.icol << "/" << ipix.irow << endl;
        }
      }
    }
    auto tend = std::chrono::high_resolution_clock::now();
    long long  dus = chrono::duration_cast<chrono::microseconds>(tend-tbegin).count();
    auto dura = chrono::duration_cast<chrono::microseconds>(tend-tbegin).count();
    totalTime += dus;
    vdura.push_back(static_cast<double>(dus));
    vduraE.push_back(50.);
    hTime->Fill(static_cast<double>(dus));
    cout << "##timing: " << dura << " dus = " << dus << endl;
  }
  double aveVal(0.), aveErr(0.), chi2(0.);
  average(aveVal, aveErr, vdura, vduraE, chi2);
  cout << "##timing/evt: " << totalTime/nevts
       << " average = " << aveVal << " +/- " << aveErr
       << " TH1D = " << hTime->GetMean() << " +/- " << hTime->GetMeanError() << " (RMS = " << hTime->GetRMS() << ")"
       << " # NCHIPS/NNOISY/NRECHITS = "
       << NCHIPS << "/" << NNOISY << "/" << NRECHITS
       << endl;

  TCanvas c1;
  hTime->Draw();
  c1.SaveAs("hTime.pdf");
}

// ----------------------------------------------------------------------
void writeBlob(string filename, int nchip, int nnoisy) {
  long unsigned int header(0xdeadface);
  ofstream ONS;
  ONS.open(filename);
  if (1) {
    ONS << dumpArray(uint2Blob(header));
  }
  
  char data[8], data1[8], data2[8]; 
  for (unsigned int i = 0; i < nchip; ++i) {
    int nnoisypix(nnoisy);
    if (0) cout << "ichip = " << i
                << " nchip = " << nchip
                << " nnoisy = " << nnoisy
                << " nnoisypix = " << nnoisypix
                << endl;
    ONS << dumpArray(uint2Blob(i)) 
        << dumpArray(int2Blob(nnoisypix));
    for (unsigned int ipix = 0; ipix < nnoisypix; ++ipix) {
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
  writeBlob(tmpfilename, nchips, nnoisy);

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
  
  pixhit a;
  for (unsigned int ic = 0; ic < nrecchips; ++ic) {
    vector<pixhit> v; 
    for (unsigned int ih = 0; ih < nrechits; ++ih) {
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
