#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <string.h>
#include <chrono>
#include <algorithm> // for std::lower_bound

#include <fstream>

//#include "cdbUtil.hh" 
#include "Mu3eConditions.hh"
#include "calPixelAlignment.hh"
#include "calPixelQualityLM.hh"


#include "gMapChipIDLinkOffsets.icc"

#include "cdbJSON.hh"
#include "base64.hh"

#include "TCanvas.h"
#include "TStyle.h"
#include "TFile.h"
#include "TTree.h"
#include "TH2F.h"
#include "TMath.h"
#include "TKey.h"
#include "TROOT.h"

#include "anaMidasMetaTree.hh"

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
// -g datav6.3=2025V0 -j ~/data/mu3e/cdb/ 
// -f ~/data/mu3e/run2025/mlzr/merged-dqm_histos_06252.root -r MidasMeta2025Data.root
//
// Create _iov_1 payload:
// moor>./bin/pixelFillQualityLM -g datav6.3=2025V0 -j ~/data/mu3e/cdb/ -p 4
// 
// ----------------------------------------------------------------------

#define JSONDIR "/Users/ursl/data/mu3e/cdb"
#define CSVSCHEMA "#chipID,ckdivend,ckdivend2,linkA,linkB,linkC,linkM,ncol[,icol,iqual],npix[,icol,irow,iqual]"
// ----------------------------------------------------------------------
struct pixhit {
  unsigned int ichip;
  int icol, irow;
  int tot;
  int status;
};

// ----------------------------------------------------------------------
void createPayload(string, calAbs *, string, string, string);
void chipIDSpecBook(int chipid, int &station, int &layer, int &phi, int &z);
bool determineBrokenLinks(TH2 *h, vector<int> &links);
bool determineBrokenLinksV0(TH2 *h, vector<int> &links);
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



