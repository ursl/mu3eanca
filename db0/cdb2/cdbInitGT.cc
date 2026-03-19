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

void writeInitialTag(string jsondir, string gt, string initialTag, string comment = "");
void insertIovTag(const std::string &jsondir, const std::string &tag, int insertRun /*-i*/, int removeRun /*-r*/, bool clear /*-c*/);

// ----------------------------------------------------------------------
// cdbInitGT 
// ---------
// The primary purpose is mcideal initialization (and to provide the ideal starting point for data GT?)
// Note: The initial payload for ANY GT should be perfect conditions. realistic starts for specific runs (determined in data analysis)
//       However, the geometric detector contents varies
//         - mcidealv6.5 with complete detector
//         - mcidealv6.5=2025 with VTX and complete(?) fibres and DS tiles
//         - mcidealv6.5=central3 with 3-layer central pixel 
//         - mcrealisticv6.5=2025 with VTX and complete(?) fibres and tiles, eventually smeared calibrations
//         - datav6.5=2025 
//
// Usage examples
// --------------
//
// -- create a global tag with all tags/payloads 
// merlin> ./bin/cdbInitGT -g mcidealv6.5 -j ~/data/mu3e/cdb 
//
// ----------------------------------------------------------------------

struct structGT {
  string gt;
  vector<string> tags;
  string comment;
  string rootfile;
};



