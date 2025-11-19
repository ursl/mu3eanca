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

#include "calPixelAlignment.hh"
#include "calFibreAlignment.hh"
#include "calMppcAlignment.hh"
#include "calTileAlignment.hh"
#include "calPixelCablingMap.hh"
#include "calPixelQualityLM.hh"
#include "calPixelTimeCalibration.hh"

#include "calDetSetupV1.hh"

using namespace std;

// Forward declarations
void writeDetSetupV1(string jsondir, string gt, int iov);
void writePixelQualityLM(string jsondir, string gt, string filename, int iov);
void writeAlignmentInformation(string jsondir, string gt, string type, string ifilename, int iov);

// ----------------------------------------------------------------------
// cdbWritePayload
// ---------------
//
// NOTE: works (validated) for pixelalignement. All the rest might need some improvements!
//
// Usage examples   ./bin/cdbWritePayload -c pixelalignment -g datav6.3=2025V1test -j ~/data/mu3e/cdb -f ascii/sensors-alignment-CosmicTracksV0.csv -i 1
// --------------
//
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
int main(int argc, const char* argv[]) {

  // -- command line arguments
  string cal("");
  string jsondir("");
  string gt("");
  string filename("");
  int iov(1);
  int verbose(0);
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-c"))  {cal        = argv[++i];}
    if (!strcmp(argv[i], "-i"))  {iov        = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-j"))  {jsondir    = argv[++i];}
    if (!strcmp(argv[i], "-g"))  {gt         = argv[++i];}
    if (!strcmp(argv[i], "-f"))  {filename   = argv[++i];}
    if (!strcmp(argv[i], "-v"))  {verbose    = 1;}
  }
  
  cout << "===============" << endl;
  cout << "== cdbWritePayload ==" << endl;
  cout << "===============" << endl;
  cout << "== installing in directory " << jsondir << endl;
  cout << "== global tag " << gt << endl;
  cout << "== filename " << filename << endl;
  cout << "== iov " << iov << endl;
    
  
 
  // -- write alignment information payloads and tags
  if (string::npos != cal.find("alignment")) {
    writeAlignmentInformation(jsondir, gt, cal, filename, iov);
  }
  // -- write detsetupv1 (basically magnet status) payloads and tags
  //writeDetSetupV1(jsondir, gt, iov);
  // -- write pixelqualitylm payloads and tags
  //writePixelQualityLM(jsondir, gt, filename, iov);

  return 0;
}

// ----------------------------------------------------------------------
void writeDetSetupV1(string jsondir, string gt, string filename, int iov) {
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
  cdc->writePayloadToFile(hash, jsondir + "/payloads", pl);
  cout << "   ->cdbWritePayload> writing IOV " << iov << " with " << templateHash << endl;

  delete cdc;
}

// ----------------------------------------------------------------------
void writePixelQualityLM(string jsondir, string gt, string filename, int iov) {
  cout << "   ->cdbInitGT> writing local template pixelqualitylm payloads" << endl;
  // -- create (local template) payloads for no problematic pixels
  calPixelQualityLM *cpq = new calPixelQualityLM();
  cpq->readCsv(filename);
  string spl = cpq->makeBLOB();
  string hash = "tag_pixelqualitylm_" + gt + "_iov_" + to_string(iov);
  payload pl;
  pl.fHash = hash;
  pl.fComment = filename;
  pl.fSchema  = cpq->getSchema();
  pl.fBLOB = spl;
  cpq->writePayloadToFile(hash, jsondir + "/payloads", pl);
  cout << "   ->cdbWritePayload> writing IOV " << iov << " with " << hash << endl;
  delete cpq;
}


// ----------------------------------------------------------------------
void writeAlignmentInformation(string jsondir, string gt, string type, string ifilename, int iov) {
  cout << "   ->cdbWritePayload> writing alignment " << type << " from file " << ifilename << endl;

  // -- pixel sensor alignment
  if (type == "pixelalignment") {
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
      cpa->writePayloadToFile(hash, jsondir + "/payloads", pl);
    }
  } 
}

