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
#include "cdbPayloadWriter.hh"

#include "calPixelAlignment.hh"
#include "calFibreAlignment.hh"
#include "calMppcAlignment.hh"
#include "calTileAlignment.hh"
#include "calPixelCablingMap.hh"
#include "calPixelQualityLM.hh"

#include "calDetConfV1.hh" // decrepit!
#include "calDetSetupV1.hh"

using namespace std;

// Forward declarations
void writeDetSetupV1(string jsondir, string gt);
void writePixelQualityLM(string jsondir, string gt, string payloaddir);
void writeAlignmentInformation(string jsondir, string gt, string alignmentTag);
void writeInitialTag(string jsondir, string gt, string initialTag, string comment = "");
void insertIovTag(const std::string &jsondir, const std::string &tag, int insertRun /*-i*/, int removeRun /*-r*/, bool clear /*-c*/);

// ----------------------------------------------------------------------
// cdbInitGT 
// ---------
// The primary purpose is mcideal initialization (and to provide the ideal starting point for data GT?)
//
// Usage examples
// --------------
//
// -- create a global tag with all tags/payloads 
// merlin> ./bin/cdbInitGT -g mcidealv6.5 -j ~/data/mu3e/cdb 
//
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
int main(int argc, const char* argv[]) {
  
  // ----------------------------------------------------------------------
  // -- global tags
  // ----------------------------------------------------------------------
  map<string, vector<string>> iniGlobalTags = {
     {"mcidealv6.5", {
      "pixelalignment_", 
      "fibrealignment_", 
      "tilealignment_", 
      "mppcalignment_", 
      "pixelqualitylm_ideal", 
      "fibrequality_ideal", 
      "tilequality_ideal", 
      "pixelefficiency_ideal", 
      "pixeltimecalibration_ideal",
      "eventstuffv1_ideal",
      "detsetupv1_"} 
    },
    {"datav6.2=2025Beam", {
      "pixelalignment_mcidealv6.1=2025CosmicsVtxOnly", 
      "fibrealignment_mcidealv6.1", 
      "tilealignment_mcidealv6.1", 
      "mppcalignment_mcidealv6.1", 
      "pixelqualitylm-datav6.1=2025CosmicsVtxOnly", 
      "detsetupv1_mcidealv6.1"} 
    },
    {"datav6.2=2025CosmicsNoMagnet", {
      "pixelalignment_mcidealv6.1=2025CosmicsVtxOnly", 
      "fibrealignment_mcidealv6.1", 
      "tilealignment_mcidealv6.1", 
      "mppcalignment_mcidealv6.1", 
      "pixelqualitylm-datav6.1=2025CosmicsVtxOnly", 
      "detsetupv1_mcidealv6.1=2025CosmicsVtxOnly"} 
    }
    
  };
  
  // -- comments for global tags (optional)
  map<string, string> gtComments = {
    {"mcidealv6.5", "MC ideal (=complete) detector geometry v6.5. No deficiencies, all 100% efficient. No time-walk corrections (zero shift and uncertainty)."},
    {"datav6.2=2025Beam", "Data tag for 2025 beam runs with magnetic field. Pixel 2-layer (VTX), fibres and tiles complete detector. Ideal detector geometry based on v6.1."},
    {"datav6.2=2025CosmicsNoMagnet", "Data tag for cosmics without magnet. Pixel 2-layer (VTX), fibres and tiles complete detector. Ideal detector geometry based on v6.1."}
  };
  
  // -- comments for tags (optional)
  map<string, string> tagComments = {
    {"pixelalignment_mcidealv6.5", "Ideal detector geometry with pixel alignment with MC truth from v6.5."},
    {"tilealignment_mcidealv6.5", "Ideal detector geometry with tile alignment with MC truth from v6.5."},
    {"fibrealignment_mcidealv6.5", "Ideal detector geometry with fibre alignment with MC truth from v6.5."},
    {"mppcalignment_mcidealv6.5", "Ideal detector geometry with mppc alignment with MC truth from v6.5."},
    {"pixeltimecalibration_ideal", "Pixel time calibration for the entire detector with zero shifts and uncertainties, i.e. no time walk corrections for ideal MC."},
    {"pixelqualitylm_ideal", "Perfect pixel detector with no deficiencies."},
    {"fibrequality_ideal", "Perfect fibre detector with no deficiencies."},
    {"tilequality_ideal", "Perfect tile detector with no deficiencies."},
    {"pixelefficiency_ideal", "Fully efficient for complete pixel detector."},
    {"eventstuffv1_ideal", "No limitations to time stamps."},
    {"detsetupv1_ideal", "Magnet turned on."},
    {"pixelalignment_mcidealv6.1=2025CosmicsVtxOnly", "Ideal detector geometry with 2-layer pixel (VTX)."},
    {"pixelqualitylm-datav6.1=2025CosmicsVtxOnly", "Perfect 2-layer pixel detector (VTX)with no deficiencies."},
    {"fibrealignment_mcidealv6.1", "Ideal complete fibre detector geometry based on v6.1."},
    {"tilealignment_mcidealv6.1", "Ideal complete tile detector geometry based on v6.1."},
    {"mppcalignment_mcidealv6.1", "Ideal complete mppc detector geometry based on v6.1."}
  };
  
  // // -- complete the tags by replacing trailing _ with the _GT
  for (auto &it: iniGlobalTags) {
    for (unsigned int i = 0; i < it.second.size(); i++) { 
      if (it.second[i].back() == '_') {
        it.second[i] = it.second[i] + it.first;
      }
    }
  } 
  
  // -- command line arguments
  string jsondir("");
  string gt("");
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-j"))  {jsondir    = argv[++i];}
    if (!strcmp(argv[i], "-g"))  {gt         = argv[++i];}
  }
  
  cout << "===============" << endl;
  cout << "== cdbInitGT ==" << endl;
  cout << "===============" << endl;
  cout << "== installing in directory " << jsondir << endl;
  cout << "== global tag " << gt << endl;
  cout << "== json directory " << jsondir << endl;
  
  // -- check whether directories for JSONs already exist
  vector<string> testdirs{jsondir,
    jsondir + "/globaltags",
    jsondir + "/tags",
    jsondir + "/payloads",
    jsondir + "/runrecords",
    jsondir + "/configs",
  };
  
  for (auto it: testdirs) {
    DIR *folder = opendir(it.c_str());
    if (folder == NULL) {
      cout << "creating " << it << endl;
      system(string("mkdir -p " + it).c_str());
    } else {
      closedir(folder);
    }
  }
  
  ofstream JS;
  
  string jdir  = jsondir + "/globaltags";
  
  for (auto igt : iniGlobalTags) {
    if (gt != "all" && igt.first != gt) continue;
    
    vector<string> arrayBuilder;
    for (auto it : igt.second) {
      string tag = ('_' == it.back()? it + igt.first: it);
      arrayBuilder.push_back(tag);
    }
    stringstream sstr;
    sstr << "{ \"gt\" : \"" << igt.first << "\", \"tags\" : ";
    sstr << jsFormat(arrayBuilder);
    auto itComment = gtComments.find(igt.first);
    if (itComment != gtComments.end() && !itComment->second.empty()) {
      sstr << ", \"comment\" : \"" << escapeJsonString(itComment->second) << "\"";
    }
    sstr << " }" << endl;
    
    
    // -- JSON
    JS.open(jdir + "/" + igt.first);
    if (JS.fail()) {
      cout << "Error failed to open " << jdir << "/" << igt.first << endl;
    }
    JS << sstr.str();
    cout << sstr.str();
    JS.close();
  }
  
  cdbPayloadWriter writer;
  string payloaddir = jsondir + "/payloads";
  
  string gtall = (gt == "all"? "all": "unset"); 
  string tagLabel = gt; 
  for (auto it : iniGlobalTags) {
    if (gtall == "all") gt = it.first;
    if (gt != it.first) continue;
    cout << "************* processing global tag " << it.first << " and gt = " << gt << endl;
    for (auto it2 : it.second) {
      tagLabel = it2.substr(it2.rfind('_') + 1);
      cout << "tagLabel = " << tagLabel << " it2 = " << it2 << " it.first = " << it.first << endl;
      
      bool RF(false); // RF = rootfile
      string asciiFilename("");
      if (string::npos != it.first.find("mcideal")) {
        RF = true;
      }
      
      if (string::npos != it2.find("pixelalignment_")) {
        if (RF) {
          asciiFilename = string(LOCALDIR) + "/ascii/mu3e_alignment_" + gt + ".root";
        } else {
          asciiFilename = string(LOCALDIR) + "/ascii/sensors-" + tagLabel + ".csv";
        }
        writer.writeAlignmentPayloads(payloaddir, tagLabel, it2, asciiFilename, tagComments[it2], 1);
        writeInitialTag(jsondir, tagLabel, it2, tagComments[it2]);
      }
      
      if (string::npos != it2.find("tilealignment_")) {
        if (RF) {
          asciiFilename = string(LOCALDIR) + "/ascii/mu3e_alignment_" + gt + ".root";
        } else {
          asciiFilename = string(LOCALDIR) + "/ascii/tiles-" + tagLabel + ".csv";
        }
        writer.writeAlignmentPayloads(payloaddir, tagLabel, it2, asciiFilename, tagComments[it2], 1);
        writeInitialTag(jsondir, tagLabel, it2, tagComments[it2]);
      }
      
      if (string::npos != it2.find("fibrealignment_")) {
        if (RF) {
          asciiFilename = string(LOCALDIR) + "/ascii/mu3e_alignment_" + gt + ".root";
        } else {
          asciiFilename = string(LOCALDIR) + "/ascii/fibres-" + tagLabel + ".csv";
        }
        writer.writeAlignmentPayloads(payloaddir, tagLabel, it2, asciiFilename, tagComments[it2], 1);
        writeInitialTag(jsondir, tagLabel, it2, tagComments[it2]);
      }
      
      if (string::npos != it2.find("mppcalignment_")) {
        if (RF) {
          asciiFilename = string(LOCALDIR) + "/ascii/mu3e_alignment_" + gt + ".root";
        } else {
          asciiFilename = string(LOCALDIR) + "/ascii/mppcs-" + tagLabel + ".csv";
        }
        writer.writeAlignmentPayloads(payloaddir, tagLabel, it2, asciiFilename, tagComments[it2], 1);
        writeInitialTag(jsondir, tagLabel, it2, tagComments[it2]);
      }
      
      if (string::npos != it2.find("pixelqualitylm_")) {
        if (RF) {
          asciiFilename = string(LOCALDIR) + "/ascii/mu3e_alignment_" + gt + ".root";
        } else {
          asciiFilename = string(LOCALDIR) + "/ascii/pixelqualitylm-" + tagLabel + ".csv";
        }
        writer.writePixelQualityLMPayloads(payloaddir, tagLabel, asciiFilename, tagComments[it2], 1);
        writeInitialTag(jsondir, tagLabel, it2, tagComments[it2]);
      }
      
      if (string::npos != it2.find("fibrequality_")) {  
        writer.writeFibreQualityPayloads(payloaddir, tagLabel, string(LOCALDIR) + "/ascii/fibre-asics-perfect.csv", tagComments[it2], 1);
        writeInitialTag(jsondir, tagLabel, it2, tagComments[it2]);
      }
      
      if (string::npos != it2.find("tilequality_")) {
        writer.writeTileQualityPayloads(payloaddir, tagLabel, string(LOCALDIR) + "/ascii/tile-quality-perfect.json", tagComments[it2], 1);
        writeInitialTag(jsondir, tagLabel, it2, tagComments[it2]);
      }
      
      if (string::npos != it2.find("detsetupv1_")) {
        writer.writeDetSetupV1Payloads(payloaddir, tagLabel, string(LOCALDIR) + "/ascii/detector-MagnetOff-v6.5.json", tagComments[it2], 1);
        writer.writeDetSetupV1Payloads(payloaddir, tagLabel, string(LOCALDIR) + "/ascii/detector-MagnetOn-v6.5.json", tagComments[it2], 1);
        writeInitialTag(jsondir, tagLabel, it2, tagComments[it2]);
      }
      
      if (string::npos != it2.find("eventstuffv1_")) {
        writer.writeEventStuffV1Payloads(payloaddir, tagLabel, string(LOCALDIR) + "/ascii/eventstuff-ideal.json", tagComments[it2], 1);
        writeInitialTag(jsondir, tagLabel, it2, tagComments[it2]);
      }
      
      if (string::npos != it2.find("pixelefficiency_")) {
        if (RF) {
          asciiFilename = string(LOCALDIR) + "/ascii/mu3e_alignment_" + gt + ".root";
        } else {
          asciiFilename = string(LOCALDIR) + "/ascii/pixelefficiency-" + tagLabel + ".csv";
        }
        writer.writePixelEfficiencyPayloads(payloaddir, tagLabel, asciiFilename, tagComments[it2], 1);
        writeInitialTag(jsondir, tagLabel, it2, tagComments[it2]);
      }

      if (string::npos != it2.find("pixeltimecalibration_")) {
        writer.writePixelTimeCalibrationPayloads(payloaddir, tagLabel, string(LOCALDIR) + "/ascii/largecalib-ideal.calib", tagComments[it2], 1);
        writeInitialTag(jsondir, tagLabel, it2, tagComments[it2]);
      }

    }
  }  
  
  return 0;
}

