#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <string.h>
#include <chrono>

#include "Mu3eConditions.hh"
#include "calPixelAlignment.hh"
#include "calPixelQuality.hh"
#include "calPixelQualityV.hh"
#include "calPixelQualityM.hh"
#include "calPixelQualityLM.hh"

#include "util.hh"

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
// pixelFillQualityLM
// ------------------
// Determine (placeholder/offline) DQM information for pixelQualityLM
// 
// Examples:
// ---------
// cd mu3eanca/run2025/analysis
// bin/pixelFillQualityLM -j /Users/ursl/data/mu3e/cdb/ -g datav6.1=2025CosmicsVtxOnly -f /Users/ursl/mu3e/software/250429/minalyzer/root_output_files/dqm_histos_00553.root
// ----------------------------------------------------------------------

#define JSONDIR "/Users/ursl/data/mu3e/cdb"

// ----------------------------------------------------------------------
struct pixhit {
  unsigned int ichip;
  int icol, irow;
  int tot;
  int status;
};

// ----------------------------------------------------------------------
void createPayload(string, calAbs *, string);
void chipIDSpecBook(int chipid, int &station, int &layer, int &phi, int &z);
void determineBrokenLinks(TH2 *h, vector<int> &links, bool turnedOn);
void determineDeadColumns(TH2 *h, vector<int> &colums, vector<int> &links);
void determineNoisyPixels(TH2 *h, vector<int> &pixels); // icol,irow,iqual


// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {

  // -- filter on US only?
  bool filterUS(true);

  // -- chipIDs that are in fact turned on (complete vertex)
  vector<pair<int, int> > vLayLdrTurnedOn = {
    {1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}, {1, 7}, {1, 8},
    {2, 1}, {2, 2}, {2, 3}, {2, 4}, {2, 5}, {2, 6}, {2, 7}, {2, 8}, {2, 9}, {2, 10}
  };
 


  // -- command line arguments
  int verbose(0), mode(1), printMode(0);
  // note: mode = 1 PixelQuality, 2 PixelQualityV, 3 PixelQualityM
  string jsondir(JSONDIR), filename("nada.root");
  string gt("mcidealv6.1");
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-f"))      {filename = argv[++i];}
    if (!strcmp(argv[i], "-g"))      {gt = argv[++i];}
    if (!strcmp(argv[i], "-j"))      {jsondir = argv[++i];}
    if (!strcmp(argv[i], "-m"))      {mode = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-p"))      {printMode = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-v"))      {verbose = atoi(argv[++i]);}
  }
  
  // -- this is just to get the list of all chipIDs
  cdbAbs *pDB = new cdbJSON(gt, jsondir, verbose);
  Mu3eConditions* pDC = Mu3eConditions::instance("mcidealv6.1", pDB);
  pDC->setRunNumber(1);
  if(!pDC->getDB()) {
      std::cout << "CDB database not found" << std::endl;
  }

  if (1 <= printMode && printMode <= 3) {
    cout << "print chipIDs printMode = " << printMode << endl;
    int station(0), layer(0), phi(0), z(0);
    calAbs* cal = pDC->getCalibration("pixelalignment_");
    calPixelAlignment* cpa = dynamic_cast<calPixelAlignment*>(cal);
    uint32_t i = 0;
    cpa->resetIterator();
    while(cpa->getNextID(i)) {
      uint32_t chipID = cpa->id(i);
      chipIDSpecBook(chipID, station, layer, phi, z);
      if (printMode == 1 && (layer < 3)) {
        // -- print all chipIDs
        cout << "chipID = " << cpa->id(i) << endl;
      } else if (printMode == 2 && (layer < 3) && (z < 4)) {
        // -- print US chipIDs
        cout << "chipID = " << cpa->id(i) <<  endl;
      } else if (printMode == 3 && (layer< 3) && (z > 3)) {
        // -- print DS chipIDs
        cout << "chipID = " << cpa->id(i) << endl;

      }

    }
  }
  if (4 == printMode) {
    // -- create turned on CSV for all chipIDs
    ofstream ofs;
    ofs.open(Form("csv/deadlinks-allpixels.csv"));
    ofs << "#chipID,linkA,linkB,linkC,linkM,ncol[,icol] NB: linkX: 0 = no error, 1 = dead" << endl;
    
    cout << "print all chipIDs" << endl;
    int station(0), layer(0), phi(0), z(0);
    calAbs* cal = pDC->getCalibration("pixelalignment_");
    calPixelAlignment* cpa = dynamic_cast<calPixelAlignment*>(cal);
    uint32_t i = 0;
    cpa->resetIterator();
    while(cpa->getNextID(i)) {
      ofs << cpa->id(i) << ",0,0,0,0,0" << endl;
    }  
    ofs.close();

    string hash = string("tag_pixelqualitylm_") + gt + string("_iov_0");
    calPixelQualityLM *cpq = new calPixelQualityLM();
    cpq->readCsv(Form("csv/deadlinks-allpixels.csv"));
    string blob = cpq->makeBLOB();
    createPayload(hash, cpq, "./payloads");
  
    return 0; 
  }


  vector<uint32_t> allSensorIDs;
  allSensorIDs.reserve(3000);
  calAbs* cal = pDC->getCalibration("pixelalignment_");
  calPixelAlignment* cpa = dynamic_cast<calPixelAlignment*>(cal);
  uint32_t i = 0;
  cpa->resetIterator();
  while(cpa->getNextID(i)) {
      allSensorIDs.push_back(cpa->id(i));
  }
  
  cout << "fill mChipIDOK from allSensorIDs.size() = " << allSensorIDs.size() << endl;
  map <unsigned int, tuple<int, int, int> > mChipIDOK;
  for (auto chipID : allSensorIDs) {
    bool found(false);
    int station(0), layer(0), phi(0), z(0);
    if (1) {
      // -- check if chipID is in the list of turned on chips
      for (auto itL : vLayLdrTurnedOn) {
        chipIDSpecBook(chipID, station, layer, phi, z);

        if (itL.first == layer && itL.second == phi && (filterUS ? (z < 4) : true)) {
          // cout << " FOUND chipID = " << chipID << " in layer/ladder = " << layer << "/" << phi << endl;
          found = true;
          break;
        }
      }
      if (found) {
        mChipIDOK.insert({chipID, {layer, phi, z}});
      }
    }

    if (0) {
      // -- place all VTX chipIDs in the map
      chipIDSpecBook(chipID, station, layer, phi, z);
      if (layer < 3) {
        mChipIDOK.insert({chipID, {layer, phi, z}});
      }
    }
  }

  int run(-1);
  string sbla(filename);
  cout << "filename ->" << filename << "<-" << endl;
  if (string::npos != filename.find_last_of("/")) {
    string dir = filename.substr(0, filename.find_last_of("/")+1);
    cout << "dir ->" << dir << "<-" << endl;
    replaceAll(sbla, dir, "");
  }
  // -- extract the runnumber. It directly precedes .root
  replaceAll(sbla, ".root", "");
  size_t rIdx = sbla.rfind("_")+1;
  sbla = sbla.substr(rIdx);
  cout << "sbla ->" << sbla << "<-" << endl;
  run = ::stoi(sbla);
  cout << "run = " << run << endl;
  

  string hash = string("tag_pixelqualitylm_") + gt + string("_iov_") + to_string(run);
  
  TFile *f = TFile::Open(filename.c_str());
  
  // -- read in ALL chipids in VTX
  vector<unsigned int> vchipid;
  vector<string> vStations = {"station_0"};
  map<string, int> vLayers  = {{"layer_1", 8}, {"layer_2", 10}};
  map<int, TH2*> mHitmaps;

  // -- read in all hitmaps
  for (auto itS : vStations) {
    for (auto itL : vLayers) {
      for (int iLdr = 1; iLdr <= itL.second; ++iLdr) {
        stringstream ss;
        ss << "pixel/hitmaps/" << itS << "/" << itL.first
           << Form("/ladder_%02d", iLdr);
        string s = ss.str();
        cout << "TDirectory " << s << "<-" << endl;
        f->cd(s.c_str());
        TIter next(gDirectory->GetListOfKeys());
        TKey *key(0);
        while ((key = (TKey *)next())) {
          if (key) {
            cout << "found key " << s << endl;
            if (gROOT->GetClass(key->GetClassName())->InheritsFrom("TDirectory")) continue;
            if (!gROOT->GetClass(key->GetClassName())->InheritsFrom("TH1")) continue;       
            string name = key->GetName();
            if (string::npos != name.find("hitmap_perChip")) {
              unsigned int chipID;
              replaceAll(name, "hitmap_perChip_", "");
              chipID = ::stoi(name); 
              cout << "   .. chipID = " << chipID << " from name = " << name << endl;
              vchipid.push_back(chipID);
              TH2F *h = (TH2F*)key->ReadObj();
              mHitmaps.insert(make_pair(chipID, h));
            }
          }
        }
      }
    }
  }

  cout << "mHitmaps.size() = " << mHitmaps.size() << endl;

  // -- do analysis
  TCanvas *c0 = new TCanvas("c0", "c0", 800, 600);
  c0->Divide(3,5);
  int ipad(1);
  gStyle->SetOptStat(0);
  vector<int> deadlinks, deadcolumns, noisyPixels;
  ofstream ofs;
  ofs.open(Form("csv/pixelqualitylm-run%d.csv", run));
  ofs << "#chipID,linkA,linkB,linkC,linkM,ncol[,icol],npix[,icol,irow,qual] NB: 0=no error, 1=noisy, 2=dead, 9=chip off" << endl;
  for (auto it: mHitmaps){
    bool turnedOn = mChipIDOK.find(it.first) != mChipIDOK.end();
    if (turnedOn) {
      cout << "chipID = " << it.first << " is turned on" << endl;
    } else {
      cout << "chipID = " << it.first << " is NOT turned on" << endl;
    }
    // -- clear all vectors for this new chipID
    deadlinks.clear();
    deadcolumns.clear();
    noisyPixels.clear();
    // -- determine broken links
    determineBrokenLinks(mHitmaps[it.first], deadlinks, turnedOn);
    if (deadlinks.size() > 0) {
      cout << "chipID = " << it.first << " has broken links: ";
      ofs << it.first << ",";
      for (auto itL : deadlinks) {
        ofs << itL << ",";
        cout << itL << " ";
      }
      cout << endl;
    } else {
      cout << "chipID = " << it.first << " has no broken links" << endl;
    }
    // -- determine dead columns
    determineDeadColumns(mHitmaps[it.first], deadcolumns, deadlinks);
    if (deadcolumns.size() > 0) {
      cout << "chipID = " << it.first << " has dead columns: ";
      ofs << deadcolumns.size() << ",";
      for (auto itC = deadcolumns.begin(); itC != deadcolumns.end(); ++itC) {
        cout << *itC << " ";
        ofs << *itC << ",";
      }
      cout << endl;
    } else {
      cout << "chipID = " << it.first << " has no dead columns" << endl;
      ofs << 0 << ",";
    }
    // -- determine noisy pixels
    determineNoisyPixels(mHitmaps[it.first], noisyPixels);
    if (noisyPixels.size() > 0) {
      cout << "chipID = " << it.first << " has noisy pixels: ";
      ofs << noisyPixels.size()/3 << ",";
      for (auto itN = noisyPixels.begin(); itN != noisyPixels.end(); ++itN) {
        cout << *itN << " ";
        ofs << *itN;
        if (itN != noisyPixels.end() - 1) ofs << ",";
      }
      cout << endl;
    } else {
      cout << "chipID = " << it.first << " has no noisy pixels" << endl;
      ofs << 0;
    }
    
    ofs << endl;
    if (turnedOn) {
      c0->cd(ipad++);
      shrinkPad(0.1, 0.17);
      mHitmaps[it.first]->Draw("colz");
      mHitmaps[it.first]->SetTitle(Form("Run %d, LAY/LDR/CHP %d/%d/%d (%d)",  run, 
        get<0>(mChipIDOK[it.first]), get<1>(mChipIDOK[it.first]), get<2>(mChipIDOK[it.first]), it.first));
      mHitmaps[it.first]->GetXaxis()->SetTitle("col");
      mHitmaps[it.first]->GetYaxis()->SetTitle("row");
    }
  }
  c0->SaveAs(Form("out/run%d_hitmaps.pdf", run));
  ofs.close();

  calPixelQualityLM *cpq = new calPixelQualityLM();
  cpq->readCsv(Form("csv/pixelqualitylm-run%d.csv", run));
  string blob = cpq->makeBLOB();
  createPayload(hash, cpq, "./payloads");

  cpq->writeCsv(Form("csv/validatepixelqualitylm-run%d.csv", run));

  for (auto it: mHitmaps) {
    // TODO: check that non-zero entries and that it.first is not in mChipIDOK
  }

  cout << "This is the end, my friend" << endl;
  return 0;
}

