#include <iostream>
#include <string.h>
#include <stdio.h>

#include <algorithm>
#include <cstdlib>
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
    // -- v8.0 
    {"mcidealv8.0V0", {
      .gt = "mcidealv8.0V0",
      .tags = {"detsetupv1_ideal", "eventstuffv2_ideal", 
               "fibrealignment_", "fibrequality_ideal",
               "mppcalignment_", 
               "pixelalignment_", "pixelqualitylm_ideal", "pixelefficiency_ideal", "pixeltimecalibration_ideal", 
               "tilealignment_", "tilequality_ideal", "tiletimecalibration_ideal"},
      .comment = "Sensor swapped geometry correction for MC ideal (=complete) detector geometry. No deficiencies, all 100% efficient",
      .rootfile = string(LOCALDIR) + "/ascii/mu3e_alignment_swap_corrected.root"
    }},
    {"datav8.0=2026V0", {
      .gt = "datav8.0=2026V0",
      .tags = {"detsetupv1_datav7.1=2026V0", "eventstuffv2_datav7.1=2026V0", 
               "fibrealignment_", "fibrequality_", 
               "mppcalignment_",    
               "pixelalignment_", "pixelqualitylm_datav7.1=2026V1", "pixelefficiency_", "pixeltimecalibration_", 
               "tilealignment_", "tilequality_", "tiletimecalibration_"},
      .comment = "Sensor swapped geometry correction for data (=complete) detector geometry, alignment_test2026_v5 by DF",
      .rootfile = string(LOCALDIR) + "/ascii/mu3e_alignment_swap_corrected.root"
    }},
    {"mcrealisticv8.0=2026V0", {
      .gt = "mcrealisticv8.0=2026V0",
      .tags = {"detsetupv1_datav7.1=2026V0", "eventstuffv2_datav7.1=2026V0", 
               "fibrealignment_", "fibrequality_datav8.0=2026V0", 
               "mppcalignment_", 
               "pixelalignment_", "pixelefficiency_datav8.0=2026V0", "pixelqualitylm_datav7.1=2026V1", "pixeltimecalibration_", 
               "tilealignment_", "tilequality_datav8.0=2026V0",  "tiletimecalibration_", 
               },
      .comment = "Sensor swapped geometry correction for MC realistic (=complete) detector geometry. Should describe 2026 data",
      .rootfile = string(LOCALDIR) + "/ascii/mu3e_alignment_swap_corrected.root"
    }},
    // -- v6.5
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
    {"mcrealisticv6.5=2025V0", {
      .gt = "mcrealisticv6.5=2025V0",
      .tags = {"pixelalignment_mcidealv6.5=2025", "fibrealignment_mcidealv6.5=2025", "tilealignment_mcidealv6.5=2025", "mppcalignment_mcidealv6.5=2025", 
               "pixelqualitylm_datav6.5=2025V0", "fibrequality_datav6.3=2025V0", "tilequality_datav6.3=2025V0", 
               "pixelefficiency_datav6.5=2025V0", "pixeltimecalibration_", 
               "eventstuffv1_ideal", "detsetupv1_"},
      .comment = "MC realistic 2025 detector conditions with *placeholder* MC (no) smearing. Ideal 2025 geometries using MC truth from v6.5 (VTX + DS tiles + all fibres). Starting point for analysis of MC simulation data.",
      .rootfile = string(LOCALDIR) + "/ascii/mu3e_alignment_mcidealv6.5.root"
    }}
  };
  
  // -- comments for tags. Since tags can be part of multiple GT, do not insert this into iniStructGT
  map<string, string> tagComments = {
    {"detsetupv1_ideal", "Ideal with magnet turned on."},
    {"eventstuffv2_ideal", "Ideal eventstuffv2: no frame limits."},

    {"fibrequality_ideal", "Perfect fibre detector with no deficiencies."},

    {"pixelefficiency_ideal", "Fully efficient for complete pixel detector."},
    {"pixelqualitylm_ideal", "Perfect pixel detector with no deficiencies."},
    {"pixeltimecalibration_ideal", "Pixel time calibration for the entire detector with zero shifts and uncertainties, i.e. no time walk corrections for ideal MC."},

    {"pixelalignment_mcidealv8.0V0", "Swapped sensor geometry correction"},
    {"pixelalignment_mcrealisticv8.0=2026V0", "Swapped sensor geometry correction"},
    {"pixelalignment_datav8.0=2026V0", "Swapped sensor geometry correction with alignment_test2026_v5 by DF"},

    {"mppcalignment_mcidealv8.0V0", "Correct MPPC IDs as in det r/o "},
    {"mppcalignment_mcidealv8.0V0", "Correct MPPC IDs as in det r/o "},
    {"mppcalignment_mcrealisticv8.0=2026V0", "Correct MPPC IDs as in det r/o "},

    {"tilequality_ideal", "Perfect tile detector with no deficiencies."},

    // -- old stuff
    {"pixelalignment_mcidealv6.5", "Ideal detector geometry with pixel alignment using MC truth from v6.5."},
    {"pixelalignment_mcidealv6.5=2025", "Ideal detector geometry (VTX and complete fibres and tiles) with pixel alignment using MC truth from v6.5."},
    {"tilealignment_mcidealv6.5", "Ideal detector geometry with tile alignment using MC truth from v6.5."},
    {"fibrealignment_mcidealv6.5", "Ideal detector geometry with fibre alignment using MC truth from v6.5."},
    {"mppcalignment_mcidealv6.5", "Ideal detector geometry with mppc alignment using MC truth from v6.5."},
    {"eventstuffv1_ideal", "No limitations to time stamps."},
    {"eventstuffv2_datav8.0=2026V0", "IOV 1 is the ideal eventstuffv2 input."}
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
  string jsondir("~/data/mu3e/test-cdb");
  string gt("");
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-j"))  {jsondir    = argv[++i];}
    if (!strcmp(argv[i], "-g"))  {gt         = argv[++i];}
  }

  // opendir and ofstream leave '~' literal. system("mkdir -p ...") runs a
  // shell, which expands it, so the 0000 block dir is created under $HOME
  // while the payload is opened at a path that does not exist.
  if (!jsondir.empty() && jsondir[0] == '~') {
    const char *home = getenv("HOME");
    if (home && home[0] != '\0' && (jsondir.size() == 1 || jsondir[1] == '/')) {
      jsondir = string(home) + jsondir.substr(1);
    }
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
    sort(arrayBuilder.begin(), arrayBuilder.end());
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
    cout << "cdbInitGT: alignment root file " << its.second.rootfile << endl;
    // datav8.0=2026V0 -> "2026". A name without =YYYY stays "all" (full detector).
    // Used for alignment and for the ideal quality, efficiency, and time-calibration tags.
    string payloadMode = "all";
    size_t eq = gt.find('=');
    if (eq != string::npos && eq + 4 <= gt.size()) {
      string year = gt.substr(eq + 1, 4);
      if (year.size() == 4
          && year[0] >= '0' && year[0] <= '9'
          && year[1] >= '0' && year[1] <= '9'
          && year[2] >= '0' && year[2] <= '9'
          && year[3] >= '0' && year[3] <= '9') {
        payloadMode = year;
      }
    }
    cout << "cdbInitGT: payload mode " << payloadMode << endl;
    for (auto it2 : its.second.tags) {
      tagLabel = it2.substr(it2.rfind('_') + 1);
      cout << "tagLabel = " << tagLabel << " it2 = " << it2 << " its.first = " << its.first << endl;

      // A tag whose name ends with this GT is owned by it and is rewritten, so a
      // corrected alignment root file replaces the previous mppc/sensor IDs.
      // A borrowed or shared tag (fibrequality_ideal, another GT's alignment)
      // is left in place.
      bool owned = its.first.size() <= it2.size()
          && it2.compare(it2.size() - its.first.size(), its.first.size(), its.first) == 0;
      if (!owned && cdbTagOrPayloadTreeExists(jsondir, payloaddir, it2)) {
        cout << "cdbInitGT: skip tag \"" << it2 << "\" (existing " << jsondir << "/tags/" << it2
             << " and/or " << payloaddir << "/" << it2 << "/)" << endl;
        continue;
      }

      // A GT with a root file uses it for every alignment tag. The CSV names
      // are the fallback for a GT that has none.
      bool RF = !its.second.rootfile.empty();
      string asciiFilename("");
      
      if (string::npos != it2.find("pixelalignment_")) {
        if (RF) {
          asciiFilename = its.second.rootfile;
        } else {
          asciiFilename = string(LOCALDIR) + "/ascii/sensors-" + tagLabel + ".csv";
        }
        cout << "cdbInitGT: " << it2 << " from " << asciiFilename << endl;
        writer.writeAlignmentPayloads(payloaddir, tagLabel, it2, asciiFilename, tagComments[it2], 1, payloadMode);
        writeInitialTag(jsondir, gt, it2, tagComments[it2]);
      }
      
      if (string::npos != it2.find("tilealignment_")) {
        if (RF) {
          asciiFilename = its.second.rootfile;
        } else {
          asciiFilename = string(LOCALDIR) + "/ascii/tiles-" + tagLabel + ".csv";
        }
        cout << "cdbInitGT: " << it2 << " from " << asciiFilename << endl;
        writer.writeAlignmentPayloads(payloaddir, tagLabel, it2, asciiFilename, tagComments[it2], 1, payloadMode);
        writeInitialTag(jsondir, gt, it2, tagComments[it2]);
      }
      
      if (string::npos != it2.find("fibrealignment_")) {
        if (RF) {
          asciiFilename = its.second.rootfile;
        } else {
          asciiFilename = string(LOCALDIR) + "/ascii/fibres-" + tagLabel + ".csv";
        }
        cout << "cdbInitGT: " << it2 << " from " << asciiFilename << endl;
        writer.writeAlignmentPayloads(payloaddir, tagLabel, it2, asciiFilename, tagComments[it2], 1, payloadMode);
        writeInitialTag(jsondir, gt, it2, tagComments[it2]);
      }
      
      if (string::npos != it2.find("mppcalignment_")) {
        if (RF) {
          asciiFilename = its.second.rootfile;
        } else {
          asciiFilename = string(LOCALDIR) + "/ascii/mppcs-" + tagLabel + ".csv";
        }
        cout << "cdbInitGT: " << it2 << " from " << asciiFilename << endl;
        writer.writeAlignmentPayloads(payloaddir, tagLabel, it2, asciiFilename, tagComments[it2], 1, payloadMode);
        writeInitialTag(jsondir, gt, it2, tagComments[it2]);
      }
      
      if (string::npos != it2.find("pixelqualitylm_")) {
        asciiFilename = string(LOCALDIR) + "/tmp-pixelqualitylm-" + tagLabel + ".csv";
        writer.writePixelQualityLMIdealInput(asciiFilename, payloadMode);
        writer.writePixelQualityLMPayloads(payloaddir, tagLabel, asciiFilename, tagComments[it2], 1);
        writeInitialTag(jsondir, gt, it2, tagComments[it2]);
        //remove(asciiFilename.c_str());
      }
      
      if (string::npos != it2.find("fibrequality_")) {  
        asciiFilename = string(LOCALDIR) + "/tmp-fibrequality-" + tagLabel + ".csv";
        writer.writeFibreQualityIdealInput(asciiFilename, payloadMode);
        writer.writeFibreQualityPayloads(payloaddir, tagLabel, asciiFilename, tagComments[it2], 1);
        writeInitialTag(jsondir, gt, it2, tagComments[it2]);
        //remove(asciiFilename.c_str());
      }
      
      if (string::npos != it2.find("tilequality_")) {
        asciiFilename = string(LOCALDIR) + "/tmp-tilequality-" + tagLabel + ".json";
        writer.writeTileQualityIdealInput(asciiFilename, payloadMode);
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

      // IOV 1 of a data tag is the ideal input; later runs are added separately.
      if (string::npos != it2.find("eventstuffv2_")) {
        writer.writeEventStuffV2Payloads(payloaddir, tagLabel, string(LOCALDIR) + "/ascii/eventstuffv2-ideal.json", tagComments[it2], 1);
        writeInitialTag(jsondir, gt, it2, tagComments[it2]);
      }
      
      if (string::npos != it2.find("pixelefficiency_")) {
        asciiFilename = string(LOCALDIR) + "/tmp-pixelefficiency-" + tagLabel + ".csv";
        writer.writePixelEfficiencyIdealInput(asciiFilename, payloadMode);
        writer.writePixelEfficiencyPayloads(payloaddir, tagLabel, asciiFilename, tagComments[it2], 1);
        writeInitialTag(jsondir, gt, it2, tagComments[it2]);
        //remove(asciiFilename.c_str());
      }
      
      if (string::npos != it2.find("pixeltimecalibration_")) {
        asciiFilename = string(LOCALDIR) + "/tmp-pixeltimecalibration-" + tagLabel + ".calib";
        writer.writePixelTimeCalibrationIdealInput(asciiFilename, payloadMode);      
        writer.writePixelTimeCalibrationPayloads(payloaddir, tagLabel, asciiFilename, tagComments[it2], 1);
        writeInitialTag(jsondir, gt, it2, tagComments[it2]);
        //remove(asciiFilename.c_str());
      }

      // Zero offsets and timewalk. payloadMode selects the tiles.
      if (string::npos != it2.find("tiletimecalibration_")) {
        asciiFilename = string(LOCALDIR) + "/tmp-tiletimecalibration-" + tagLabel + ".json";
        writer.writeTileTimeCalibrationIdealInput(asciiFilename, payloadMode);
        writer.writeTileTimeCalibrationPayloads(payloaddir, tagLabel, asciiFilename, tagComments[it2], 1);
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