// ----------------------------------------------------------------------
void writeDetSetupV1(string jsondir, string gt) {
  cout << "   ->cdbInitGT> writing local template detsetupv1 payloads" << endl;
  // -- create (local template) payloads for no field and with field
  calDetSetupV1 *cdc = new calDetSetupV1();
  string filename = string(LOCALDIR) + "/ascii/detector-MagnetOff-v6.2.json";
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
  
  filename = string(LOCALDIR) + "/ascii/detector-MagnetOn-v6.2.json";
  result = cdc->readJSON(filename);
  if (string::npos == result.find("Error")) {
    string spl = cdc->makeBLOB();
    string hash = "detsetupv1_MagnetOn";
    payload pl;
    if (fileExists(string(LOCALDIR) + "/" + hash)) {
      cout << "   ->cdbInitGT> payload " << hash << " already exists, skipping" << endl;
    } else {
      pl.fHash = hash;
      pl.fComment = "detector setup with magnet on";
      pl.fSchema  = cdc->getSchema();
      pl.fBLOB = spl;
      cdc->writePayloadToFile(hash, string(LOCALDIR), pl);
    }
  }
  
  // -- now the payloads
  vector<pair<int, int>> iovMagnet = { {1, 0}, {2177, 1}, {6302, 0}};
  vector<int> iovs;
  for (auto it: iovMagnet) {
    string templateHash = "detsetupv1_";
    string hash = "tag_" + templateHash + gt + "_iov_" + to_string(it.first);
    iovs.push_back(it.first);
    if (it.second == 1) {
      templateHash = templateHash + "MagnetOn";
    } else {
      templateHash = templateHash + "noField";
    }
    cdc->readPayloadFromFile(templateHash, string(LOCALDIR));
    payload pl = cdc->getPayload(templateHash);
    pl.fSchema = cdc->getSchema();
    pl.fHash = hash;
    cdc->writePayloadToFile(hash, jsondir + "/payloads", pl);
    cout << "   ->cdbInitGT> writing IOV " << it.first << " with " << templateHash << endl;
  }
  
  // -- and the tag/IOVs
  string tag = "detsetupv1_" + gt;
  stringstream sstr;
  sstr << "  { \"tag\" : \"" << tag << "\", \"iovs\" : ";
  sstr << jsFormat(iovs);
  sstr << ", \"comment\" : \"detector setup\" }" << endl;
  cout << sstr.str();
  ofstream ONS;
  ONS.open(jsondir + "/tags/" + tag);
  if (ONS.fail()) {
    cout << "Error failed to open " << jsondir + "/tags/" + tag << endl;
  }
  ONS << sstr.str();
  cout << sstr.str();
  ONS.close();
  
  delete cdc;
}

