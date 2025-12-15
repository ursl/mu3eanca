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
#include "calPixelCablingMap.hh"
#include "calPixelQualityLM.hh"
#include "calPixelTimeCalibration.hh"
#include "calPixelEfficiency.hh"

#include "calDetSetupV1.hh"
#include "calEventStuffV1.hh"
using namespace std;

// Forward declarations
void writeDetSetupV1(string payloaddir, string gt, int iov);
void writePixelQualityLM(string payloaddir, string gt, string filename, int iov);
void writeAlignmentInformation(string payloaddir, string gt, string type, string ifilename, int iov);
void writePixelEfficiencyPayload(string payloaddir, string gt, string filename, int iov);
void writeEventStuffV1(string payloaddir, string gt, string filename, int iov);
// ----------------------------------------------------------------------
// cdbWritePayload
// ---------------
//
// NOTE: works (validated) for pixelalignement. All the rest might need some improvements!
// 
//  -c pixelalignment    produce the pixelalignment payloads. This is (currently) the only properly validated usage.
//  -f filename          file to read in
//  -g GT                the global tag (GT) for the payload (part of the "hash" written into the payload metadata)
//  -i RUN               the interval of validity, i.e., the first run for which the payload is valid
//  -p payloaddir        the CDB directory where the payloads will be written to payloaddir/payloads/
//
// Usage examples   
// --------------
//
// cdbWritePayload -c pixelalignment -g datav6.3=2025V1test -i 1 -j ~/data/mu3e/cdb -f ascii/sensors-alignment-CosmicTracksV0.csv 
// cdbWritePayload -c pixelalignment -g datav6.3=2025V1test -i 1 -j ~/data/mu3e/cdb -f Downloads/cosmic_alignment_xyz.root 
// 
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
int main(int argc, const char* argv[]) {

  // -- command line arguments
  string cal("");
  string payloaddir(".");
  string gt("");
  string filename("");
  int iov(1);
  int verbose(0);
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-c"))  {cal        = argv[++i];}
    if (!strcmp(argv[i], "-i"))  {iov        = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-g"))  {gt         = argv[++i];}
    if (!strcmp(argv[i], "-f"))  {filename   = argv[++i];}
    if (!strcmp(argv[i], "-p"))  {payloaddir = argv[++i];}
    if (!strcmp(argv[i], "-v"))  {verbose    = 1;}
  }
  
  cout << "===============" << endl;
  cout << "== cdbWritePayload ==" << endl;
  cout << "===============" << endl;
  cout << "== installing in directory " << payloaddir << endl;
  cout << "== global tag " << gt << endl;
  cout << "== filename " << filename << endl;
  cout << "== iov " << iov << endl;
    
  
 
  // -- write alignment information payloads and tags
  if (string::npos != cal.find("alignment")) {
    writeAlignmentInformation(payloaddir, gt, cal, filename, iov);
  }
  // -- write detsetupv1 (basically magnet status) payloads and tags
  //writeDetSetupV1(payloaddir, gt, iov);
  // -- write pixelqualitylm payloads and tags
  if (string::npos != cal.find("pixelqualitylm")) {
    writePixelQualityLM(payloaddir, gt, filename, iov);
  }
  // -- write pixelefficiency payloads and tags
  if (string::npos != cal.find("pixelefficiency")) {
    writePixelEfficiencyPayload(payloaddir, gt, filename, iov);
  }
  // -- write eventstuffv1 payloads and tags
  if (string::npos != cal.find("eventstuffv1")) {
    writeEventStuffV1(payloaddir, gt, filename, iov);
  }
}

// ----------------------------------------------------------------------
void writePixelEfficiencyPayload(string payloaddir, string gt, string filename, int iov) {
  cout << "   ->cdbInitGT> writing local template pixelefficiency payloads" << endl;
  // -- create (local template) payloads for pixelefficiency
  calPixelEfficiency *cpe = new calPixelEfficiency();
  cpe->readCsv(filename);
  string spl = cpe->makeBLOB();
  string hash = "tag_pixelefficiency_" + gt + "_iov_" + to_string(iov);
  payload pl;
  pl.fHash = hash;
  pl.fComment = filename;
  pl.fSchema  = cpe->getSchema();
  pl.fBLOB = spl;
  cpe->writePayloadToFile(hash, payloaddir, pl);
  cout << "   ->cdbWritePayload> writing IOV " << iov << " with " << hash << endl;
  delete cpe;
}

// ----------------------------------------------------------------------
void writeDetSetupV1(string payloaddir, string gt, string filename, int iov) {
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
  pl.fComment = filename;
  pl.fHash = hash;
  cdc->writePayloadToFile(hash, payloaddir, pl);
  cout << "   ->cdbWritePayload> writing IOV " << iov << " with " << templateHash << endl;

  delete cdc;
}

// ----------------------------------------------------------------------
void writeEventStuffV1(string payloaddir, string gt, string filename, int iov) {
  cout << "   ->cdbInitGT> writing local template eventstuffv1 payloads" << endl;
  // -- create (local template) payloads for eventstuffv1
  calEventStuffV1 *ces = new calEventStuffV1();
  ces->readJSON(filename);
  string hash = "tag_eventstuffv1_" + gt + "_iov_" + to_string(iov);
  payload pl;
  pl.fHash = hash;
  pl.fComment = filename;
  pl.fSchema  = ces->getSchema();
  pl.fBLOB = ces->makeBLOB();
  cout << "   ->cdbWritePayload> writing payload with BLOB " << ces->printBLOBString(pl.fBLOB, 1000) << endl;
  ces->writePayloadToFile(hash, payloaddir, pl);
  cout << "   ->cdbWritePayload> writing IOV " << iov << " with " << hash << " and schema " << pl.fSchema << endl;
  delete ces;
}

// ----------------------------------------------------------------------
void writePixelQualityLM(string payloaddir, string gt, string filename, int iov) {
  cout << "   ->cdbInitGT> writing local template pixelqualitylm payloads" << endl;
  // -- create (local template) payloads for no problematic pixels
  calPixelQualityLM *cpq = new calPixelQualityLM();
  cpq->readCsv(filename);
  string spl = cpq->makeBLOB();
  string hash = "tag_pixelqualitylm_" + gt + "_iov_" + to_string(iov);
  payload pl;
  pl.fHash = hash;
  pl.fComment = filename + string(". ") + cpq->getStatusDocumentation();
  pl.fSchema  = cpq->getSchema();
  pl.fBLOB = spl;
  cpq->writePayloadToFile(hash, payloaddir, pl);
  cout << "   ->cdbWritePayload> writing IOV " << iov << " with " << hash << " and comment " << pl.fComment << endl;
  delete cpq;
}


// ----------------------------------------------------------------------
void writeAlignmentInformation(string payloaddir, string gt, string type, string ifilename, int iov) {
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
      pl.fComment = ifilename;
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
      cout << "   ->cdbWritePayload> read " << mppcs.size() << " mppcs from tree with " << ta->GetEntries() << " entries" << endl;
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
      pl.fComment = ifilename;
      pl.fSchema  = cmp->getSchema();
      pl.fBLOB = spl;
      cmp->writePayloadToFile(hash, payloaddir, pl);
    }
    if (string::npos != ifilename.find(".root")) {
      cout << "   ->cdbWritePayload> removing temporary file " << tmpFilename << endl;
      remove(tmpFilename.c_str());
    }
  } 


}

