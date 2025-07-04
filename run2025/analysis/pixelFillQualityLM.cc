#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <string.h>
#include <chrono>
#include <algorithm> // for std::lower_bound

#include <fstream>

#include "cdbUtil.hh"
#include "Mu3eConditions.hh"
#include "calPixelAlignment.hh"
#include "calPixelQuality.hh"
#include "calPixelQualityV.hh"
#include "calPixelQualityM.hh"
#include "calPixelQualityLM.hh"


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
// Notes: 
// (1) Dead/problematic links are determined based on average (per-link) hits
// (2) Dead/problematic columns are determined based on average (per-column) hits 
// (3) Noisy pixels are determined on hit counts being larger than 10*sigma of the average (per-pixel) hits. 
// (4) If more than 40 of the pixels in a column are noisy, the entire column is flagged as suspect.
// (5) Since columns do not have a quality flag, both dead and suspect columns simply appear as "bad" columns
// 
// Examples:
// ---------
// cd mu3eanca/run2025/analysis
// bin/pixelFillQualityLM \
// -j /Users/ursl/data/mu3e/cdb/ -g datav6.2=2025Beam \
// -f /Users/ursl/mu3e/software/250429/minalyzer/root_output_files/dqm_histos_00553.root \
// -o /Users/ursl/mu3e/software/250429/minalyzer/root_output_files/dqm_histos_00553.json
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
bool determineBrokenLinks(TH2 *h, vector<int> &links);
void determineDeadColumns(TH2 *h, vector<int> &colums, vector<int> &links);
void determineNoisyPixels(TH2 *h, vector<int> &pixels, vector<int> &colums, vector<int> &links); // icol,irow,iqual


//string DATADIR("/Users/ursl/data/mu3e/run2025");
string DATADIR("/data/experiment/mu3e/data/2025/raw");

// ----------------------------------------------------------------------
string getDataSubdir(int runnumber) {
  int block = runnumber / 1000;
  string blockdir = Form("%03d", block);
  return Form("%s/%s", DATADIR.c_str(), blockdir.c_str());
}

