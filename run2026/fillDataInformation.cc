#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <filesystem>

#include <cstdint>
#include <ctime>
#include <vector>
#include <string.h>
#include <chrono>
#include <algorithm> // for std::lower_bound
#include <dirent.h>

#include <fstream>

//#include "cdbUtil.hh" 
#include "Mu3eConditions.hh"
#include "calPixelAlignment.hh"
#include "calPixelQualityLM.hh"
#include "calEventStuffV3.hh"
#include "cdbJSON.hh"
#include "base64.hh"
#include "DataQuality.hh"

#include <nlohmann/json.hpp>
#include <curl/curl.h>
using json = nlohmann::ordered_json;


#include "asicInfo.hh"
#include "util.hh"

#include "TCanvas.h"
#include "TStyle.h"
#include "TFile.h"
#include "TTree.h"
#include "TH2F.h"
#include "TMath.h"
#include "TKey.h"
#include "TROOT.h"
#include "TSystem.h"

using namespace std;

// ----------------------------------------------------------------------
// fillDataInformation
// -------------------
// 
//  The goal of this simple program is to determine stuff for the RDB DataQuality resource
//  - the total hit count/rate for each subdector.
//  - deff for the DAQ
//  - possibly other stuff from calEventStuffV3
// 
// cd mu3eanca/run2026
//    ./bin/fillQualityPixels -g datav7.1=2026V2
// ----------------------------------------------------------------------



// ----------------------------------------------------------------------
void parseMidasMetaData(string filename);

//string MLZRDIR("/Users/ursl/data/mu3e/run2026/mlzr");
//string DATADIR("/Users/ursl/data/mu3e/run2026/raw");

string MLZRDIR("/data/experiment/mu3e/data/2026/mlzr");
string DATADIR("/data/experiment/mu3e/data/2026/raw");



// ----------------------------------------------------------------------
static bool fileExists(const string& filename) {
  ifstream in(filename.c_str());
  return in.good();
}

// ----------------------------------------------------------------------
string getDataSubdir(int runnumber) {
  int block = runnumber / 1000;
  string blockdir = Form("%03d", block);
  return Form("%s", blockdir.c_str());
}

// ----------------------------------------------------------------------
static size_t curlWrite(void *contents, size_t size, size_t nmemb, void *userp) {
  ((string*)userp)->append((char*)contents, size * nmemb);
  return size * nmemb;
}

// ----------------------------------------------------------------------
// "0" bad and "1" good stay as they are. Unset (and a missing field) becomes "2".
static bool dqGradeLocked(const json &v) {
  if (v.is_string()) {
    string s = v.get<string>();
    return s == "0" || s == "1";
  }
  if (v.is_number_integer()) {
    int n = v.get<int>();
    return n == 0 || n == 1;
  }
  return false;
}

static string dqGradeString(const json &v) {
  if (v.is_string()) return v.get<string>();
  if (v.is_number_integer()) return to_string(v.get<int>());
  return "";
}

static string currentDateTime() {
  time_t now = time(nullptr);
  struct tm tmnow;
  localtime_r(&now, &tmnow);
  char buf[32];
  strftime(buf, sizeof(buf), "%Y/%m/%d %H:%M:%S", &tmnow);
  return string(buf);
}