// ----------------------------------------------------------------------
void writePixelQualityLM(string jsondir, string gt, string payloaddir) {
  cout << "   ->cdbInitGT> writing local template pixelqualitylm payloads" << endl;
  // -- create (local template) payloads for no problematic pixels
  calPixelQualityLM *cpq = new calPixelQualityLM();
  string filename = string(LOCALDIR) + "/ascii/pixelqualitylm-" + gt + ".csv";
  cpq->readCsv(filename);
  string spl = cpq->makeBLOB();
  string hash = "tag_pixelqualitylm_" + gt + "_iov_1";
  payload pl;
  if (fileExists(jsondir + "/" + hash)) {
    cout << "   ->cdbInitGT> payload " << hash << " already exists, skipping" << endl;
  }
  pl.fHash = hash;
  pl.fComment = "pixelqualitylm with no problematic pixels";
  pl.fSchema  = cpq->getSchema();
  pl.fBLOB = spl;
  cpq->writePayloadToFile(hash, jsondir + "/payloads", pl);
  
  // -- and the tag/IOVs
  string tag = "pixelqualitylm_" + gt;
  stringstream sstr;
  sstr << "  { \"tag\" : \"" << tag << "\", \"iovs\" : ";
  vector<int> iovs{1};
  sstr << jsFormat(iovs);
  sstr << ", \"comment\" : \"pixelqualitylm with no problematic pixels\" }" << endl;
  cout << sstr.str();
  ofstream ONS;
  ONS.open(jsondir + "/tags/" + tag);
  if (ONS.fail()) {
    cout << "Error failed to open " << jsondir + "/tags/" + tag << endl;
  }
  ONS << sstr.str();
  cout << sstr.str();
  ONS.close();
  
  
  // -- now the other payloads
  glob_t globbuf;
  int err = glob((payloaddir + "/tag_pixelqualitylm_*").c_str(), 0, NULL, &globbuf);
  map<int, string> mRunToHash;
  if (err == 0)  {
    for (size_t i = 0; i < globbuf.gl_pathc; i++)  {
      cout << "   ->cdbInitGT> processing file: " << globbuf.gl_pathv[i] << endl;
      string hash = globbuf.gl_pathv[i];
      hash = hash.substr(hash.rfind('/') + 1);
      string run = hash.substr(hash.rfind('_') + 1);
      int irun = ::stoi(run);
      // -- skip run 1 (because that is created above)
      if (irun == 1) continue;
      // -- check if the run already exists in the map with the correct hash (containing the gt)
      if ((mRunToHash.find(irun) != mRunToHash.end()) 
      && (string::npos != mRunToHash[irun].find(gt))) {
        cout << "   ->cdbInitGT> run " << irun 
        << " already exists with " << mRunToHash[irun] 
        << ", skipping" 
        << endl;
        continue;
      }
      mRunToHash[::stoi(run)] = hash;
    }
    globfree(&globbuf);
  }
  for (auto it: mRunToHash) {
    cout << it.first << " -> " << it.second << " ";
    cpq->readPayloadFromFile(it.second, payloaddir);
    payload pl = cpq->getPayload(it.second);
    string hash = "tag_pixelqualitylm_" + gt + "_iov_" + to_string(it.first);
    pl.fHash = hash;
    pl.fComment = "source: " + payloaddir + "/" + it.second;
    pl.fSchema  = cpq->getSchema();
    pl.fBLOB = pl.fBLOB;
    cout << " writing payload " << hash << " for run " << it.first << endl;
    cpq->writePayloadToFile(hash, jsondir + "/payloads", pl);
    insertIovTag(jsondir, "pixelqualitylm_" + gt, it.first, 0, false);
  }
  delete cpq;
}


