#include <iostream>
#include <string.h>
#include <stdio.h>

#include <fstream>
#include <vector>
#include <sstream>
#include <dirent.h>  /// for directory reading
#include <iomanip>

#include <chrono>
#include <glob.h>

#include "cdbUtil.hh"
#include "base64.hh"

#include <TFile.h>
#include <TTree.h>
#include <map>
#include <vector>

#include "calPixelAlignment.hh"
#include "calFibreAlignment.hh"
#include "calMppcAlignment.hh"
#include "calTileAlignment.hh"
#include "calPixelQualityLM.hh"
#include "calPixelEfficiency.hh"
#include "calFibreQuality.hh"

#include "calDetSetupV1.hh"
#include "calEventStuffV1.hh"
using namespace std;

// Forward declarations
void writeDetSetupV1(string payloaddir, string gt, string annotation, int iov);
void writeAlignmentInformation(string payloaddir, string gt, string type, string ifilename, string annotation, int iov);
void writePixelEfficiencyPayload(string payloaddir, string gt, string filename, string annotation, int iov);
void writeEventStuffV1(string payloaddir, string gt, string filename, string annotation, int iov);

void writePixelQualityLM(string payloaddir, string gt, string filename, string annotation, int iov);
void writeFibreQuality(string payloaddir, string gt, string filename, string annotation, int iov);

// ----------------------------------------------------------------------
// cdbWritePayload
// ---------------
//
// NOTE: works (validated) for alignment. All the rest might need some improvements! FIXME fibrealignment (shape/round/square)
// 
//  -c pixelalignment    produce the pixelalignment payloads
//  -c tilealignment    produce the tilealignment payloads
//  -c fibrealignment   produce the fibrealignment payloads
//  -c mppcalignment    produce the mppcalignment payloads
//  -d inputfiledir
//  -f filename          file to read in
//  -g GT                the global tag (GT) for the payload (part of the "hash" written into the payload metadata)
//  -i RUN               the interval of validity, i.e., the first run for which the payload is valid
//  -p payloaddir        the CDB directory where the payloads will be written to payloaddir/payloads/
//
// Usage examples   
// --------------
//
// cdbWritePayload -c alignment -g mcidealv6.5 -f mu3e_alignment.root -a "complete detector with MC truth"
// cdbWritePayload -c eventstuffv1 -g mcideal -f ascii/eventstuff-ideal.json -a "entire run duration is perfect"
// cdbWritePayload -c pixelefficiency -g mcideal -f mu3e_alignment.root -a "no inefficiency"
// cdbWritePayload -c pixelqualitylm -g mcideal -f mu3e_alignment.root -a "perfect detector with no deficiencies"
// cdbWritePayload -c fibrequality -g mcideal -f ascii/fibre-asics-1.csv -a "all good"