// ----------------------------------------------------------------------
// Write subsystem grades on the last DataQuality attribute.
// Detector histograms: hits become DQ_PRESENT ("2"); no hits become DQ_NODATA ("3").
// deff becomes DQ_GOOD ("1") or DQ_BAD ("0") when the DEFF list is present.
// An existing "0" or "1" stays unless -f replaces it.
void uploadDataPresent(int run, const map<string, string> &desired, const string &host, bool dry, bool force) {
  if (desired.empty()) return;
  
  string responseData;
  CURL *curl = curl_easy_init();
  if (!curl) return;
  
  string url = host + ":5050/rdb/run/" + to_string(run) + "/json";
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWrite);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "Content-Type: application/json");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  
  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    cerr << "GET " << url << " failed: " << curl_easy_strerror(res) << endl;
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return;
  }
  long httpCode = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
  if (httpCode != 200) {
    cerr << "GET " << url << " returned " << httpCode << endl;
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return;
  }
  
  try {
    json j = json::parse(responseData);
    if (j.contains("_id")) j.erase("_id");
    
    int lastIdx = -1;
    if (j.contains("Attributes") && j["Attributes"].is_array()) {
      for (size_t i = 0; i < j["Attributes"].size(); ++i) {
        if (j["Attributes"][i].is_object() && j["Attributes"][i].contains("DataQuality")
        && j["Attributes"][i]["DataQuality"].is_object()) {
          lastIdx = static_cast<int>(i);
        }
      }
    }
    
    vector<string> changed;
    auto applyGrades = [&](json &dq) {
      for (const auto &kv : desired) {
        const string &k = kv.first;
        const string &want = kv.second;
        if (dq.contains(k) && dqGradeString(dq[k]) == want) continue;
        if (!force && dq.contains(k) && dqGradeLocked(dq[k])) {
          cout << "leave " << k << " = " << dqGradeString(dq[k]) << endl;
          continue;
        }
        dq[k] = want;
        changed.push_back(k + "=" + want);
      }
    };
    
    if (lastIdx < 0) {
      json dq = {
        {"mu3e", "-1"}, {"beam", "-1"}, {"vertex", "-1"}, {"pixel", "-1"},
        {"fibres", "-1"}, {"tiles", "-1"}, {"links", "-1"}, {"deff", "-1"}, {"version", "1"}
      };
      applyGrades(dq);
      json attr;
      attr["DataQuality"] = dq;
      if (!j.contains("Attributes") || !j["Attributes"].is_array()) j["Attributes"] = json::array();
      j["Attributes"].push_back(attr);
    } else {
      applyGrades(j["Attributes"][lastIdx]["DataQuality"]);
    }
    
    if (changed.empty()) {
      cout << "DataQuality already set, nothing uploaded" << endl;
      curl_slist_free_all(headers);
      curl_easy_cleanup(curl);
      return;
    }
    
    string comment = force ? "DataQuality replaced:" : "DataQuality data present:";
    for (const auto &k : changed) comment += " " + k;
    json historyEntry = {{"date", currentDateTime()}, {"comment", comment}};
    if (j.contains("History") && j["History"].is_array()) j["History"].push_back(historyEntry);
    else j["History"] = json::array({historyEntry});
    
    cout << comment << endl;
    if (dry) {
      cout << "dry run, not uploading" << endl;
      curl_slist_free_all(headers);
      curl_easy_cleanup(curl);
      return;
    }
    
    responseData = j.dump();
    string putUrl = host + ":5050/rdb/runrecords";
    curl_easy_reset(curl);
    curl_easy_setopt(curl, CURLOPT_URL, putUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, responseData.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(responseData.size()));
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      cerr << "PUT " << putUrl << " failed: " << curl_easy_strerror(res) << endl;
    } else {
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
      cout << "uploaded DataQuality for run " << run << " (HTTP " << httpCode << ")" << endl;
    }
  } catch (const exception &e) {
    cerr << "DataQuality upload failed: " << e.what() << endl;
  }
  
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
}

// ----------------------------------------------------------------------
string findFile(string dir, string filename, int run) {
  vector<string> vdirs;  
  for (filesystem::directory_iterator it(dir), end; it != end; ++it) {
    cout << "it->path() = " << it->path() << endl;
    if (is_directory(it->path())) {
      vdirs.push_back(it->path().string());
    }
  }
  sort(vdirs.begin(), vdirs.end(), [](const string& a, const string& b) {
    return filesystem::last_write_time(a) > filesystem::last_write_time(b);
  });
  
  for (const auto &d : vdirs) {
    string completeName = d + "/" + Form("%s/", getDataSubdir(run).c_str()) + filename;
    cout << "checking file " << completeName << endl;
    if (fileExists(completeName)) {
      cout << "found file " << completeName << endl;
      return completeName;
    }
  }
  return  "NOT FOUND";
}


// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {
  
  // -- command line arguments
  int verbose(0), run1(-1), run2(-1);
  bool dry(false), force(false);
  string fstub("dqm_histos"), mstub("run"), filename("");
  string gt("datav7.1=2026V2");
  string host("mu3edb0");
  
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-d"))      {dry = true;}
    if (!strcmp(argv[i], "-f"))      {force = true;}
    if (!strcmp(argv[i], "-g"))      {gt = argv[++i];}
    if (!strcmp(argv[i], "-r1"))     {run1 = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-r2"))     {run2 = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-h"))      {host = argv[++i];}
  }
  
  if (run1 < 0 || run2 < run1) {
    cout << "need -r1 and -r2, with run1 <= run2" << endl;
    return 1;
  }
  
  for (int run = run1; run <= run2; ++run) {
    cout << "-------- run " << run << endl;
    
    // -- open midas meta data file
    filename = DATADIR + "/" + getDataSubdir(run) + Form("/%s%d.mid.lz4.json", mstub.c_str(), run);
    if (!fileExists(filename)) {
      cout << "no midas metadata for run " << run << ", skipping" << endl;
      continue;
    }
    json j; 
    ifstream mf(filename); 
    mf >> j; 
    uint64_t tsLast = j["stat"].value("last_frame_of_the_run", uint64_t{0});
    cout << "tsLast = " << tsLast << " (" << Form("%llu", tsLast) << ")" << endl;
    
    uint64_t pixelHits = j["stat"].value("n_phit", uint64_t{0});
    uint64_t tileHits = j["stat"].value("n_thit", uint64_t{0});
    uint64_t fibreHits = j["stat"].value("n_fhit", uint64_t{0});
    int      midasEvts = j["stat"].value("n_midasEvent", int{0});
    
    cout << "pixelHits = " << pixelHits << " (" << Form("%llu", pixelHits) << ")" << endl;
    cout << "tileHits = " << tileHits << " (" << Form("%llu", tileHits) << ")" << endl;
    cout << "fibreHits = " << fibreHits << " (" << Form("%llu", fibreHits) << ")" << endl;
    cout << "midasEvts = " << midasEvts << " (" << Form("%d", midasEvts) << ")" << endl;
    
    // -- vertex, pixel, tiles, and fibres all come from the mlzr histograms.
    //    MIDAS n_thit / n_fhit are printed above and are not used for the grade.
    double nVtxHits = 0.;
    double nPixHits = 0.;
    double nTileHits = 0.;
    double nFibreHits = 0.;
    bool haveTiles = false;
    bool haveFibres = false;
    bool haveMlzr = false;
    string mlzrFile = "NOT FOUND";
    try {
      mlzrFile = findFile(MLZRDIR, Form("%s_%d.root", fstub.c_str(), run), run);
    } catch (const exception &e) {
      cout << "mlzr scan failed: " << e.what() << endl;
    }
    TFile *f = 0;
    if (mlzrFile != "NOT FOUND") f = TFile::Open(mlzrFile.c_str());
    if (f && !f->IsZombie()) {
      haveMlzr = true;
    } else {
      cout << "no mlzr file, DataQuality left unchanged" << endl;
    }
    
    if (haveMlzr) {
      
      // -- read in ALL chipids in VTX
      vector<unsigned int> vchipid;
      vector<string> vStations = {"station_0"};
      map<string, int> vLayers  = {{"layer_1", 8}, {"layer_2", 10}, {"layer_3", 24}};
      map<int, TH2*> pixelHitmaps;
      
      // -- read in all hitmaps
      for (auto itS : vStations) {
        for (auto itL : vLayers) {
          for (int iLdr = 1; iLdr <= itL.second; ++iLdr) {
            stringstream ss;
            ss << "pixel/hitmaps/" << itS << "/" << itL.first
            << Form("/ladder_%02d", iLdr);
            string s = ss.str();
            if (0) cout << "TDirectory " << s << "<-" << endl;
            if (!f->GetDirectory(s.c_str())) continue;
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
                if (string::npos != name.find("hitmap_perChip_withToTCut_")) continue;
                if (string::npos != name.find("hitmap_perChip")) {
                  unsigned int chipID;
                  replaceAll(name, "hitmap_perChip_", "");
                  if (verbose > 9) cout << "   .. chipID = ";
                  chipID = ::stoi(name); 
                  if (verbose > 9) cout << chipID << " from name = " << oname << endl;
                  if (0 == chipID) {
                    cout << "  XXXXXXXXX  skipping illegal chipID = " << chipID << " from name = " << name << " is 0, skipping" << endl;
                    continue;
                  }
                  vchipid.push_back(chipID);
                  TH2F *h = (TH2F*)key->ReadObj();
                  pixelHitmaps.insert(make_pair(chipID, h));
                  // layer_1 and layer_2 are the vertex detector; layer_3 is the outer pixel layer
                  double w = h->GetSumOfWeights();
                  if (itL.first == "layer_3") nPixHits += w;
                  else nVtxHits += w;
                }
              }
            }
          }
        }
      }
      cout << "pixelHitmaps.size() = " << pixelHitmaps.size() << endl;
      cout << "vertex hits (layer_1+layer_2) = " << nVtxHits << endl;
      cout << "pixel hits (layer_3) = " << nPixHits << endl;
      
      TH2 *hTiles = (TH2*)f->Get("tile/Zphi_TileHitmap_Global");
      TH2 *hFibres = (TH2*)f->Get("fibre/channelID_all");
      if (hTiles) {
        haveTiles = true;
        nTileHits = hTiles->GetSumOfWeights();
        cout << "nTileHitsTotal = " << nTileHits << endl;
      }
      if (hFibres) {
        haveFibres = true;
        nFibreHits = hFibres->GetSumOfWeights();
        cout << "nFibreHitsTotal = " << nFibreHits << endl;
      }
    }

    // -- deff for the DAQ
    calEventStuffV3 *ces = new calEventStuffV3();
    ces->readJSON(filename);
    double deffPixel = ces->deff("Central pixel total");
    double deffTile  = ces->deff("DS tile total");
    double deffFibre = ces->deff("SciFi total");
    bool goodDeff = (deffPixel > 0.99 && deffTile > 0.75 && deffFibre > 0.99);
    bool haveDeff = ces->n_deff() > 0;
    cout << "deffPixel = " << deffPixel << ", deffTile = " << deffTile << ", deffFibre = " << deffFibre << endl;
    cout << "goodDeff = " << goodDeff << " (n_deff = " << ces->n_deff() << ")" << endl;
    delete ces;

    // Subsystem -> grade. No hits is DQ_NODATA, including when the stored grade is still unset.
    map<string, string> desired;
    auto consider = [&](const string &name, bool known, double hits) {
      if (!known) return;
      if (hits > 0.) desired[name] = to_string(DQ_PRESENT);
      else desired[name] = to_string(DQ_NODATA);
    };
    if (haveMlzr) {
      consider("vertex", true, nVtxHits);
      consider("pixel", true, nPixHits);
      consider("tiles", haveTiles, nTileHits);
      consider("fibres", haveFibres, nFibreHits);
    }
    if (haveDeff) {
      desired["deff"] = goodDeff ? to_string(DQ_GOOD) : to_string(DQ_BAD);
    }
    if (desired.empty()) {
      cout << "no grades, DataQuality unchanged" << endl;
    } else {
      uploadDataPresent(run, desired, host, dry, force);
    }
    if (f) {
      f->Close();
      delete f;
    }
    
  }
  
  return 0;
}

// ----------------------------------------------------------------------
void parseMidasMetaData(string filename) {
  if (!fileExists(filename)) {
    cout << "parseMidasMetaData> ERROR: file " << filename << " does not exist" << endl;
    return;
  }
  json j;
  ifstream f(filename);
  f >> j;
} 