// ----------------------------------------------------------------------
void writeAlignmentInformation(string jsondir, string gt, string alignmentTag) {
  cout << "   ->cdbInitGT> writing alignment payloads" << endl;
  // -- pixel sensor alignment
  calPixelAlignment *cpa = new calPixelAlignment();
  string filename = string(LOCALDIR) + "/ascii/sensors-" + alignmentTag + ".csv";
  cout << "   ->cdbInitGT> reading sensor alignment from " << filename << endl;
  string result = cpa->readCsv(filename);
  if (string::npos == result.find("Error")) {
    string spl = cpa->makeBLOB();
    string hash = "tag_pixelalignment_" + gt + "_iov_1";
    payload pl;
    if (fileExists(jsondir + "/" + hash)) {
      cout << "   ->cdbInitGT> payload " << hash << " already exists, skipping" << endl;
    } else {
      pl.fHash = hash;
      pl.fComment = "pixel alignment";
      pl.fSchema  = cpa->getSchema();
      pl.fBLOB = spl;
      cpa->writePayloadToFile(hash, jsondir + "/payloads", pl);
    }
  }
  writeInitialTag(jsondir, gt, "pixelalignment_", "pixel alignment");
  
  
  // -- tile sensor alignment
  calTileAlignment *cta = new calTileAlignment();
  filename = string(LOCALDIR) + "/ascii/tiles-" + alignmentTag + ".csv";
  cout << "   ->cdbInitGT> reading tile alignment from " << filename << endl;
  result = cta->readCsv(filename);
  if (string::npos == result.find("Error")) {
    string spl = cta->makeBLOB();
    string hash = "tag_tilealignment_" + gt + "_iov_1";
    payload pl;
    if (fileExists(jsondir + "/" + hash)) {
      cout << "   ->cdbInitGT> payload " << hash << " already exists, skipping" << endl;
    } else {
      pl.fHash = hash;
      pl.fComment = "tile alignment";
      pl.fSchema  = cta->getSchema();
      pl.fBLOB = spl;
      cta->writePayloadToFile(hash, jsondir + "/payloads", pl);
    }
  }
  writeInitialTag(jsondir, gt, "tilealignment_", "tile alignment");
  
  // -- fiber sensor alignment
  calFibreAlignment *cfa = new calFibreAlignment();
  filename = string(LOCALDIR) + "/ascii/fibres-" + alignmentTag + ".csv";
  cout << "   ->cdbInitGT> reading fibre alignment from " << filename << endl;
  result = cfa->readCsv(filename);
  if (string::npos == result.find("Error")) {
    string spl = cfa->makeBLOB();
    string hash = "tag_fibrealignment_" + gt + "_iov_1";
    payload pl;
    if (fileExists(jsondir + "/" + hash)) {
      cout << "   ->cdbInitGT> payload " << hash << " already exists, skipping" << endl;
    } else {
      pl.fHash = hash;
      pl.fComment = "fibre alignment";
      pl.fSchema  = cfa->getSchema();
      pl.fBLOB = spl;
      cfa->writePayloadToFile(hash, jsondir + "/payloads", pl);
    }
  }
  writeInitialTag(jsondir, gt, "fibrealignment_", "fibre alignment");
  
  // -- mppc sensor alignment
  calMppcAlignment *cma = new calMppcAlignment();
  filename = string(LOCALDIR) + "/ascii/mppcs-" + alignmentTag + ".csv";
  cout << "   ->cdbInitGT> reading mppc alignment from " << filename << endl;
  result = cma->readCsv(filename);
  if (string::npos == result.find("Error")) {
    string spl = cma->makeBLOB();
    string hash = "tag_mppcalignment_" + gt + "_iov_1";
    payload pl;
    if (fileExists(jsondir + "/" + hash)) {
      cout << "   ->cdbInitGT> payload " << hash << " already exists, skipping" << endl;
    } else {
      pl.fHash = hash;
      pl.fComment = "mppc alignment";
      pl.fSchema  = cma->getSchema();
      pl.fBLOB = spl;
      cma->writePayloadToFile(hash, jsondir + "/payloads", pl);
    }
  }
  writeInitialTag(jsondir, gt, "mppcalignment_", "mppc alignment");
}