// ----------------------------------------------------------------------
void createPayload(string hash, calAbs *a, string jsondir) {

  string sblob = a->makeBLOB();
  
  payload pl;
  pl.fHash = hash;
  pl.fComment = "pixelqualitylm";
  pl.fBLOB = sblob;
  cout << "######################################################################" << endl;
  cout << "### createPayload" << endl;
  a->printBLOB(sblob);
  
  a->writePayloadToFile(hash, jsondir, pl);
}

// ----------------------------------------------------------------------
void chipIDSpecBook(int chipid, int &station, int &layer, int &phi, int &z) {
  station = chipid/(0x1<<12);
  layer   = chipid/(0x1<<10) % 4 + 1;
  phi     = chipid/(0x1<<5) % (0x1<<5) + 1;
 
  int zp  = chipid % (0x1<<5);
 
  if (layer == 3) {
    z = zp - 7;
  } else if (layer == 4) {
    z = zp - 6;
  } else {
    z = zp;
  }

}

// ----------------------------------------------------------------------
void determineBrokenLinks(TH2 *h, vector<int> &links, bool turnedOn) {
  bool DBX(false);
  double fractionHits(0.1);
  double nLinkA = h->Integral(1, 88, 1, 250);
  double nLinkB = h->Integral(89, 172, 1, 250);
  double nLinkC = h->Integral(173, 256, 1, 250);
  double nLinkAverage = (nLinkA + nLinkB + nLinkC)/3.;
  double cutMinHits = (nLinkAverage < 10? 1: fractionHits*nLinkAverage);

  string schip = h->GetName();
  replaceAll(schip, "hitmap_perChip_", "");
  int chipID = ::stoi(schip);

  if (DBX)
    cout << "histogram " << h->GetName() << " nLinkA = " << nLinkA
         << " nLinkB = " << nLinkB << " nLinkC = " << nLinkC
         << " nLinkAverage = " << nLinkAverage << " cutMinHits = " << cutMinHits
         << endl;
  if (!turnedOn) {
    links.push_back(9);
    links.push_back(9);
    links.push_back(9);
    links.push_back(9);
  } else {
    
    if (nLinkA < cutMinHits) {
      links.push_back(2);
    } else {
      links.push_back(0);
    }
    if (nLinkB < cutMinHits) {
      links.push_back(2);
    } else {
      links.push_back(0);
    }
    if (nLinkC < cutMinHits) {
      links.push_back(2);
    } else {
      links.push_back(0);
    }
    // -- add dummy MUX link
    links.push_back(0);
  }
  
}