map<int, AsicInfo> gRunInfoMap;
bool gIsBeam(true);

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
  string gt("datav6.3=2025V0");
  string igt("datav6.2=2025Beam");
  string rootMetaMidasFilename("nada.root");
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-c"))      {check = 1;}
    if (!strcmp(argv[i], "-f"))      {filename = argv[++i];}
    if (!strcmp(argv[i], "-g"))      {gt = argv[++i];}
    if (!strcmp(argv[i], "-i"))      {igt = argv[++i];}
    if (!strcmp(argv[i], "-j"))      {jsondir = argv[++i];}
    if (!strcmp(argv[i], "-m"))      {mode = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-p"))      {printMode = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-r"))      {rootMetaMidasFilename = argv[++i];}
    if (!strcmp(argv[i], "-v"))      {verbose = atoi(argv[++i]);}
  }
  
  cout << "printMode = " << printMode << endl;
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
    // string bla = jsonGetValue(lBuffer, {"Equipment", "PixelsCentral", "Settings", "CONFDACS", "ckdivend2"});
    //cout << "bla = " << bla << endl;

    return 0;
  }

  // -- this is just to get the list of all chipIDs
  cdbAbs *pDB = new cdbJSON(igt, jsondir, verbose);
  Mu3eConditions* pDC = Mu3eConditions::instance(igt, pDB);
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
    string filename = Form("csv/deadlinks-allpixels.csv");
    ofs.open(filename);
    ofs << CSVSCHEMA << " NB: " << calPixelQualityLM::getStatusDocumentation() << endl;
    
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
    string schema = cpq->getSchema();
    string comment = "runclass/meanY. " + filename;
    cout << "XXXXXXXXX createPayload with hash = " << hash << " comment = " << comment << " schema = " << schema << endl;
    createPayload(hash, cpq, "./payloads", schema, comment);
  
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
    (sbla, dir, "");
  }
  // -- extract the runnumber and runclass
  replaceAll(sbla, ".root", "");
  size_t rIdx = sbla.rfind("_")+1;
  sbla = sbla.substr(rIdx);
  cout << "sbla ->" << sbla << "<-" << endl;  
  run = ::stoi(sbla);
  pDC->setRunNumber(run);
  runRecord rr = pDC->getRunRecord(run);
  rr.print();
  string runclass = rr.fBORRunClass;
  int qVertex = rr.fvDQ[rr.fDataQualityIdx].vertex;
  cout << "run = " << run << " runclass = " << runclass << " qVertex = " << qVertex << endl;
  if (runclass.find("Cosmic") != string::npos) {
    gIsBeam = false;
  }
  if (runclass.find("Calibration") != string::npos) {
    gIsBeam = false;
  }
  if (runclass.find("Test") != string::npos) {
    gIsBeam = false;
  }
  cout << "gIsBeam = " << gIsBeam << endl;

  // ----------------------------------------------------------------------
  // -- read in the meta midas tree
  // ----------------------------------------------------------------------
  TFile *fMetaMidas = TFile::Open(rootMetaMidasFilename.c_str());
  TTree *tMetaMidas = (TTree*)fMetaMidas->Get("midasMetaTree");
  anaMidasMetaTree *ammt = new anaMidasMetaTree(tMetaMidas);

  cout << "loadRunInfo" << endl;
  gRunInfoMap = ammt->loadRunInfo(run);
  cout << "gRunInfoMap.size() = " << gRunInfoMap.size() << endl;
  for (auto it : gRunInfoMap) {
    cout << "run = " << it.second.confId
      << " globalChipID = " << it.first << " globalId = " << it.second.globalId 
      << " offsets = " << gMapChipIDLinkOffsets[it.first][0] << gMapChipIDLinkOffsets[it.first][1] << gMapChipIDLinkOffsets[it.first][2]
      << " linkMask = " << it.second.linkMask[0] << it.second.linkMask[1] << it.second.linkMask[2]
      << " linkMatrix = " << it.second.linkMatrix[0] << it.second.linkMatrix[1] << it.second.linkMatrix[2]
      << " abcLinkMask = " << it.second.abcLinkMask[0] << it.second.abcLinkMask[1] << it.second.abcLinkMask[2]
      << " abcLinkMatrix = " << it.second.abcLinkMatrix[0] << it.second.abcLinkMatrix[1] << it.second.abcLinkMatrix[2]
      << " abcLinkErrs = " << it.second.abcLinkErrs[0] << ","<< it.second.abcLinkErrs[1] << "," << it.second.abcLinkErrs[2]
      << " ckdivend = " << it.second.ckdivend << " ckdivend2 = " << it.second.ckdivend2
      << endl;
  }
  
  // -------------------------
  // -- ckdivend2 and ckdivend
  // -------------------------
  int ckdivendDefault(0), ckdivend2Default(31);
  if (run < 3098) {
    ckdivend2Default = 15;
  } else {
    ckdivend2Default = 31;
  }
  int ckdivend(gRunInfoMap[1].ckdivend), ckdivend2(gRunInfoMap[1].ckdivend2);

  string hash = string("tag_pixelqualitylm_") + gt + string("_iov_") + to_string(run);
  
  cout << "hallo" << endl;
  TFile *f = TFile::Open(filename.c_str());
  if (f) {
    if (!f->IsOpen()) {
      cout << "XXXXXXXXX pixelFillQualityLM failed to open file " << filename << endl;
      return 0;
    }
  } else {
    cout  << "XXXXXXXXX pixelFillQualityLM not found file " << filename << endl;
    return 0;
  }

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
        if (0) cout << "TDirectory " << s << "<-" << endl;
        f->cd(s.c_str());
        TIter next(gDirectory->GetListOfKeys());
        TKey *key(0);
        while ((key = (TKey *)next())) {
          if (key) {
            if (0) cout << "found key " << s << endl;
            if (gROOT->GetClass(key->GetClassName())->InheritsFrom("TDirectory")) continue;
            if (!gROOT->GetClass(key->GetClassName())->InheritsFrom("TH1")) continue;       
            string name = key->GetName();
            string oname = s + "/" + name;
            if (string::npos != name.find("hitmap_perChip")) {
              unsigned int chipID;
              replaceAll(name, "hitmap_perChip_", "");
              chipID = ::stoi(name); 
              if (1) cout << "   .. chipID = " << chipID << " from name = " << oname << endl;
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
  ofs << CSVSCHEMA << " NB: " << calPixelQualityLM::getStatusDocumentation() << endl;
  for (auto it: mHitmaps){
    // -- debug with first 12 only FIXME
    //if (it.first != 1315) continue;
    // -- check if chipID is in the list of valid chipIDs
    if (find(gChipIDs.begin(), gChipIDs.end(), it.first) == gChipIDs.end()) {
      cout << "XXXXXXXXXchipID = " << it.first << " is not in the list of valid chipIDs" << endl;
      continue;
    }
    // -- clear all vectors for this new chipID
    deadlinks.clear();
    deadcolumns.clear();
    noisyPixels.clear();
    // -- determine broken links
    cout << "mHitmaps[" << it.first << "] : " << mHitmaps[it.first]->GetName() << endl;
    bool turnedOn = determineBrokenLinks(mHitmaps[it.first], deadlinks);
    cout << "chipID = " << it.first << " turnedOn = " << turnedOn << endl;
    if (deadlinks.size() > 0) {
      cout << "chipID = " << it.first << " with link quality: ";
      ofs << it.first << ",";
      // -- ckdivend and ckdivend2
      ofs << ckdivend << "," << ckdivend2 << ",";
      for (auto itL : deadlinks) {
        ofs << itL << ",";
        cout << itL << " ";
      }
      cout << endl;
    } else {
      cout << "chipID = " << it.first << " has no broken links" << endl;
    }
    // -- determine dead columns
    if (gIsBeam) {
      determineDeadColumns(mHitmaps[it.first], deadcolumns, deadlinks);
    } else {
      deadcolumns.clear();
    }
   
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
  string schema = cpq->getSchema();
  string comment = "no E, only LVDS error rate. " + calPixelQualityLM::getStatusDocumentation();

  createPayload(hash, cpq, "./payloads", schema, comment);

  if (0) {
    cout << "READ CSV: " << Form("csv/pixelqualitylm-run%d.csv", run) << endl;
    cout << "WRITING CSV for validation: " << Form("csv/validatepixelqualitylm-run%d.csv", run) << endl;
    cpq->writeCsv(Form("csv/validatepixelqualitylm-run%d.csv", run));
  }
  cout << "This is the end, my friend" << endl;
  return 0;
}

// ----------------------------------------------------------------------
void createPayload(string hash, calAbs *a, string jsondir, string schema, string comment) {

  string sblob = a->makeBLOB();
  payload pl;
  pl.fHash = hash;
  pl.fComment = comment;
  pl.fSchema = schema;
  pl.fBLOB = sblob;
  cout << "######################################################################" << endl;
  cout << "### createPayload" << endl;
  cout <<  pl.printString(false) << endl;
  a->printBLOB(sblob);
  cout << "######################################################################" << endl;
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
// -- this is completely new and now relies on PCLS data read from midas meta data tree
bool determineBrokenLinks(TH2 *h, vector<int> &links) {
  bool DBX(false);

  // -- the histogram still used to provide the chip name
  string schip = h->GetName();
  replaceAll(schip, "hitmap_perChip_", "");
  int chipID = ::stoi(schip);

  // -- determine dead chips
  double nLinkA = h->Integral(1, 88, 1, 250);
  double nLinkB = h->Integral(89, 172, 1, 250);
  double nLinkC = h->Integral(173, 256, 1, 250);
  vector<double> vLink = {nLinkA, nLinkB, nLinkC};
  double nLinkAverage = (nLinkA + nLinkB + nLinkC)/3.;
  if (1) cout << "  nLinkA = " << nLinkA << " nLinkB = " << nLinkB << " nLinkC = " << nLinkC << " nLinkAverage = " << nLinkAverage << endl;
  if (nLinkA < 1 && nLinkB < 1 && nLinkC < 1) {
    links.push_back(7);
    links.push_back(7);
    links.push_back(7);
    links.push_back(7);
    return false;
  }

  // -- search for pathological cases where all hits are very close to the top (bottom) edge
  vector<double> vLinkMeanY;
  vLinkMeanY.push_back(h->ProjectionY("A", 1, 88)->GetMean());
  vLinkMeanY.push_back(h->ProjectionY("B", 89, 172)->GetMean());
  vLinkMeanY.push_back(h->ProjectionY("C", 173, 256)->GetMean());
  int badChip(-1); // -- if any link is broken and not masked, the chip is bad
  if (nLinkA > 0 && (vLinkMeanY[0] < 10 || vLinkMeanY[0] > 245)) badChip = 1;
  if (nLinkB > 0 && (vLinkMeanY[1] < 10 || vLinkMeanY[1] > 245)) badChip = 2;
  if (nLinkC > 0 && (vLinkMeanY[2] < 10 || vLinkMeanY[2] > 245)) badChip = 3;
  if (badChip > 0) {
    cout << "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZ badChip = " << badChip << " vLinkMeanY = " << vLinkMeanY[0] << " " << vLinkMeanY[1] << " " << vLinkMeanY[2] << endl;  
    if (nLinkA < 1) {
      links.push_back(8);
    } else {
      links.push_back(6);
    }
    if (nLinkB < 1) {
      links.push_back(8);
    } else {
      links.push_back(6);
    }
    if (nLinkC < 1) {
      links.push_back(8);
    } else {
      links.push_back(6);
    }
    links.push_back(0);
    return true;
  }


  // -- hand-curated list to avoid getting lost in algorithmic nightmares
  vector<int> vBadChips = {102};

  if (find(vBadChips.begin(), vBadChips.end(), chipID) != vBadChips.end()) {  
    links.push_back(3);
    links.push_back(3);
    links.push_back(3);
    links.push_back(3);
    return true;
  }

  AsicInfo ai = gRunInfoMap[chipID];

  // -- completely disabled chip
  if (0 == ai.abcLinkMask[0]
      && 0 == ai.abcLinkMask[1]
      && 0 == ai.abcLinkMask[2]) {
    links.push_back(9);
    links.push_back(9);
    links.push_back(9);
    links.push_back(0);
    return false;
  }

  
  // -- check individual links
  int lkStatus[4] = {0, 0, 0, 0};
  int nBadLinks(0);
  badChip = -1;
  bool badLink[3] = {false, false, false};
  for (int i = 0; i < 3; ++i) {
    if (vLink[i] < 1) lkStatus[i] = 8;
    if (0 == ai.abcLinkMask[i]) lkStatus[i] = 9;
    if (ai.abcLinkErrs[i] > 10) lkStatus[i] = 4;
    if (ai.abcLinkErrs[i] > 10 && 1 == ai.abcLinkMask[i]) {
      badChip = 1;
      ++nBadLinks;
      badLink[i] = true;
    }
  }
 
  for (int i = 0; i < 3; ++i) {
    cout << "Chip " << chipID << " link " << i << " status = " << lkStatus[i] 
    << " mask = " << ai.abcLinkMask[i] << " errs = " << ai.abcLinkErrs[i]
    << " badLink = " << badLink[i]
    << endl;
  }

  // -- Check for row overflow (indicating LVDS errors)
  int iOverFlow = h->Integral(1, 256, 251, 252);
  if (0) cout << "Chip " << chipID << " overflow bin: " << iOverFlow << endl;

  if (iOverFlow > 0) {
    double nhits = h->Integral(1, 256, 1, 250);
    uint32_t istatus = static_cast<uint32_t>(nhits/iOverFlow);
    if (iOverFlow > nhits) istatus = 1;
    if (nhits/iOverFlow > UINT32_MAX) istatus = UINT32_MAX;
    lkStatus[3] = istatus;
    cout << "Chip " << chipID << " overflow: " << iOverFlow << " nhits = " << nhits << " status = " << istatus 
         << " UINT32_MAX = " << UINT32_MAX << endl;
  } else {
    lkStatus[3] = 0;
    cout << "Chip " << chipID << " NO overflow: " << iOverFlow << " status = " << lkStatus[3] <<  endl;
  }


  if (badChip >= 0) {    
    cout << "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZ badChip = " << badChip << " lkStatus = " << lkStatus[0] << " " << lkStatus[1] << " " << lkStatus[2] << endl;
    for (int i = 0; i < 3; ++i) {
      if (badLink[i]) {
        lkStatus[i] = 4;
      } else {
        lkStatus[i] = 5;
      }
    }
  }


  links.push_back(lkStatus[0]);
  links.push_back(lkStatus[1]);
  links.push_back(lkStatus[2]);
  links.push_back(lkStatus[3]);

  return true;
}

// ----------------------------------------------------------------------
// -- this is completely new and now relies on PCLS data read from midas meta data tree
bool determineBrokenLinksV0(TH2 *h, vector<int> &links) {
  bool DBX(false);

  // -- the histogram still used to provide the chip name
  string schip = h->GetName();
  replaceAll(schip, "hitmap_perChip_", "");
  int chipID = ::stoi(schip);

  // -- determine dead chips
  double nLinkA = h->Integral(1, 88, 1, 250);
  double nLinkB = h->Integral(89, 172, 1, 250);
  double nLinkC = h->Integral(173, 256, 1, 250);
  vector<double> vLink = {nLinkA, nLinkB, nLinkC};
  double nLinkAverage = (nLinkA + nLinkB + nLinkC)/3.;
  if (1) cout << "  nLinkA = " << nLinkA << " nLinkB = " << nLinkB << " nLinkC = " << nLinkC << " nLinkAverage = " << nLinkAverage << endl;
  if (nLinkA < 1 && nLinkB < 1 && nLinkC < 1) {
    links.push_back(8);
    links.push_back(8);
    links.push_back(8);
    links.push_back(8);
    return false;
  }

  // -- search for pathological cases where all hits are very close to the top (bottom) edge
  vector<double> vLinkMeanY;
  vLinkMeanY.push_back(h->ProjectionY("A", 1, 88)->GetMean());
  vLinkMeanY.push_back(h->ProjectionY("B", 89, 172)->GetMean());
  vLinkMeanY.push_back(h->ProjectionY("C", 173, 256)->GetMean());
  int badChip(-1); // -- if any link is broken and not masked, the chip is bad
  if (nLinkA > 0 && (vLinkMeanY[0] < 10 || vLinkMeanY[0] > 245)) badChip = 1;
  if (nLinkB > 0 && (vLinkMeanY[1] < 10 || vLinkMeanY[1] > 245)) badChip = 2;
  if (nLinkC > 0 && (vLinkMeanY[2] < 10 || vLinkMeanY[2] > 245)) badChip = 3;
  if (badChip > 0) {
    cout << "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZ badChip = " << badChip << " vLinkMeanY = " << vLinkMeanY[0] << " " << vLinkMeanY[1] << " " << vLinkMeanY[2] << endl;  
    if (nLinkA < 1) {
      links.push_back(8);
    } else {
      links.push_back(3);
    }
    if (nLinkB < 1) {
      links.push_back(8);
    } else {
      links.push_back(3);
    }
    if (nLinkC < 1) {
      links.push_back(8);
    } else {
      links.push_back(3);
    }
    links.push_back(0);
    return true;
  }


  // -- hand-curated list to avoid getting lost in algorithmic nightmares
  vector<int> vBadChips = {102};

  if (find(vBadChips.begin(), vBadChips.end(), chipID) != vBadChips.end()) {  
    links.push_back(3);
    links.push_back(3);
    links.push_back(3);
    links.push_back(3);
    return true;
  }

  AsicInfo ai = gRunInfoMap[chipID];

  // -- completely disabled chip
  if (0 == ai.abcLinkMask[0]
      && 0 == ai.abcLinkMask[1]
      && 0 == ai.abcLinkMask[2]) {
    links.push_back(9);
    links.push_back(9);
    links.push_back(9);
    links.push_back(0);
    return false;
  }

  
  // -- check individual links
  int lkStatus[3] = {0, 0, 0};
  int nBadLinks(0);
  badChip = -1;
  for (int i = 0; i < 3; ++i) {
    if (vLink[i] < 1) lkStatus[i] = 8;
    if (0 == ai.abcLinkMask[i]) lkStatus[i] = 9;
    if (9 == ai.abcLinkMask[i]) lkStatus[i] = 9;
    if (ai.abcLinkErrs[i] > 10) lkStatus[i] = 4;
    if (ai.abcLinkErrs[i] > 10 && 1 == ai.abcLinkMask[i]) {
      badChip = i;
      ++nBadLinks;
    }
  }
 
  for (int i = 0; i < 3; ++i) {
    cout << "Chip " << chipID << " link " << i << " status = " << lkStatus[i] 
    << " mask = " << ai.abcLinkMask[i] << " errs = " << ai.abcLinkErrs[i]
    << endl;
  }


  if (badChip >= 0) {    
    cout << "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZ badChip = " << badChip << " lkStatus = " << lkStatus[0] << " " << lkStatus[1] << " " << lkStatus[2] << endl;
    for (int i = 0; i < 3; ++i) {
      if (i != badChip) {
        lkStatus[i] = 5;
      } else {
        lkStatus[i] = 4;
      }
    }
  }

  // -- Marius' special case, ignoring EEE case IF LVDS error rate low enough
  // -- "So I would say if all links are EEE then we should not trust the 
  //     matrix information and only use the lvds errors."
  if (ai.abcLinkMatrix[0] == 4 && ai.abcLinkMatrix[1] == 4 && ai.abcLinkMatrix[2] == 4) {
   if (ai.abcLinkErrs[0] > 10) lkStatus[0] = 4;
   if (ai.abcLinkErrs[1] > 10) lkStatus[1] = 4;
   if (ai.abcLinkErrs[2] > 10) lkStatus[2] = 4;
   if (badChip >= 0) {
    cout << "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZ badChip = " << badChip << " lkStatus = " << lkStatus[0] << " " << lkStatus[1] << " " << lkStatus[2] << endl;
   }
  }

  links.push_back(lkStatus[0]);
  links.push_back(lkStatus[1]);
  links.push_back(lkStatus[2]);
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
      columns.push_back(8); 
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
      // -- skip defective links 
      if (links[0] > 0 && ix < 89) {
        continue;
      }
      if (links[1] > 0 && (ix >= 89 && ix < 173)) { 
        continue;
      }
      if (links[2] > 0 && ix >= 173) {
        continue;
      }
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