// ----------------------------------------------------------------------
void writeInitialTag(string jsondir, string gt, string initialTag, string comment) {
  // -- and the tag/IOVs
  string tag = initialTag;// + gt;
  stringstream sstr;
  sstr << "  { \"tag\" : \"" << tag << "\", \"iovs\" : ";
  vector<int> iovs{1};
  sstr << jsFormat(iovs);
  string lcomment = "Created for GT=" + gt;
  if (!comment.empty()) {
    sstr << ", \"comment\" : \"" << escapeJsonString(comment + " " + lcomment) << "\"";
  } else {
    sstr << ", \"comment\" : \"" << escapeJsonString(lcomment) << "\"";
  }
  sstr << " }" << endl;
  cout << sstr.str();
  ofstream ONS;
  ONS.open(jsondir + "/tags/" + tag);
  if (ONS.fail()) {
    cout << "Error failed to open " << jsondir + "/tags/" + tag << endl;
  }
  ONS << sstr.str();
  cout << sstr.str();
  ONS.close();
}

// ----------------------------------------------------------------------
// Translated from  run2025/scripts/insertIovTag into C++
// Reads jsondir/tags/<tag>, updates the IOV list by inserting (-i) or
// removing (-r) a run, or clears to single '1' when clear (-c) is true.
// Writes a .bac backup and rewrites the file in compact JSON.
void insertIovTag(const std::string &jsondir, const std::string &tag, int insertRun, int removeRun, bool clear) {
  const std::string file = jsondir + "/tags/" + tag;
  
  // Read whole file
  std::ifstream in(file);
  if (!in) {
    std::cerr << "insertIovTag: Cannot open ->" << file << "<-" << std::endl;
    return;
  }
  std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  in.close();
  
  // Extract optional comment before normalizing
  std::string comment;
  std::string commentVal = jsonGetString(content, "comment");
  if (!commentVal.empty() && commentVal.find("parseError") == std::string::npos) {
    comment = commentVal;
  }
  
  // Remove spaces and newlines to simplify parsing
  content.erase(std::remove(content.begin(), content.end(), '\n'), content.end());
  content.erase(std::remove(content.begin(), content.end(), ' '), content.end());
  
  // Find the iovs array between '[' and ']'
  std::vector<int> runs;
  size_t lbr = content.find("[");
  size_t rbr = content.find("]", lbr == std::string::npos ? 0 : lbr + 1);
  if (lbr != std::string::npos && rbr != std::string::npos && rbr > lbr) {
    std::string arr = content.substr(lbr + 1, rbr - lbr - 1);
    std::stringstream ss(arr);
    std::string tok;
    while (std::getline(ss, tok, ',')) {
      if (!tok.empty()) {
        try { runs.push_back(std::stoi(tok)); } catch (...) {}
      }
    }
  }
  
  // Modify runs per options
  if (removeRun > 0) {
    std::vector<int> filtered;
    filtered.reserve(runs.size());
    for (int r : runs) if (r != removeRun) filtered.push_back(r);
    runs.swap(filtered);
  } else if (insertRun > 0) {
    // Insert keeping ascending order and unique
    bool inserted = false;
    for (auto it = runs.begin(); it != runs.end(); ++it) {
      if (*it == insertRun) { inserted = true; break; }
      if (insertRun < *it) { runs.insert(it, insertRun); inserted = true; break; }
    }
    if (!inserted) runs.push_back(insertRun);
  }
  
  // Backup existing file
  {
    std::ifstream src(file, std::ios::binary);
    std::ofstream dst(file + ".bac", std::ios::binary);
    if (src && dst) dst << src.rdbuf();
  }
  
  // Write back
  std::ofstream out(file);
  if (!out) {
    std::cerr << "insertIovTag: Cannot open " << file << " for output" << std::endl;
    return;
  }
  out << "{\"tag\":\"" << tag << "\", \"iovs\": [";
  if (clear) {
    out << 1;
  } else {
    for (size_t i = 0; i < runs.size(); ++i) {
      out << runs[i];
      if (i + 1 < runs.size()) out << ", ";
    }
  }
  out << "]";
  if (!comment.empty()) {
    out << ", \"comment\":\"" << escapeJsonString(comment) << "\"";
  }
  out << "}\n";
  out.close();
  
  // Optional: log
  std::cout << "insertIovTag: " << tag << ": ";
  for (size_t i = 0; i < runs.size(); ++i) {
    std::cout << runs[i] << (i + 1 < runs.size() ? " " : "\n");
  }
}
