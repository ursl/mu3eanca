#include "cdbPayloadWriter.hh"

#include "cdbUtil.hh"
#include "base64.hh"

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <sstream>
#include <dirent.h>
#include <iomanip>
#include <chrono>
#include <glob.h>

#include <TFile.h>
#include <TTree.h>
#include <map>

#include "calPixelAlignment.hh"
#include "calFibreAlignment.hh"
#include "calMppcAlignment.hh"
#include "calTileAlignment.hh"
#include "calPixelQualityLM.hh"
#include "calPixelEfficiency.hh"
#include "calFibreQuality.hh"
#include "calTileQuality.hh"
#include "calDetSetupV1.hh"
#include "calEventStuffV1.hh"
#include "calPixelTimeCalibration.hh"

using namespace std;

// ----------------------------------------------------------------------
cdbPayloadWriter::cdbPayloadWriter() {
  fChipIDs.clear();
  // -- layer 1
  fLayer1ChipIDs = {
    1,2,3,4,5,6,
    33, 34, 35, 36, 37, 38,
    65, 66, 67, 68, 69, 70,
    97, 98, 99, 100, 101, 102,
    129, 130, 131, 132, 133, 134,
    161, 162, 163, 164, 165, 166,
    193, 194, 195, 196, 197, 198,
    225, 226, 227, 228, 229, 230
  };
  // -- layer 2
  fLayer2ChipIDs = {
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

  // -- layer 3



  // -- combination
  fChipIDs.insert(fChipIDs.end(), fLayer1ChipIDs.begin(), fLayer1ChipIDs.end());
  fChipIDs.insert(fChipIDs.end(), fLayer2ChipIDs.begin(), fLayer2ChipIDs.end());
}

// ----------------------------------------------------------------------
void cdbPayloadWriter::writePixelTimeCalibrationIdealInput(string filename) {
  cout << "   ->cdbInitGT> writing local template pixeltimecalibration ideal input" << endl;
  ofstream ONS;
  ONS.open(filename);
  for (auto &id : fChipIDs) {
    for (int i = 0; i < 6; i++) {
      for (int j = 0; j < 32; j++) {
        ONS << id << " " << i << " " << j << " 0 0 0 0" << endl;
      }
    }
  }
  ONS.close();
}

// ----------------------------------------------------------------------
void cdbPayloadWriter::writePixelEfficiencyPayloads(string payloaddir, string gt, string filename, string annotation, int iov) {
  cout << "   ->cdbInitGT> writing local template pixelefficiency payloads" << endl;
  calPixelEfficiency *cpe = new calPixelEfficiency();
  string tmpFilename = "pixelefficiency_tmp.csv";
  if (string::npos != filename.find(".root")) {
    cout << "   ->cdbWritePayload> reading pixel chipIDs from root file " << filename << endl;
    TFile *file = TFile::Open(filename.c_str());
    TTree *ta = static_cast<TTree*>(file->Get("alignment/sensors"));
    unsigned int id;
    vector<unsigned int> vChipIDs;
    ta->SetBranchAddress("id", &id);
    for (int i = 0; i < ta->GetEntries(); ++i) {
      ta->GetEntry(i);
      vChipIDs.push_back(id);
    }
    cout << "   ->cdbWritePayload> read " << vChipIDs.size() << " chipIDs from tree with " << ta->GetEntries() << " entries" << endl;
    file->Close();
    ofstream ONS;
    ONS.open(tmpFilename);
    for (auto &id : vChipIDs) {
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
  if (filename == tmpFilename) { 
    cout << "->cdbWritePayload> removing temporary file " << tmpFilename << endl; 
    remove(tmpFilename.c_str()); 
  }
}

// ----------------------------------------------------------------------
void cdbPayloadWriter::writeDetSetupV1Payloads(string payloaddir, string gt, string filename, string annotation, int iov) {
  cout << "   ->cdbInitGT> writing local template detsetupv1 payloads" << endl;
  calDetSetupV1 *cdc = new calDetSetupV1();
  string result = cdc->readJSON(filename);
  string templateHash = "detsetupv1_";
  string hash = "tag_" + templateHash + gt + "_iov_" + to_string(iov);
  payload pl;
  pl.fHash = hash;
  pl.fComment = annotation;
  pl.fSchema  = cdc->getSchema();
  pl.fBLOB = cdc->makeBLOB();
  cdc->writePayloadToFile(hash, payloaddir, pl);
  cout << "   ->cdbWritePayload> writing IOV " << iov << " with " << hash << " and comment " << pl.fComment << endl;
  delete cdc;
}

// ----------------------------------------------------------------------
void cdbPayloadWriter::writeEventStuffV1Payloads(string payloaddir, string gt, string filename, string annotation, int iov) {
  cout << "   ->cdbInitGT> writing local template eventstuffv1 payloads" << endl;
  calEventStuffV1 *ces = new calEventStuffV1();
  ces->readJSON(filename);
  string hash = "tag_eventstuffv1_" + gt + "_iov_" + to_string(iov);
  payload pl;
  pl.fHash = hash;
  pl.fComment = annotation;
  pl.fSchema  = ces->getSchema();
  pl.fBLOB = ces->makeBLOB();
  ces->writePayloadToFile(hash, payloaddir, pl);
  cout << "   ->cdbWritePayload> writing IOV " << iov << " with " << hash << " and schema " << pl.fSchema << endl;
  delete ces;
}

// ----------------------------------------------------------------------
void cdbPayloadWriter::writePixelQualityLMPayloads(string payloaddir, string gt, string filename, string annotation, int iov) {
  cout << "   ->cdbInitGT> writing local template pixelqualitylm payloads" << endl;
  calPixelQualityLM *cpq = new calPixelQualityLM();
  if (string::npos != filename.find(".root")) {
    cout << "   ->cdbWritePayload> reading pixel chipIDs from root file " << filename << endl;
    TFile *file = TFile::Open(filename.c_str());
    TTree *ta = static_cast<TTree*>(file->Get("alignment/sensors"));
    unsigned int id;
    vector<unsigned int> vChipIDs;
    ta->SetBranchAddress("id", &id);
    for (int i = 0; i < ta->GetEntries(); ++i) {
      ta->GetEntry(i);
      vChipIDs.push_back(id);
    }
    cout << "   ->cdbWritePayload> read " << vChipIDs.size() << " chipIDs from tree with " << ta->GetEntries() << " entries" << endl;
    file->Close();
    ofstream ONS;
    string tmpFilename = "pixelqualitylm_tmp.csv";
    ONS.open(tmpFilename);
    for (auto &id : vChipIDs) {
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
  if (string::npos == filename.find(".root")) { 
    cout << "->cdbWritePayload> removing temporary file " << filename << endl; 
    remove(filename.c_str()); 
  }
}

// ----------------------------------------------------------------------
void cdbPayloadWriter::writeFibreQualityPayloads(string payloaddir, string gt, string filename, string annotation, int iov) {
  cout << "   ->cdbInitGT> writing local template fibrequality payloads" << endl;
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
void cdbPayloadWriter::writeTileQualityPayloads(string payloaddir, string gt, string filename, string annotation, int iov) {
  cout << "   ->cdbInitGT> writing local template tilequality payloads" << endl;
  calTileQuality *ctq = new calTileQuality();
  ctq->readJSON(filename);
  string spl = ctq->makeBLOB();
  string hash = "tag_tilequality_" + gt + "_iov_" + to_string(iov);
  payload pl;
  pl.fHash = hash;
  pl.fComment = annotation;
  pl.fSchema  = ctq->getSchema();
  pl.fBLOB = spl;
  ctq->writePayloadToFile(hash, payloaddir, pl);
  cout << "   ->cdbWritePayload> writing IOV " << iov << " with " << hash << " and comment " << pl.fComment << endl;
  delete ctq;
}

// ----------------------------------------------------------------------
void cdbPayloadWriter::writePixelTimeCalibrationPayloads(string payloaddir, string gt, string filename, string annotation, int iov) {
  cout << "   ->cdbInitGT> writing local template pixeltimecalibration payloads" << endl;
  calPixelTimeCalibration *cpt = new calPixelTimeCalibration();
  cpt->readTxtFile(filename);
  string spl = cpt->makeBLOB();
  string hash = "tag_pixeltimecalibration_" + gt + "_iov_" + to_string(iov);
  
  payload pl;
  pl.fHash = hash;
  pl.fComment = annotation;
  pl.fSchema  = cpt->getSchema();
  pl.fBLOB = spl;
  cpt->writePayloadToFile(hash, payloaddir, pl);
  cout << "   ->cdbWritePayload> writing IOV " << iov << " with " << hash << " and comment " << pl.fComment << endl;
  delete cpt;
}


// ----------------------------------------------------------------------
void cdbPayloadWriter::writeAlignmentPayloads(string payloaddir, string gt, string type, string ifilename, string annotation, int iov) {
  cout << "   ->cdbWritePayload> writing alignment " << type << " from file " << ifilename << endl;
  
  string tmpFilename("");
  if (string::npos != ifilename.find(".root")) {
    tmpFilename = ifilename;
    size_t pos = tmpFilename.find(".root");
    if (pos != string::npos) tmpFilename.replace(pos, 5, "_tmp.csv");
    size_t lastSlash = tmpFilename.find_last_of("/");
    if (lastSlash != string::npos) tmpFilename = tmpFilename.substr(lastSlash + 1);
    cout << "   ->cdbWritePayload> temporary file " << tmpFilename << endl;
  }
  if (string::npos != type.find("pixelalignment")) {
    if (string::npos != ifilename.find(".root")) {
      cout << "   ->cdbWritePayload> reading pixelalignment from root file " << ifilename << endl;
      struct sensor { unsigned int id; double vx, vy, vz; double rowx, rowy, rowz; double colx, coly, colz; int nrow, ncol; double width, length, thickness, pixelSize; };
      map<unsigned int, sensor> sensors;
      TFile *file = TFile::Open(ifilename.c_str());
      TTree *ta = static_cast<TTree*>(file->Get("alignment/sensors"));
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
      for (int i = 0; i < ta->GetEntries(); ++i) { 
        ta->GetEntry(i); 
        sensors.insert(make_pair(a.id, a)); 
      }
      cout << "   ->cdbWritePayload> read " << sensors.size() << " sensors" << endl;
      ofstream ONS; ONS.open(tmpFilename);
      for (auto &s : sensors) {
        ONS << s.first << "," << std::setprecision(15) << s.second.vx << "," << s.second.vy << "," << s.second.vz << ","
        << s.second.rowx << "," << s.second.rowy << "," << s.second.rowz << "," << s.second.colx << "," << s.second.coly << "," << s.second.colz
        << "," << s.second.nrow << "," << s.second.ncol << "," << s.second.width << "," << s.second.length << "," << s.second.thickness << "," << s.second.pixelSize << endl;
      }
      ONS.close(); file->Close();
    }
    if (string::npos != ifilename.find(".root")) ifilename = tmpFilename;
    calPixelAlignment *cpa = new calPixelAlignment();
    string result = cpa->readCsv(ifilename);
    if (string::npos == result.find("Error")) {
      string spl = cpa->makeBLOB();
      string hash = "tag_pixelalignment_" + gt + "_iov_" + to_string(iov);
      payload pl; pl.fHash = hash; pl.fComment = annotation; pl.fSchema = cpa->getSchema(); pl.fBLOB = spl;
      cpa->writePayloadToFile(hash, payloaddir, pl);
    }
    if (string::npos != ifilename.find(".root")) { cout << "   ->cdbWritePayload> removing temporary file " << tmpFilename << endl; remove(tmpFilename.c_str()); }
  }
  
  if (string::npos != type.find("mppcalignment")) {
    if (string::npos != ifilename.find(".root")) {
      cout << "   ->cdbWritePayload> reading mppcalignment from root file " << ifilename << endl;
      struct mppc { unsigned int mppc; double vx, vy, vz; double colx, coly, colz; int ncol; };
      map<unsigned int, mppc> mppcs;
      struct mppc m;
      TFile *file = TFile::Open(ifilename.c_str());
      TTree *ta = static_cast<TTree*>(file->Get("alignment/mppcs"));
      ta->SetBranchAddress("mppc", &m.mppc); 
      ta->SetBranchAddress("vx", &m.vx); 
      ta->SetBranchAddress("vy", &m.vy); 
      ta->SetBranchAddress("vz", &m.vz);
      ta->SetBranchAddress("colx", &m.colx); 
      ta->SetBranchAddress("coly", &m.coly); 
      ta->SetBranchAddress("colz", &m.colz); 
      ta->SetBranchAddress("ncol", &m.ncol);
      for (int i = 0; i < ta->GetEntries(); ++i) { 
        ta->GetEntry(i); 
        mppcs.insert(make_pair(m.mppc, m)); 
      }
      cout << "   ->cdbWritePayload> read " << mppcs.size() << " mppcs" << endl;
      ofstream ONS; ONS.open(tmpFilename);
      for (auto &m : mppcs) ONS << m.first << "," << std::setprecision(15) << m.second.vx << "," << m.second.vy << "," << m.second.vz << "," << m.second.colx << "," << m.second.coly << "," << m.second.colz << "," << m.second.ncol << endl;
      ONS.close(); file->Close();
    }
    if (string::npos != ifilename.find(".root")) ifilename = tmpFilename;
    calMppcAlignment *cmp = new calMppcAlignment();
    string result = cmp->readCsv(ifilename);
    if (string::npos == result.find("Error")) {
      string spl = cmp->makeBLOB();
      string hash = "tag_mppcalignment_" + gt + "_iov_" + to_string(iov);
      payload pl; pl.fHash = hash; pl.fComment = annotation; pl.fSchema = cmp->getSchema(); pl.fBLOB = spl;
      cmp->writePayloadToFile(hash, payloaddir, pl);
    }
    if (string::npos != ifilename.find(".root")) { 
      cout << "   ->cdbWritePayload> removing temporary file " << tmpFilename << endl; 
      remove(tmpFilename.c_str()); 
    }
    delete cmp;
  }
  
  if (string::npos != type.find("tilealignment")) {
    if (string::npos != ifilename.find(".root")) {
      cout << "   ->cdbWritePayload> reading tilealignment from root file " << ifilename << endl;
      struct tile { unsigned int id; double posx, posy, posz; double dirx, diry, dirz; };
      map<unsigned int, tile> tiles;
      struct tile t;
      TFile *file = TFile::Open(ifilename.c_str());
      TTree *ta = static_cast<TTree*>(file->Get("alignment/tiles"));
      ta->SetBranchAddress("id", &t.id); 
      ta->SetBranchAddress("posx", &t.posx); 
      ta->SetBranchAddress("posy", &t.posy); 
      ta->SetBranchAddress("posz", &t.posz);
      ta->SetBranchAddress("dirx", &t.dirx); 
      ta->SetBranchAddress("diry", &t.diry); 
      ta->SetBranchAddress("dirz", &t.dirz);
      for (int i = 0; i < ta->GetEntries(); ++i) { 
        ta->GetEntry(i); 
        tiles.insert(make_pair(t.id, t)); 
      }
      cout << "   ->cdbWritePayload> read " << tiles.size() << " tiles" << endl;
      ofstream ONS; ONS.open(tmpFilename);
      for (auto &t : tiles) ONS << t.first << "," << t.first << "," << std::setprecision(15) << t.second.posx << "," << t.second.posy << "," << t.second.posz << "," << t.second.dirx << "," << t.second.diry << "," << t.second.dirz << endl;
      ONS.close(); file->Close();
    }
    if (string::npos != ifilename.find(".root")) ifilename = tmpFilename;
    calTileAlignment *cta = new calTileAlignment();
    string result = cta->readCsv(ifilename);
    if (string::npos == result.find("Error")) {
      string spl = cta->makeBLOB();
      string hash = "tag_tilealignment_" + gt + "_iov_" + to_string(iov);
      payload pl; pl.fHash = hash; pl.fComment = annotation; pl.fSchema = cta->getSchema(); pl.fBLOB = spl;
      cta->writePayloadToFile(hash, payloaddir, pl);
    }
    if (string::npos != ifilename.find(".root")) { 
      cout << "   ->cdbWritePayload> removing temporary file " << tmpFilename << endl; 
      remove(tmpFilename.c_str()); 
    }
    delete cta;
  }
  
  if (string::npos != type.find("fibrealignment")) {
    if (string::npos != ifilename.find(".root")) {
      cout << "   ->cdbWritePayload> reading fibrealignment from root file " << ifilename << endl;
      struct fibre { unsigned int fibre; double cx, cy, cz; double fx, fy, fz; bool round, square; double diameter; };
      map<unsigned int, fibre> fibres;
      struct fibre f;
      TFile *file = TFile::Open(ifilename.c_str());
      TTree *ta = static_cast<TTree*>(file->Get("alignment/fibres"));
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
      for (int i = 0; i < ta->GetEntries(); ++i) { 
        ta->GetEntry(i); 
        fibres.insert(make_pair(f.fibre, f)); 
      }
      cout << "   ->cdbWritePayload> read " << fibres.size() << " fibres" << endl;
      ofstream ONS; ONS.open(tmpFilename);
      for (auto &f : fibres) ONS << f.first << "," << std::setprecision(15) << f.second.cx << "," << f.second.cy << "," << f.second.cz << "," << f.second.fx << "," << f.second.fy << "," << f.second.fz << "," << f.second.round << "," << f.second.square << "," << f.second.diameter << endl;
      ONS.close(); file->Close();
    }
    if (string::npos != ifilename.find(".root")) ifilename = tmpFilename;
    calFibreAlignment *cfa = new calFibreAlignment();
    string result = cfa->readCsv(ifilename);
    if (string::npos == result.find("Error")) {
      string spl = cfa->makeBLOB();
      string hash = "tag_fibrealignment_" + gt + "_iov_" + to_string(iov);
      payload pl; pl.fHash = hash; pl.fComment = annotation; pl.fSchema = cfa->getSchema(); pl.fBLOB = spl;
      cfa->writePayloadToFile(hash, payloaddir, pl);
    }
    if (string::npos != ifilename.find(".root")) { 
      cout << "   ->cdbWritePayload> removing temporary file " << tmpFilename << endl; 
      remove(tmpFilename.c_str()); 
    }
    delete cfa;
  }
  if (string::npos == ifilename.find(".root")) { 
    cout << "->cdbWritePayload> removing temporary file " << tmpFilename << endl; 
    remove(tmpFilename.c_str()); 
  }
}

// ----------------------------------------------------------------------
void cdbPayloadWriter::run(int argc, const char* argv[]) {
  string cal(""), payloaddir("."), inputfiledir(""), annotation(""), gt(""), filename("");
  int iov(1);
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-a"))  {annotation = argv[++i];}
    if (!strcmp(argv[i], "-c"))  {cal = argv[++i];}
    if (!strcmp(argv[i], "-d"))  {inputfiledir = argv[++i];}
    if (!strcmp(argv[i], "-i"))  {iov = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-g"))  {gt = argv[++i];}
    if (!strcmp(argv[i], "-f"))  {filename = argv[++i];}
    if (!strcmp(argv[i], "-p"))  {payloaddir = argv[++i];}
  }
  
  cout << "======================" << endl;
  cout << "== cdbPayloadWriter ==" << endl;
  cout << "======================" << endl;
  cout << "== writing payload " << cal << " for global tag " << gt << endl;
  cout << "== installing in directory " << payloaddir << endl;
  cout << "== filename " << filename << endl;
  cout << "== iov " << iov << endl;
  cout << "== annotation " << annotation << endl << endl;
  
  if (inputfiledir != "") {
    vector<string> vfiles;
    DIR *folder = opendir(inputfiledir.c_str());
    if (folder == NULL) {
      cout << "Error failed to open " << inputfiledir << endl;
      return;
    }
    struct dirent *entry;
    while ((entry = readdir(folder))) {
      if (entry->d_type == DT_REG) {
        if ((cal == "eventstuffv1") && (string::npos == string(entry->d_name).find(".mid.lz4.json"))) continue;
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
      writeEventStuffV1Payloads(payloaddir, gt, filename, annotation, irun);
    }
    return;
  }
  
  if (string::npos != cal.find("alignment")) {
    writeAlignmentPayloads(payloaddir, gt, cal, filename, annotation, iov);
  }
  if ("alignment" == cal) {
    writeAlignmentPayloads(payloaddir, gt, "pixelalignment", filename, annotation, iov);
    writeAlignmentPayloads(payloaddir, gt, "tilealignment", filename, annotation, iov);
    writeAlignmentPayloads(payloaddir, gt, "fibrealignment", filename, annotation, iov);
    writeAlignmentPayloads(payloaddir, gt, "mppcalignment", filename, annotation, iov);
  }
  if (string::npos != cal.find("pixelqualitylm")) {
    writePixelQualityLMPayloads(payloaddir, gt, filename, annotation, iov);
  }
  if (string::npos != cal.find("fibrequality")) {
    writeFibreQualityPayloads(payloaddir, gt, filename, annotation, iov);
  }
  if (string::npos != cal.find("tilequality")) {
    writeTileQualityPayloads(payloaddir, gt, filename, annotation, iov);
  }
  if (string::npos != cal.find("pixelefficiency")) {
    writePixelEfficiencyPayloads(payloaddir, gt, filename, annotation, iov);
  }
  if (string::npos != cal.find("eventstuffv1")) {
    writeEventStuffV1Payloads(payloaddir, gt, filename, annotation, iov);
  }
}


// ----------------------------------------------------------------------
void cdbPayloadWriter::createChipIDsPerLayer(string inputfilename) {
  cout << "   ->cdbPayloadWriter> creating chip IDs per layer from input file " << inputfilename << endl;
  vector<unsigned int> Layer1ChipIDs, Layer2ChipIDs, Layer3ChipIDs, Layer4ChipIDs;

  if (string::npos != inputfilename.find(".csv")) {
    ifstream IN;
    IN.open(inputfilename);
    string line;
    while (getline(IN, line)) {
      stringstream ss(line);
      string id;
      ss >> id;
      unsigned int ChipID = ::stoi(id);
      int layer = (ChipID/1024)%4 + 1;
      if (layer == 1) {
        Layer1ChipIDs.push_back(ChipID);
      } else if (layer == 2) {
        Layer2ChipIDs.push_back(ChipID);
      } else if (layer == 3) {
        Layer3ChipIDs.push_back(ChipID);
      } else if (layer == 4) {
        Layer4ChipIDs.push_back(ChipID);
      }
    }
    IN.close();
  } else if (string::npos != inputfilename.find(".root")) {
    TFile *file = TFile::Open(inputfilename.c_str());
    TTree *ta = static_cast<TTree*>(file->Get("alignment/sensors"));
    unsigned int id;
    ta->SetBranchAddress("id", &id);
    for (int i = 0; i < ta->GetEntries(); ++i) {
      ta->GetEntry(i);
      int layer = (id/1024)%4 + 1;
      if (layer == 1) {
        Layer1ChipIDs.push_back(id);
      } else if (layer == 2) {
        Layer2ChipIDs.push_back(id);
      } else if (layer == 3) {
        Layer3ChipIDs.push_back(id);
      } else if (layer == 4) {
        Layer4ChipIDs.push_back(id);
      }
    }
    file->Close();
  } else {
    cout << "Error: input filename " << inputfilename << " is not a .csv or .root file" << endl;
    return;
  } 

  cout << "   ->cdbPayloadWriter> read " << Layer1ChipIDs.size() << " layer 1 chip IDs" << endl;
  int cnt(0);
  for (auto &id : Layer1ChipIDs) {
    cout << id << ", ";
    cnt++;
    if ((cnt % 12) == 0) cout << endl;
  }
  cout << endl;

  cout << "   ->cdbPayloadWriter> read " << Layer2ChipIDs.size() << " layer 2 chip IDs" << endl;
  cnt = 0; 
  for (auto &id : Layer2ChipIDs) {
    cout << id << ", ";
    cnt++;
    if ((cnt % 12) == 0) cout << endl;
  }
  cout << endl;

  cout << "   ->cdbPayloadWriter> read " << Layer3ChipIDs.size() << " layer 3 chip IDs" << endl;
  cnt = 0;
  for (auto &id : Layer3ChipIDs) {
    cout << id << ", ";
    cnt++;
    if ((cnt % 17) == 0) cout << endl;
  }
  cout << endl;

  cout << "   ->cdbPayloadWriter> read " << Layer4ChipIDs.size() << " layer 4 chip IDs" << endl;
  cnt = 0;
  for (auto &id : Layer4ChipIDs) {
    cout << id << ", ";
    cnt++;
    if ((cnt % 18) == 0) cout << endl;
  }
  cout << endl;

  cout << "   ->cdbPayloadWriter> read " << Layer1ChipIDs.size() + Layer2ChipIDs.size() + Layer3ChipIDs.size() + Layer4ChipIDs.size() << " total chip IDs" << endl;
}