// cdbWritePayload -c pixelalignment -g mcidealv6.5 -f mu3e_alignment.root
// cdbWritePayload -c tilealignment -g mcidealv6.5 -f mu3e_alignment.root
// cdbWritePayload -c fibrealignment -g mcidealv6.5 -f mu3e_alignment.root
// cdbWritePayload -c mppcalignment -g mcidealv6.5  -f mu3e_alignment.root
// 
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
int main(int argc, const char* argv[]) {

  // -- command line arguments
  string cal("");
  string payloaddir(".");
  string inputfiledir("");
  string annotation("");
  string gt("");
  string filename("");
  int iov(1);
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-a"))  {annotation = argv[++i];}
    if (!strcmp(argv[i], "-c"))  {cal        = argv[++i];}
    if (!strcmp(argv[i], "-d"))  {inputfiledir = argv[++i];}
    if (!strcmp(argv[i], "-i"))  {iov        = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-g"))  {gt         = argv[++i];}
    if (!strcmp(argv[i], "-f"))  {filename   = argv[++i];}
    if (!strcmp(argv[i], "-p"))  {payloaddir = argv[++i];}
  }
  
  cout << "===============" << endl;
  cout << "== cdbWritePayload ==" << endl;
  cout << "===============" << endl;
  cout << "== installing in directory " << payloaddir << endl;
  cout << "== global tag " << gt << endl;
  cout << "== filename " << filename << endl;
  cout << "== iov " << iov << endl;
  cout << "== annotation " << annotation << endl;

  if (inputfiledir != "") {
    vector<string> vfiles;
    DIR *folder = opendir(inputfiledir.c_str());
    if (folder == NULL) {
      cout << "Error failed to open " << inputfiledir << endl;
      return 0;  
    }
    struct dirent *entry;
    while ((entry = readdir(folder))) {
      if (entry->d_type == DT_REG) {
        // -- this cannot be moved to the function below because that expectes to get a filename
        if ((cal == "eventstuffv1") && (string::npos == string(entry->d_name).find(".mid.lz4.json"))) {
          continue;
        }
        vfiles.push_back(inputfiledir + "/" + entry->d_name);
      }
    }
    closedir(folder);
    for (auto it: vfiles) {
      filename = it;
      string srun = filename;
      replaceAll(srun, ".mid.lz4.json", "");
      replaceAll(srun, inputfiledir, "");
      replaceAll(srun, "/", "");
      replaceAll(srun, "run", "");
      int irun = ::stoi(srun);
      cout << "filename = " << filename << " srun ->" << srun << "<- run = " << irun << endl;
      writeEventStuffV1(payloaddir, gt, filename, annotation, irun);
    }
    return 0;
  }

  // -- write alignment information payloads and tags
  if (string::npos != cal.find("alignment")) {
    writeAlignmentInformation(payloaddir, gt, cal, filename, annotation, iov);
  }

  if ("alignment" == cal) {
    writeAlignmentInformation(payloaddir, gt, "pixelalignment", filename, annotation, iov);
    writeAlignmentInformation(payloaddir, gt, "tilealignment", filename, annotation, iov);
    writeAlignmentInformation(payloaddir, gt, "fibrealignment", filename, annotation, iov);
    writeAlignmentInformation(payloaddir, gt, "mppcalignment", filename, annotation, iov);
  }

  // -- write pixelqualitylm payloads and tags
  if (string::npos != cal.find("pixelqualitylm")) {
    writePixelQualityLM(payloaddir, gt, filename, annotation, iov);
  }
  
  // -- write pixelqualitylm payloads and tags
  if (string::npos != cal.find("fibrequality")) {
    writeFibreQuality(payloaddir, gt, filename, annotation, iov);
  }
  // -- write detsetupv1 (basically magnet status) payloads and tags
  //writeDetSetupV1(payloaddir, gt, iov);
  // -- write pixelefficiency payloads and tags
  if (string::npos != cal.find("pixelefficiency")) {
    writePixelEfficiencyPayload(payloaddir, gt, filename, annotation, iov);
  }
  // -- write eventstuffv1 payloads and tags
  if (string::npos != cal.find("eventstuffv1")) {
    writeEventStuffV1(payloaddir, gt, filename, annotation, iov);
  }
}

// ----------------------------------------------------------------------
void writePixelEfficiencyPayload(string payloaddir, string gt, string filename, string annotation, int iov) {
  cout << "   ->cdbInitGT> writing local template pixelefficiency payloads" << endl;
  // -- create (local template) payloads for pixelefficiency
  calPixelEfficiency *cpe = new calPixelEfficiency();
  if (string::npos != filename.find(".root")) {
    cout << "   ->cdbWritePayload> reading pixel chipIDs from root file " << filename << endl;
    TFile *file = TFile::Open(filename.c_str());
    TTree *ta = (TTree*)file->Get("alignment/sensors");
    unsigned int id;
    vector<unsigned int> vChipIDs;
    ta->SetBranchAddress("id", &id);
    for (int i = 0; i < ta->GetEntries(); ++i) {
      ta->GetEntry(i);
      vChipIDs.push_back(id);
    }
    cout << "   ->cdbWritePayload> read " << vChipIDs.size() << " chipIDs from tree with " << ta->GetEntries() << " entries" << endl;
    file->Close();
    // -- create a temporary csv file
    ofstream ONS;
    string tmpFilename = "pixelefficiency_tmp.csv";
    ONS.open(tmpFilename);
    for (auto &id : vChipIDs) {
      //  chipID,nsec,[1.000]
      ONS << id << "," << 18 << ",";
      for (int i = 0; i < 18; i++) {
        ONS << 1.000;
        if (i < 17) ONS << ",";
        else ONS << endl;
      }
    }
    ONS.close();
    filename = tmpFilename;
  } 
  cpe->readCsv(filename);
  string spl = cpe->makeBLOB();
  string hash = "tag_pixelefficiency_" + gt + "_iov_" + to_string(iov);
  payload pl;
  pl.fHash = hash;
  pl.fComment = annotation;
  pl.fSchema  = cpe->getSchema();
  pl.fBLOB = spl;
  cpe->writePayloadToFile(hash, payloaddir, pl);
  cout << "   ->cdbWritePayload> writing IOV " << iov << " with " << hash << endl;
  delete cpe;
}