// ----------------------------------------------------------------------
// -- VALID chipIDs for layer 1 and layer 2
// -- without this, the payloads will have too many chips 
// ----------------------------------------------------------------------
vector<int> gChipIDs = {1,2,3,4,5,6,
          33, 34, 35, 36, 37, 38,
          65, 66, 67, 68, 69, 70,
          97, 98, 99, 100, 101, 102,
          129, 130, 131, 132, 133, 134,
          161, 162, 163, 164, 165, 166,
          193, 194, 195, 196, 197, 198,
          225, 226, 227, 228, 229, 230,
          // -- layer 2
          1025, 1026, 1027, 1028, 1029, 1030,
          1057, 1058, 1059, 1060, 1061, 1062,
          1089, 1090, 1091, 1092, 1093, 1094,
          1121, 1122, 1123, 1124, 1125, 1126,
          1153, 1154, 1155, 1156, 1157, 1158,
          1185, 1186, 1187, 1188, 1189, 1190,
          1217, 1218, 1219, 1220, 1221, 1222,
          1249, 1250, 1251, 1252, 1253, 1254,
          1281, 1282, 1283, 1284, 1285, 1286,
          1313, 1314, 1315, 1316, 1317, 1318
          };


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
  int verbose(0), mode(1), printMode(0), check(0);
  // note: mode = 1 PixelQuality, 2 PixelQualityV, 3 PixelQualityM
  string jsondir(JSONDIR), filename("nada.root");
  string gt("mcidealv6.1");
  string odbfilename("nada.json");
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-c"))      {check = 1;}
    if (!strcmp(argv[i], "-f"))      {filename = argv[++i];}
    if (!strcmp(argv[i], "-g"))      {gt = argv[++i];}
    if (!strcmp(argv[i], "-j"))      {jsondir = argv[++i];}
    if (!strcmp(argv[i], "-m"))      {mode = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-o"))      {odbfilename = argv[++i];}
    if (!strcmp(argv[i], "-p"))      {printMode = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-v"))      {verbose = atoi(argv[++i]);}
  }
  
  if (check) {
    cout << "check mode" << endl;
    // --
    ifstream INS(filename);
    if (INS.fail()) {
      cout << "Error failed to open ->" << filename << "<-" << endl;
      return 1;
    }
    
    std::stringstream buffer;
    buffer << INS.rdbuf();
    INS.close();
    
    string lBuffer = buffer.str();

    // -- get the first one   
    string bla = jsonGetValue(lBuffer, {"Equipment", "PixelsCentral", "Settings", "CONFDACS", "ckdivend2"});
    cout << "bla = " << bla << endl;

    return 0;
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
    int station(0), layer(0), phi(0), z(0);

    // -- create turned on CSV for all chipIDs
    ofstream ofs;
    ofs.open(Form("csv/deadlinks-allpixels.csv"));
    ofs << "#chipID,ckdivend,ckdivend2,linkA,linkB,linkC,linkM,ncol[,icol],npix[,icol,irow,qual] NB: 0 = good, 1 = noisy, 2 = suspect, 3 = declared bad, 9 = turned off" << endl;
    
    cout << "print all chipIDs" << endl;
    calAbs* cal = pDC->getCalibration("pixelalignment_");
    calPixelAlignment* cpa = dynamic_cast<calPixelAlignment*>(cal);
    uint32_t i = 0;
    cpa->resetIterator();
    while(cpa->getNextID(i)) {
      uint32_t chipID = cpa->id(i);
      chipIDSpecBook(chipID, station, layer, phi, z);
      if (layer < 3) {
        cout << "chipID = " << cpa->id(i) << " station/layer/phi/z = " << station << "/" << layer << "/" << phi << "/" << z << endl;
        ofs << cpa->id(i) << "," << 31 << "," << 0 << "," << 0 << "," << 0 << "," << 0 << "," << 0 << "," << 0 << "," << 0 << endl;
      }
    }  
    ofs.close();

    string hash = string("tag_pixelqualitylm_") + gt + string("_iov_1");
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
  
  // -------------------------
  // -- ckdivend2 and ckdivend
  // -------------------------
  int ckdivendDefault(0), ckdivend2Default(31);
  if (run < 3098) {
    ckdivend2Default = 15;
  } else {
    ckdivend2Default = 31;
  }

  int ckdivend(0), ckdivend2(0);

  // -- try to assemble ODB filename. The logic here is flawed, but this is not relevant atm
  string dataSubdir = getDataSubdir(run);
  cout << "dataSubdir = " << dataSubdir << endl;
  if (odbfilename == "nada.json") {
    odbfilename = Form("%s/run%05d.odb", dataSubdir.c_str(), run);
    cout << "trying to open odfilename = " << odbfilename << endl;
  }
  if (odbfilename != "nada.json") {
    ifstream INS(odbfilename.c_str());
    if (INS.fail()) {
      cout << "Error failed to open ->" << odbfilename << "<-" << endl;
      ckdivend2 = ckdivend2Default;
      ckdivend = ckdivendDefault;
      cout << "using default ckdivend2 = " << ckdivend2 << " and ckdivend = " << ckdivend << endl;
    } else {
      cout << "reading ckdivend2 and ckdivend from ->" << odbfilename << "<-" << endl;
      std::stringstream buffer;
      buffer << INS.rdbuf();
      INS.close();
      string lBuffer = buffer.str();

      ckdivend2 = ::stoi(jsonGetValue(lBuffer, {"Equipment", "PixelsCentral", "Settings", "CONFDACS", "ckdivend2/key", "ckdivend2"}));
      ckdivend = ::stoi(jsonGetValue(lBuffer, {"Equipment", "PixelsCentral", "Settings", "CONFDACS", "ckdivend/key", "ckdivend"}));
      cout << "XXXXX ckdivend2 = " << ckdivend2 << " and ckdivend = " << ckdivend << endl;
    }
  } else {
    ckdivend2 = ckdivend2Default;
    ckdivend = ckdivendDefault;
    cout << "XXXXX using default ckdivend2 = " << ckdivend2 << " and ckdivend = " << ckdivend << endl;
  }

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
              if (0 == chipID) {
                cout << "  XXXXXXXXX  skipping illegal chipID = " << chipID << " from name = " << name << " is 0, skipping" << endl;
                continue;
              }
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
  vector<int> deadlinks, deadcolumns, noisyPixels;
  ofstream ofs;
  ofs.open(Form("csv/pixelqualitylm-run%d.csv", run));
  ofs << "#chipID,ckdivend,ckdivend2,linkA,linkB,linkC,linkM,ncol[,icol,iqual],npix[,icol,irow,iqual] NB: 0 = good, 1 = noisy, 2 = suspect, 3 = declared bad, 9 = turned off" << endl;
  for (auto it: mHitmaps){
    // -- debug with first 12 only FIXME
    //if (it.first != 1315) continue;
    // -- check if chipID is in the list of valid chipIDs
    if (find(gChipIDs.begin(), gChipIDs.end(), it.first) == gChipIDs.end()) {
      cout << "chipID = " << it.first << " is not in the list of valid chipIDs" << endl;
      continue;
    }
    // -- clear all vectors for this new chipID
    deadlinks.clear();
    deadcolumns.clear();
    noisyPixels.clear();
    // -- determine broken links
    bool turnedOn = determineBrokenLinks(mHitmaps[it.first], deadlinks);
    cout << "chipID = " << it.first << " turnedOn = " << turnedOn << endl;
    if (deadlinks.size() > 0) {
      cout << "chipID = " << it.first << " with link quality: ";
      ofs << it.first << ",";
      // -- ckdivend and ckdivend2
      ofs << ckdivend2 << "," << ckdivend << ",";
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
   
    // -- determine noisy pixels
    determineNoisyPixels(mHitmaps[it.first], noisyPixels, deadcolumns, deadlinks);

    // -- write out dead columns only after potential correction in noisy pixel determination
    if (deadcolumns.size() > 0) {
      cout << "chipID = " << it.first << " has " << deadcolumns.size()/2 << " dead/bad columns: ";
      ofs << deadcolumns.size()/2 << ",";
      for (auto itC = deadcolumns.begin(); itC != deadcolumns.end(); ++itC) {
        cout << *itC << " ";
        ofs << *itC << ",";
      }
      cout << endl;
    } else {
      cout << "chipID = " << it.first << " has no dead columns" << endl;
      ofs << 0 << ",";
    }
   
    if (noisyPixels.size() > 0) {
      cout << "chipID = " << it.first << " has " << noisyPixels.size()/3 << " noisy pixels: ";
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
    //break;
  }
  ofs.close();

  cout << "READING CSV: " << Form("csv/pixelqualitylm-run%d.csv", run) << endl;
  calPixelQualityLM *cpq = new calPixelQualityLM();
  cpq->readCsv(Form("csv/pixelqualitylm-run%d.csv", run));
  createPayload(hash, cpq, "./payloads");

  if (0) {
    cout << "READ CSV: " << Form("csv/pixelqualitylm-run%d.csv", run) << endl;
    cout << "WRITING CSV for validation: " << Form("csv/validatepixelqualitylm-run%d.csv", run) << endl;
    cpq->writeCsv(Form("csv/validatepixelqualitylm-run%d.csv", run));
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
bool determineBrokenLinks(TH2 *h, vector<int> &links) {
  bool DBX(false);
  double fractionHits(0.1);
  // -- remove 5 pixels from one edge to avoid edge noise in the A/C links which would make B "suspect"
  double edge(8.);
  double nLinkA = h->Integral(1+edge, 88, 1, 250);
  double nLinkB = h->Integral(89+edge, 172, 1, 250);
  double nLinkC = h->Integral(173, 256-edge, 1, 250);
  double nLinkAverage = (nLinkA + nLinkB + nLinkC)/3.;
  double cutMinHits = (nLinkAverage < 10? 1: fractionHits*nLinkAverage);

  string schip = h->GetName();
  replaceAll(schip, "hitmap_perChip_", "");
  int chipID = ::stoi(schip);

  // -- hand-curated list to avoid getting lost in algorithmic nightmares
  vector<int> vBadChips = {102};

  if (find(vBadChips.begin(), vBadChips.end(), chipID) != vBadChips.end()) {  
    links.push_back(3);
    links.push_back(3);
    links.push_back(3);
    links.push_back(3);
    return true;
  }

  if (DBX)
    cout << "histogram " << h->GetName() << " nLinkA = " << nLinkA
         << " nLinkB = " << nLinkB << " nLinkC = " << nLinkC
         << " nLinkAverage = " << nLinkAverage << " cutMinHits = " << cutMinHits
         << endl;

  if (nLinkA < 1 && nLinkB < 1 && nLinkC < 1) {
    links.push_back(9);
    links.push_back(9);
    links.push_back(9);
    links.push_back(9);
    return false;
  }

  if (nLinkA < cutMinHits) {
    if (nLinkA < 1) {
      links.push_back(9);
    } else {
      links.push_back(2);
    }
  } else {
    links.push_back(0);
  }
  if (nLinkB < cutMinHits) {
    if (nLinkB < 1) {
      links.push_back(9);
    } else {
      links.push_back(2);
    }
  } else {
    links.push_back(0);
  }
  if (nLinkC < cutMinHits) {
    if (nLinkC < 1) {
      links.push_back(9);
    } else {
      links.push_back(2);
    }
  } else {
    links.push_back(0);
  }
  // -- add dummy MUX link
  links.push_back(0);

  return true;
}

// ----------------------------------------------------------------------
void determineDeadColumns(TH2 *h, vector<int> &columns, vector<int> &links) {
  bool DBX(false);
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
      columns.push_back(ix-1);
      columns.push_back(9); 
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
void determineNoisyPixels(TH2 *h, vector<int> &noisyPixels, vector<int> &columns, vector<int> &links) {
  bool DBX(false);
  bool DODEADPIX(false);

  int chipCnt = h->GetSumOfWeights();
  int npix(0);
  for (int ix = 1; ix <= h->GetNbinsX(); ++ix) {
    for (int iy = 1; iy <= h->GetNbinsY(); ++iy) {
      if (h->GetBinContent(ix,iy) > 0) ++npix;
    }
  }
  if (DBX) cout << "chipID = " << h->GetName() << " chipCnt =  " << chipCnt << " for npix = " << npix << endl;
  if (npix > 0) {
    // -- noise threshold to be 10 x sigma above the mean hit number per pixel
    double NSIGMA(10.0);
    // -- if more than SUSPECTFRACTION of the pixel in a column are noisy, flag the entire column as suspect
    double SUSPECTMAXIMUM(40);
    double meanHits  = static_cast<double>(chipCnt)/npix;
    // -- this is WRONG and biased by noisy pixels
    double meanHitsE = TMath::Sqrt(meanHits);
    double noiseThr  = meanHits + NSIGMA*meanHitsE;
    double noiseThr2  = meanHits - NSIGMA*meanHitsE;
    if (noiseThr2 < 0.) noiseThr2 = 1.;
    if (meanHits > 0.) {
      if (DBX) cout << "meanHits = " << meanHits << " +/- " << meanHitsE << " noise threshold = " << noiseThr << endl;
    }
    int nNoisyPix(0);
    int nDeadPix(0);
    int colNoisyPixels(0);
    for (int ix = 1; ix <= h->GetNbinsX(); ++ix) {
      colNoisyPixels = 0;
      for (int iy = 1; iy <= h->GetNbinsY(); ++iy) {

        double nhits = h->GetBinContent(ix,iy);
        if (nhits > noiseThr) {
          ++nNoisyPix;
          ++colNoisyPixels;
          noisyPixels.push_back(ix-1);
          noisyPixels.push_back(iy-1);
          noisyPixels.push_back(1);  
          if (DBX) {
            cout << "  noisy pixel at icol/irow = " << ix-1 << "/" << iy-1  << " nhits = " << nhits << endl;
          }
        }
        if (DODEADPIX && (nhits < noiseThr2)) {
          // -- check if pixel is in dead column
          if (find(columns.begin(), columns.end(), ix-1) != columns.end()) {
            continue;
          }
          // -- check if pixel is in dead link
          if (links[0] > 0 && ix < 89) {
            continue;
          }
          if (links[1] > 0 && (ix >= 89 && ix < 173)) { 
            continue;
          }
          if (links[2] > 0 && ix >= 173) {
            continue;
          }
          ++nDeadPix;
          noisyPixels.push_back(ix-1);
          noisyPixels.push_back(iy-1);
          noisyPixels.push_back(9);  
          if (DBX) {
            cout << "  dead pixel at icol/irow = " << ix-1 << "/" << iy-1  << " nhits = " << nhits << endl;
          }
        }
      }
      if (DBX) cout << "col = " << ix-1 << "  colNoisyPixels = " << colNoisyPixels << " SUSPECTMAXIMUM = " << SUSPECTMAXIMUM << endl;
      if (colNoisyPixels > SUSPECTMAXIMUM) {
        for (int i = 0; i < colNoisyPixels; ++i) {
          // -- remove the noisy pixel col,row,iqual
          noisyPixels.pop_back(); 
          noisyPixels.pop_back();
          noisyPixels.pop_back();
        }

        // -- possibly out of order!
        columns.push_back(ix-1);
        columns.push_back(3);

        if (DBX) {
          cout << "  -> colNoisyPixels > SUSPECTMAXIMUM -> removing " << colNoisyPixels << " noisy pixels, deadcolumn: " << endl;
          for (auto it00: columns) {
            cout << it00 << " ";
          }
          cout << endl;
        }
      }

    }
    if (DBX) cout << "  -> nNoisyPixel = " << nNoisyPix << " nDeadPix = " << nDeadPix << endl;
  }
}