// ----------------------------------------------------------------------
int main(int argc, const char* argv[]) {
  
  // ----------------------------------------------------------------------
  // -- global tags
  // ----------------------------------------------------------------------
  map<string, structGT> iniStructGT = {
    // -- MC ideal
    {"mcidealv6.5", {
      .gt = "mcidealv6.5",
      .tags = {"pixelalignment_", "fibrealignment_", "tilealignment_", "mppcalignment_", 
               "pixelqualitylm_", "fibrequality_ideal", "tilequality_ideal", 
               "pixelefficiency_ideal", "pixeltimecalibration_", 
               "eventstuffv1_ideal", "detsetupv1_"},
      .comment = "MC ideal (=complete) detector geometry v6.5. No deficiencies, all 100% efficient. No time-walk corrections (zero shift and uncertainty).",
      .rootfile = string(LOCALDIR) + "/ascii/mu3e_alignment_mcidealv6.5.root"
    }},
    {"mcidealv6.5=2025", {
      .gt = "mcidealv6.5=2025",
      .tags = {"pixelalignment_", "fibrealignment_", "tilealignment_", "mppcalignment_", 
               "pixelqualitylm_", "fibrequality_", "tilequality_", 
               "pixelefficiency_", "pixeltimecalibration_", 
               "eventstuffv1_ideal", "detsetupv1_"},
      .comment = "MC ideal 2025 detector geometry v6.5 (VTX + DS tiles + all fibres). No deficiencies, all 100% efficient. No time-walk corrections (zero shift and uncertainty).",
      .rootfile = string(LOCALDIR) + "/ascii/mu3e_alignment_mcidealv6.5.root"
    }},
    {"mcidealv6.5=2026", {
      .gt = "mcidealv6.5=2026",
      .tags = {"pixelalignment_", "fibrealignment_", "tilealignment_", "mppcalignment_", 
               "pixelqualitylm_", "fibrequality_", "tilequality_", 
               "pixelefficiency_", "pixeltimecalibration_", 
               "eventstuffv1_ideal", "detsetupv1_"},
      .comment = "MC ideal 2026 detector geometry v6.5 (VTX + central L3 + DS tiles + all fibres). No deficiencies, all 100% efficient. No time-walk corrections (zero shift and uncertainty).",
      .rootfile = string(LOCALDIR) + "/ascii/mu3e_alignment_mcidealv6.5.root"
    }},
    // -- MC realistic
    {"mcrealisticv6.5=2025V0", {
      .gt = "mcrealisticv6.5=2025V0",
      .tags = {"pixelalignment_mcidealv6.5=2025", "fibrealignment_mcidealv6.5=2025", "tilealignment_mcidealv6.5=2025", "mppcalignment_mcidealv6.5=2025", 
               "pixelqualitylm_datav6.5=2025V0", "fibrequality_datav6.3=2025V0", "tilequality_datav6.3=2025V0", 
               "pixelefficiency_datav6.5=2025V0", "pixeltimecalibration_", 
               "eventstuffv1_ideal", "detsetupv1_"},
      .comment = "MC realistic 2025 detector conditions with *placeholder* MC smearing. Ideal 2025 geometries using MC truth from v6.5 (VTX + DS tiles + all fibres). Starting point for analysis of MC simulation data.",
      .rootfile = string(LOCALDIR) + "/ascii/mu3e_alignment_mcidealv6.5.root"
    }}

  };
  
  // -- comments for tags. Since tags can be part of multiple GT, do not insert this into iniStructGT
  map<string, string> tagComments = {
    {"pixelalignment_mcidealv6.5", "Ideal detector geometry with pixel alignment using MC truth from v6.5."},
    {"pixelalignment_mcidealv6.5=2025", "Ideal detector geometry (VTX and complete fibres and tiles) with pixel alignment using MC truth from v6.5."},
    {"tilealignment_mcidealv6.5", "Ideal detector geometry with tile alignment using MC truth from v6.5."},
    {"fibrealignment_mcidealv6.5", "Ideal detector geometry with fibre alignment using MC truth from v6.5."},
    {"mppcalignment_mcidealv6.5", "Ideal detector geometry with mppc alignment using MC truth from v6.5."},
    {"pixeltimecalibration_ideal", "Pixel time calibration for the entire detector with zero shifts and uncertainties, i.e. no time walk corrections for ideal MC."},
    {"pixelqualitylm_ideal", "Perfect pixel detector with no deficiencies."},
    {"fibrequality_ideal", "Perfect fibre detector with no deficiencies."},
    {"tilequality_ideal", "Perfect tile detector with no deficiencies."},
    {"pixelefficiency_ideal", "Fully efficient for complete pixel detector."},
    {"eventstuffv1_ideal", "No limitations to time stamps."},
    {"detsetupv1_ideal", "Magnet turned on."},
  };
  
  // -- complete the tags by replacing trailing _ with the _GT
  for (auto &its: iniStructGT) {
    auto &it = its.second.tags;
    for (unsigned int i = 0; i < it.size(); i++) { 
      if (it[i].back() == '_') {
        it[i] = it[i] + its.first;
        cout << "tag " << it[i] << " from " << it[i] << " + " << its.first << endl;
      } else {
        cout << "tag " << it[i] << " left alone" << endl;
      }
    }
  } 
  
  // ----------------------------------------------------------------------
  // -- REAL START
  // ----------------------------------------------------------------------

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
  
  // -- write GT
  for (auto its : iniStructGT) {
    if (its.first != gt) continue;
    vector<string> arrayBuilder;
    for (auto it : its.second.tags) {
      string tag = ('_' == it.back()? it + its.first: it);
      arrayBuilder.push_back(tag);
    }
    stringstream sstr;
    sstr << "{ \"gt\" : \"" << its.first << "\", \"tags\" : ";
    sstr << jsFormat(arrayBuilder);
    sstr << ", \"comment\" : \"" << escapeJsonString(its.second.comment) << "\"";
    sstr << " }" << endl;
    
    JS.open(jdir + "/" + its.first);
    JS << sstr.str();
    JS.close();
    cout << sstr.str();
  }

  
  cdbPayloadWriter writer;
  string payloaddir = jsondir + "/payloads";
  
  string gtall("unset"); 
  string tagLabel = gt; 
  for (auto its : iniStructGT) {
    if (gt != its.first) continue;
    cout << "************* processing global tag " << its.first << " and gt = " << gt << endl;
    for (auto it2 : its.second.tags) {
      tagLabel = it2.substr(it2.rfind('_') + 1);
      cout << "tagLabel = " << tagLabel << " it2 = " << it2 << " its.first = " << its.first << endl;
      
      bool RF(false); // RF = rootfile
      string asciiFilename("");
      if (string::npos != its.first.find("mcideal")) {
        RF = true;
      }
      
      if (string::npos != it2.find("pixelalignment_")) {
        if (RF) {
          asciiFilename = its.second.rootfile;
        } else {
          asciiFilename = string(LOCALDIR) + "/ascii/sensors-" + tagLabel + ".csv";
        }
        writer.writeAlignmentPayloads(payloaddir, tagLabel, it2, asciiFilename, tagComments[it2], 1);
        writeInitialTag(jsondir, gt, it2, tagComments[it2]);
      }
      
      if (string::npos != it2.find("tilealignment_")) {
        if (RF) {
          asciiFilename = its.second.rootfile;
        } else {
          asciiFilename = string(LOCALDIR) + "/ascii/tiles-" + tagLabel + ".csv";
        }
        writer.writeAlignmentPayloads(payloaddir, tagLabel, it2, asciiFilename, tagComments[it2], 1);
        writeInitialTag(jsondir, gt, it2, tagComments[it2]);
      }
      
      if (string::npos != it2.find("fibrealignment_")) {
        if (RF) {
          asciiFilename = its.second.rootfile;
        } else {
          asciiFilename = string(LOCALDIR) + "/ascii/fibres-" + tagLabel + ".csv";
        }
        writer.writeAlignmentPayloads(payloaddir, tagLabel, it2, asciiFilename, tagComments[it2], 1);
        writeInitialTag(jsondir, gt, it2, tagComments[it2]);
      }
      
      if (string::npos != it2.find("mppcalignment_")) {
        if (RF) {
          asciiFilename = its.second.rootfile;
        } else {
          asciiFilename = string(LOCALDIR) + "/ascii/mppcs-" + tagLabel + ".csv";
        }
        writer.writeAlignmentPayloads(payloaddir, tagLabel, it2, asciiFilename, tagComments[it2], 1);
        writeInitialTag(jsondir, gt, it2, tagComments[it2]);
      }
      
      if (string::npos != it2.find("pixelqualitylm_")) {
        asciiFilename = string(LOCALDIR) + "/tmp-pixelqualitylm-" + tagLabel + ".csv";
        writer.writePixelQualityLMIdealInput(asciiFilename, tagLabel);
        writer.writePixelQualityLMPayloads(payloaddir, tagLabel, asciiFilename, tagComments[it2], 1);
        writeInitialTag(jsondir, gt, it2, tagComments[it2]);
        //remove(asciiFilename.c_str());
      }
      
      if (string::npos != it2.find("fibrequality_")) {  
        asciiFilename = string(LOCALDIR) + "/tmp-fibrequality-" + tagLabel + ".csv";
        writer.writeFibreQualityIdealInput(asciiFilename, tagLabel);
        writer.writeFibreQualityPayloads(payloaddir, tagLabel, asciiFilename, tagComments[it2], 1);
        writeInitialTag(jsondir, gt, it2, tagComments[it2]);
        //remove(asciiFilename.c_str());
      }
      
      if (string::npos != it2.find("tilequality_")) {
        asciiFilename = string(LOCALDIR) + "/tmp-tilequality-" + tagLabel + ".json";
        writer.writeTileQualityIdealInput(asciiFilename, tagLabel);
        writer.writeTileQualityPayloads(payloaddir, tagLabel, asciiFilename, tagComments[it2], 1);
        writeInitialTag(jsondir, gt, it2, tagComments[it2]);
        //remove(asciiFilename.c_str());
      }
      
      if (string::npos != it2.find("detsetupv1_")) {
        //writer.writeDetSetupV1Payloads(payloaddir, tagLabel, string(LOCALDIR) + "/ascii/detector-MagnetOff-v6.5.json", tagComments[it2], 1);
        writer.writeDetSetupV1Payloads(payloaddir, tagLabel, string(LOCALDIR) + "/ascii/detector-MagnetOn-v6.5.json", tagComments[it2], 1);
        writeInitialTag(jsondir, gt, it2, tagComments[it2]);
      }
      
      if (string::npos != it2.find("eventstuffv1_")) {
        writer.writeEventStuffV1Payloads(payloaddir, tagLabel, string(LOCALDIR) + "/ascii/eventstuff-ideal.json", tagComments[it2], 1);
        writeInitialTag(jsondir, gt, it2, tagComments[it2]);
      }
      
      if (string::npos != it2.find("pixelefficiency_")) {
        asciiFilename = string(LOCALDIR) + "/tmp-pixelefficiency-" + tagLabel + ".csv";
        writer.writePixelEfficiencyIdealInput(asciiFilename, tagLabel);
        writer.writePixelEfficiencyPayloads(payloaddir, tagLabel, asciiFilename, tagComments[it2], 1);
        writeInitialTag(jsondir, gt, it2, tagComments[it2]);
        //remove(asciiFilename.c_str());
      }
      
      if (string::npos != it2.find("pixeltimecalibration_")) {
        asciiFilename = string(LOCALDIR) + "/tmp-pixeltimecalibration-" + tagLabel + ".calib";
        writer.writePixelTimeCalibrationIdealInput(asciiFilename, tagLabel);      
        writer.writePixelTimeCalibrationPayloads(payloaddir, tagLabel, asciiFilename, tagComments[it2], 1);
        writeInitialTag(jsondir, gt, it2, tagComments[it2]);
        //remove(asciiFilename.c_str());
      }
      
    }
  }  
  
  return 0;
}


// ----------------------------------------------------------------------
void writeInitialTag(string jsondir, string gt, string initialTag, string comment) {
  // -- and the tag/IOVs
  string tag = initialTag;// + gt;
  stringstream sstr;
  sstr << "  { \"tag\" : \"" << tag << "\", \"iovs\" : ";
  vector<int> iovs{1};
  sstr << jsFormat(iovs);
  string lcomment = "Created (initially) for GT " + gt;
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