// ----------------------------------------------------------------------
void writeDetSetupV1(string payloaddir, string gt, string filename, string annotation, int iov) {
  cout << "   ->cdbInitGT> writing local template detsetupv1 payloads" << endl;
  // -- create (local template) payloads for no field and with field
  calDetSetupV1 *cdc = new calDetSetupV1();
  string result = cdc->readJSON(filename);
  if (string::npos == result.find("Error")) {
    string spl = cdc->makeBLOB();
    string hash = "detsetupv1_noField";
    payload pl;
    if (fileExists(string(LOCALDIR) + "/" + hash)) {
      cout << "   ->cdbInitGT> payload " << hash << " already exists, skipping" << endl;
    } else {
      pl.fHash = hash;
      pl.fComment = "detector setup with magnet off (no magnet)";
      pl.fSchema  = cdc->getSchema();
      pl.fBLOB = spl;
      cdc->writePayloadToFile(hash, string(LOCALDIR), pl);
    }
  }

  string templateHash = "detsetupv1_";
  string hash = "tag_" + templateHash + gt + "_iov_" + to_string(iov);
  cdc->readPayloadFromFile(templateHash, filename);
  payload pl = cdc->getPayload(templateHash);
  pl.fSchema = cdc->getSchema();
  pl.fComment = annotation;
  pl.fHash = hash;
  cdc->writePayloadToFile(hash, payloaddir, pl);
  cout << "   ->cdbWritePayload> writing IOV " << iov << " with " << templateHash << endl;

  delete cdc;
}

// ----------------------------------------------------------------------
void writeEventStuffV1(string payloaddir, string gt, string filename, string annotation, int iov) {
  cout << "   ->cdbInitGT> writing local template eventstuffv1 payloads" << endl;
  // -- create (local template) payloads for eventstuffv1
  calEventStuffV1 *ces = new calEventStuffV1();
  ces->readJSON(filename);
  string hash = "tag_eventstuffv1_" + gt + "_iov_" + to_string(iov);
  payload pl;
  pl.fHash = hash;
  pl.fComment = annotation;
  pl.fSchema  = ces->getSchema();
  pl.fBLOB = ces->makeBLOB();
  cout << "   ->cdbWritePayload> writing payload with BLOB " << ces->printBLOBString(pl.fBLOB, 1000) << endl;
  ces->writePayloadToFile(hash, payloaddir, pl);
  cout << "   ->cdbWritePayload> writing IOV " << iov << " with " << hash << " and schema " << pl.fSchema << endl;
  delete ces;
}

// ----------------------------------------------------------------------
void writePixelQualityLM(string payloaddir, string gt, string filename, string annotation, int iov) {
  cout << "   ->cdbInitGT> writing local template pixelqualitylm payloads" << endl;
  // -- create (local template) payloads for no problematic pixels
  calPixelQualityLM *cpq = new calPixelQualityLM();
  if (string::npos != filename.find(".root")) {
    cout << "   ->cdbWritePayload> reading pixel chipIDs from root file " << filename << endl;
    TFile *file = TFile::Open(filename.c_str());
    TTree *ta = (TTree*)file->Get("alignment/sensors");
    unsigned int id;
    vector<unsigned int> vChipIDs;
    ta->SetBranchAddress("id", &id);
    for (int i = 0; i < ta->GetEntries(); ++i) {
      ta->GetEntry(i);
      vChipIDs.push_back(id);
    }
    cout << "   ->cdbWritePayload> read " << vChipIDs.size() << " chipIDs from tree with " << ta->GetEntries() << " entries" << endl;
    file->Close();
    // -- create a temporary csv file
    ofstream ONS;
    string tmpFilename = "pixelqualitylm_tmp.csv";
    ONS.open(tmpFilename);
    for (auto &id : vChipIDs) {
      //  chipID,ckdivend,ckdivend2,linkA,linkB,linkC,linkM,ncol[,icol],npix[,icol,irow,qual]     
      ONS << id << "," << 31 << "," << 0 << "," << 0 << "," << 0 << "," << 0 << "," << 0 << "," << 0 << "," << 0 << endl;
    }
    ONS.close();
    filename = tmpFilename;
  } 
  cpq->readCsv(filename);
  string spl = cpq->makeBLOB();
  string hash = "tag_pixelqualitylm_" + gt + "_iov_" + to_string(iov);
  payload pl;
  pl.fHash = hash;
  pl.fComment = annotation + string(". ") + cpq->getStatusDocumentation();
  pl.fSchema  = cpq->getSchema();
  pl.fBLOB = spl;
  cpq->writePayloadToFile(hash, payloaddir, pl);
  cout << "   ->cdbWritePayload> writing IOV " << iov << " with " << hash << " and comment " << pl.fComment << endl;
  delete cpq;
}