// ----------------------------------------------------------------------
void determineDeadColumns(TH2 *h, vector<int> &colums, vector<int> &links) {
  bool DBX(true);
  double minHits(1);
  double meanHits = h->Integral(1, 256, 1, 250)/h->GetNbinsX();
  if (meanHits < 10) {
    minHits = 0;
  } else {
    minHits = meanHits/10.;
    minHits = 1;
  }
  bool linkA = links[0] > 0;
  bool linkB = links[1] > 0;
  bool linkC = links[2] > 0;
  // -- loop over bin indices, not column indices
  for (int ix = 1; ix <= h->GetNbinsX(); ++ix) {
    double nHits = h->Integral(ix, ix, 1, 250);

    if (nHits < minHits) {
      // -- first check if already contained in dead links  
      if (linkA > 0 && ix < 89) {
        continue;
      } 
      if (linkB > 0 && (ix >= 89 && ix < 173)) {
       continue;
      }
      if (linkC > 0 && ix >= 173) {
        continue;
      }
      colums.push_back(ix-1);
      if (DBX) {
        cout << "determineDeadColumns> ix = " << ix << " pushed! nHits = " << nHits
             << " minHits = " << minHits << " deadlinks = ";
        for (auto itL : links) {
          cout << itL << " ";
        }
        cout << endl; 
      }

    }
  }

}

// ----------------------------------------------------------------------
void determineNoisyPixels(TH2 *h, vector<int> &noisyPixels) {
  bool DBX(true);

  int chipCnt = h->GetSumOfWeights();
  int npix(0);
  for (int ix = 1; ix <= h->GetNbinsX(); ++ix) {
    for (int iy = 1; iy <= h->GetNbinsY(); ++iy) {
      if (h->GetBinContent(ix,iy) > 0) ++npix;
    }
  }
  if (DBX) cout << "chipID = " << h->GetName() << " nhits =  " << chipCnt << " for npix = " << npix << endl;
  if (npix > 0) {
    double NSIGMA(10.0);
    double meanHits  = static_cast<double>(chipCnt)/npix;
    // -- this is WRONG!
    double meanHitsE = TMath::Sqrt(meanHits);
    double noiseThr  = meanHits + NSIGMA*meanHitsE;
    if (meanHits > 0.) {
      if (DBX) cout << "meanHits = " << meanHits << " +/- " << meanHitsE << " noise threshold = " << noiseThr << endl;
    }
    int nNoisyPix(0);
    for (int ix = 1; ix <= h->GetNbinsX(); ++ix) {
      for (int iy = 1; iy <= h->GetNbinsY(); ++iy) {
        float nhits = h->GetBinContent(ix,iy);
        if (nhits > noiseThr) {
          ++nNoisyPix;
          noisyPixels.push_back(ix-1);
          noisyPixels.push_back(iy-1);
          noisyPixels.push_back(1);  
          if (DBX) {
            cout << "  noisy pixel at icol/irow = " << ix-1 << "/" << iy-1  << " nhits = " << nhits << endl;
          }
        }
      }
    }
    if (DBX) cout << "  -> nNoisyPixel = " << nNoisyPix << endl;
  }
}