// ----------------------------------------------------------------------
void writeFibreQuality(string payloaddir, string gt, string filename, string annotation, int iov) {
  cout << "   ->cdbInitGT> writing local template fibrequality payloads" << endl;
  // -- create (local template) payloads for fibrequality
  calFibreQuality *cfq = new calFibreQuality();
  cfq->readCSV(filename);
  string spl = cfq->makeBLOB();
  string hash = "tag_fibrequality_" + gt + "_iov_" + to_string(iov);
  payload pl;
  pl.fHash = hash;
  pl.fComment = annotation;
  pl.fSchema  = cfq->getSchema();
  pl.fBLOB = spl;
  cfq->writePayloadToFile(hash, payloaddir, pl);
  cout << "   ->cdbWritePayload> writing IOV " << iov << " with " << hash << " and comment " << pl.fComment << endl;
  delete cfq;
}


// ----------------------------------------------------------------------
void writeAlignmentInformation(string payloaddir, string gt, string type, string ifilename, string annotation, int iov) {
  cout << "   ->cdbWritePayload> writing alignment " << type << " from file " << ifilename << endl;

  // -- pixel sensor alignment
  string tmpFilename("");
  if (string::npos != ifilename.find(".root")) {
    tmpFilename = ifilename;
    size_t pos = tmpFilename.find(".root");
    if (pos != string::npos) {
      tmpFilename.replace(pos, 5, ".csv-tmp");
    }
    // Remove directory path, keep only filename
    size_t lastSlash = tmpFilename.find_last_of("/");
    if (lastSlash != string::npos) {
      tmpFilename = tmpFilename.substr(lastSlash + 1);
    }
    cout << "   ->cdbWritePayload> temporary file " << tmpFilename << endl;
  }
  if (type == "pixelalignment") {
    if (string::npos != ifilename.find(".root")) {
      cout << "   ->cdbWritePayload> reading pixelalignment from root file " << ifilename << endl;
      struct sensor {
        unsigned int id;
        double vx, vy, vz;
        double rowx, rowy, rowz;
        double colx, coly, colz;
        int nrow, ncol;
        double width, length, thickness, pixelSize;
      };
      map<unsigned int, sensor> sensors;
      TFile *file = TFile::Open(ifilename.c_str());
      TTree *ta = (TTree*)file->Get("alignment/sensors");
      struct sensor a;
      ta->SetBranchAddress("id", &a.id);
      ta->SetBranchAddress("vx", &a.vx);
      ta->SetBranchAddress("vy", &a.vy);
      ta->SetBranchAddress("vz", &a.vz);
      ta->SetBranchAddress("rowx", &a.rowx);
      ta->SetBranchAddress("rowy", &a.rowy);
      ta->SetBranchAddress("rowz", &a.rowz);
      ta->SetBranchAddress("colx", &a.colx);
      ta->SetBranchAddress("coly", &a.coly);
      ta->SetBranchAddress("colz", &a.colz);
      ta->SetBranchAddress("nrow", &a.nrow);
      ta->SetBranchAddress("ncol", &a.ncol);
      ta->SetBranchAddress("width", &a.width);
      ta->SetBranchAddress("length", &a.length);
      ta->SetBranchAddress("thickness", &a.thickness);
      ta->SetBranchAddress("pixelSize", &a.pixelSize);
      int nbytes(0);
      for (int i = 0; i < ta->GetEntries(); ++i) {
        nbytes += ta->GetEntry(i);
        sensors.insert(make_pair(a.id, a));
      }
      cout << "   ->cdbWritePayload> read " << sensors.size() << " sensors from tree with " << ta->GetEntries() << " entries and " << nbytes << " bytes" << endl;
      ofstream ONS;
      ONS.open(tmpFilename);
      for (auto &s : sensors) {
        ONS << s.first << "," 
        << std::setprecision(15)
        << s.second.vx << "," << s.second.vy << "," << s.second.vz << "," 
        << s.second.rowx << "," << s.second.rowy << "," << s.second.rowz 
        << "," << s.second.colx << "," << s.second.coly << "," << s.second.colz 
        << "," << s.second.nrow << "," << s.second.ncol << "," 
        << s.second.width << "," << s.second.length << "," 
        << s.second.thickness << "," << s.second.pixelSize
        << endl;
      }
      ONS.close();
      file->Close();
    } 

    // -- if the input filename is a root file, use the temporary file
    if (string::npos != ifilename.find(".root")) {
      ifilename = tmpFilename;
    }

    calPixelAlignment *cpa = new calPixelAlignment();
    string result = cpa->readCsv(ifilename);
    if (string::npos == result.find("Error")) {
      string spl = cpa->makeBLOB();
      string hash = "tag_pixelalignment_" + gt + "_iov_" + to_string(iov);
      payload pl;
      pl.fHash = hash;
      pl.fComment = annotation;
      pl.fSchema  = cpa->getSchema();
      pl.fBLOB = spl;
      cpa->writePayloadToFile(hash, payloaddir, pl);
    }
    if (string::npos != ifilename.find(".root")) {
      cout << "   ->cdbWritePayload> removing temporary file " << tmpFilename << endl;
      remove(tmpFilename.c_str());
    }
  } 

  if (type == "mppcalignment") {
    if (string::npos != ifilename.find(".root")) {
      cout << "   ->cdbWritePayload> reading mppcalignment from root file " << ifilename << endl;
      TFile *file = TFile::Open(ifilename.c_str());
      TTree *ta = (TTree*)file->Get("alignment/mppcs");
      struct mppc {
        unsigned int mppc;
        double vx, vy, vz;
        double colx, coly, colz;
        int ncol;
      };
      map<unsigned int, mppc> mppcs;
      struct mppc m;
      ta->SetBranchAddress("mppc", &m.mppc);
      ta->SetBranchAddress("vx", &m.vx);
      ta->SetBranchAddress("vy", &m.vy);
      ta->SetBranchAddress("vz", &m.vz);
      ta->SetBranchAddress("colx", &m.colx);
      ta->SetBranchAddress("coly", &m.coly);
      ta->SetBranchAddress("colz", &m.colz);
      ta->SetBranchAddress("ncol", &m.ncol);
      int nbytes(0);
      for (int i = 0; i < ta->GetEntries(); ++i) {
        nbytes += ta->GetEntry(i);
        mppcs.insert(make_pair(m.mppc, m));
      }
      cout << "   ->cdbWritePayload> read " << mppcs.size() << " mppcs from tree with " << ta->GetEntries() << " entries and " << nbytes << " bytes" << endl;
      ofstream ONS;
      ONS.open(tmpFilename);
      for (auto &m : mppcs) {
        ONS << m.first << "," 
        << std::setprecision(15)
        << m.second.vx << "," << m.second.vy << "," << m.second.vz << "," 
        << m.second.colx << "," << m.second.coly << "," << m.second.colz << ","
        << m.second.ncol
        << endl;
      }
      ONS.close();
      file->Close();
    } 

    // -- if the input filename is a root file, use the temporary file
    if (string::npos != ifilename.find(".root")) {
      ifilename = tmpFilename;
    }

    calMppcAlignment *cmp = new calMppcAlignment();
    string result = cmp->readCsv(ifilename);
    if (string::npos == result.find("Error")) {
      string spl = cmp->makeBLOB();
      string hash = "tag_mppcalignment_" + gt + "_iov_" + to_string(iov);
      payload pl;
      pl.fHash = hash;
      pl.fComment = annotation;
      pl.fSchema  = cmp->getSchema();
      pl.fBLOB = spl;
      cmp->writePayloadToFile(hash, payloaddir, pl);
    }
    if (string::npos != ifilename.find(".root")) {
      cout << "   ->cdbWritePayload> removing temporary file " << tmpFilename << endl;
      remove(tmpFilename.c_str());
    }
  }

  if (type == "tilealignment") {
    if (string::npos != ifilename.find(".root")) {
      cout << "   ->cdbWritePayload> reading tilealignment from root file " << ifilename << endl;
      TFile *file = TFile::Open(ifilename.c_str());
      TTree *ta = (TTree*)file->Get("alignment/tiles");
      struct tile {
        unsigned int id;
        double posx, posy, posz;
        double dirx, diry, dirz;
      };
      map<unsigned int, tile> tiles;
      struct tile t;
      ta->SetBranchAddress("id", &t.id);
      ta->SetBranchAddress("posx", &t.posx);
      ta->SetBranchAddress("posy", &t.posy);
      ta->SetBranchAddress("posz", &t.posz);
      ta->SetBranchAddress("dirx", &t.dirx);
      ta->SetBranchAddress("diry", &t.diry);
      ta->SetBranchAddress("dirz", &t.dirz);
      int nbytes(0);
      for (int i = 0; i < ta->GetEntries(); ++i) {
        nbytes += ta->GetEntry(i);
        tiles.insert(make_pair(t.id, t));
      }
      cout << "   ->cdbWritePayload> read " << tiles.size() << " tiles from tree with " << ta->GetEntries() << " entries and " << nbytes << " bytes" << endl;
      ofstream ONS;
      ONS.open(tmpFilename);
      for (auto &t : tiles) {
        ONS << t.first << "," << t.first << ","
            << std::setprecision(15)
            << t.second.posx << "," << t.second.posy << "," << t.second.posz << ","
            << t.second.dirx << "," << t.second.diry << "," << t.second.dirz
            << endl;
      }
      ONS.close();
      file->Close();
    }

    if (string::npos != ifilename.find(".root")) {
      ifilename = tmpFilename;
    }

    calTileAlignment *cta = new calTileAlignment();
    string result = cta->readCsv(ifilename);
    if (string::npos == result.find("Error")) {
      string spl = cta->makeBLOB();
      string hash = "tag_tilealignment_" + gt + "_iov_" + to_string(iov);
      payload pl;
      pl.fHash = hash;
      pl.fComment = annotation;
      pl.fSchema  = cta->getSchema();
      pl.fBLOB = spl;
      cta->writePayloadToFile(hash, payloaddir, pl);
    }
    if (string::npos != ifilename.find(".root")) {
      cout << "   ->cdbWritePayload> removing temporary file " << tmpFilename << endl;
      remove(tmpFilename.c_str());
    }
    delete cta;
  }

  if (type == "fibrealignment") {
    if (string::npos != ifilename.find(".root")) {
      cout << "   ->cdbWritePayload> reading fibrealignment from root file " << ifilename << endl;
      TFile *file = TFile::Open(ifilename.c_str());
      TTree *ta = (TTree*)file->Get("alignment/fibres");
      struct fibre {
        unsigned int fibre;
        double cx, cy, cz;
        double fx, fy, fz;
        bool round, square;
        double diameter;
      };
      map<unsigned int, fibre> fibres;
      struct fibre f;
      ta->SetBranchAddress("fibre", &f.fibre);
      ta->SetBranchAddress("cx", &f.cx);
      ta->SetBranchAddress("cy", &f.cy);
      ta->SetBranchAddress("cz", &f.cz);
      ta->SetBranchAddress("fx", &f.fx);
      ta->SetBranchAddress("fy", &f.fy);
      ta->SetBranchAddress("fz", &f.fz);
      f.round = true;
      f.square = false;
      ta->SetBranchAddress("diameter", &f.diameter);
      int nbytes(0);
      for (int i = 0; i < ta->GetEntries(); ++i) {
        nbytes += ta->GetEntry(i);
        fibres.insert(make_pair(f.fibre, f));
      }
      cout << "   ->cdbWritePayload> read " << fibres.size() << " fibres from tree with " << ta->GetEntries() << " entries and " << nbytes << " bytes" << endl;
      ofstream ONS;
      ONS.open(tmpFilename);
      for (auto &f : fibres) {
        ONS << f.first << ","
            << std::setprecision(15)
            << f.second.cx << "," << f.second.cy << "," << f.second.cz << ","
            << f.second.fx << "," << f.second.fy << "," << f.second.fz << ","
            << f.second.round << "," << f.second.square << ","
            << f.second.diameter
            << endl;
      }
      ONS.close();
      file->Close();
    }

    if (string::npos != ifilename.find(".root")) {
      ifilename = tmpFilename;
    }

    calFibreAlignment *cfa = new calFibreAlignment();
    string result = cfa->readCsv(ifilename);
    if (string::npos == result.find("Error")) {
      string spl = cfa->makeBLOB();
      string hash = "tag_fibrealignment_" + gt + "_iov_" + to_string(iov);
      payload pl;
      pl.fHash = hash;
      pl.fComment = annotation;
      pl.fSchema  = cfa->getSchema();
      pl.fBLOB = spl;
      cfa->writePayloadToFile(hash, payloaddir, pl);
    }
    if (string::npos != ifilename.find(".root")) {
      cout << "   ->cdbWritePayload> removing temporary file " << tmpFilename << endl;
      remove(tmpFilename.c_str());
    }
    delete cfa;
  }


}

