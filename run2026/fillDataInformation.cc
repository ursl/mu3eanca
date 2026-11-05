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

#include "cdbJSON.hh"
#include "base64.hh"

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
//  The goal of this simple program is to determine the total hit count/rate for each subdector.
// 
// cd mu3eanca/run2026
//    ./bin/fillQualityPixels -g datav7.1=2026V2
// ----------------------------------------------------------------------



// ----------------------------------------------------------------------
void parseMidasMetaData(string filename);

string MLZRDIR("/Users/ursl/data/mu3e/run2026/mlzr");
string DATADIR("/Users/ursl/data/mu3e/run2026/raw");

//string DATADIR("/data/experiment/mu3e/data/2026/mlzr");


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
// Same rule as syncRDB mode 11: write "2" (data present) on the last DataQuality.
void uploadDataPresent(int run, const string &subsystems, const string &host, bool dry) {
  vector<string> keys;
  stringstream ks(subsystems);
  string part;
  while (getline(ks, part, ',')) {
    if (!part.empty()) keys.push_back(part);
  }
  if (keys.empty()) return;

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
    if (lastIdx < 0) {
      json dq = {
        {"mu3e", "-1"}, {"beam", "-1"}, {"vertex", "-1"}, {"pixel", "-1"},
        {"fibres", "-1"}, {"tiles", "-1"}, {"links", "-1"}, {"version", "1"}
      };
      for (const auto &k : keys) {
        dq[k] = "2";
        changed.push_back(k);
      }
      json attr;
      attr["DataQuality"] = dq;
      if (!j.contains("Attributes") || !j["Attributes"].is_array()) j["Attributes"] = json::array();
      j["Attributes"].push_back(attr);
    } else {
      json &dq = j["Attributes"][lastIdx]["DataQuality"];
      for (const auto &k : keys) {
        if (dq.contains(k) && dqGradeLocked(dq[k])) {
          cout << "leave " << k << " = " << dqGradeString(dq[k]) << endl;
          continue;
        }
        if (dq.contains(k) && dqGradeString(dq[k]) == "2") continue;
        dq[k] = "2";
        changed.push_back(k);
      }
    }

    if (changed.empty()) {
      cout << "DataQuality already set, nothing uploaded" << endl;
      curl_slist_free_all(headers);
      curl_easy_cleanup(curl);
      return;
    }

    string comment = "DataQuality data present:";
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
  int verbose(0), run(0);
  bool dry(false);
  string fstub("dqm_histos"), mstub("run"), filename("");
  string gt("datav7.1=2026V2");
  string host("mu3edb0");

  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-g"))      {gt = argv[++i];}
    if (!strcmp(argv[i], "-r"))      {run = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-h"))      {host = argv[++i];}
    if (!strcmp(argv[i], "-d"))      {dry = true;}
  }
   
  // -- open midas meta data file
  filename = DATADIR + "/" + getDataSubdir(run) + Form("/%s%d.mid.lz4.json", mstub.c_str(), run);
  if (!fileExists(filename)) {
    cout << "XXXXXXXXX fillDataInformation failed to open file " << filename << endl;
    return 0;
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

  // -- tiles and fibres come from the MIDAS counters.
  //    vertex (layer 1+2) and pixel (layer 3) need the mlzr hitmaps.
  vector<string> present;
  if (tileHits > 0) present.push_back("tiles");
  if (fibreHits > 0) present.push_back("fibres");

  double nVtxHits = 0.;
  double nPixHits = 0.;
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
    cout << "no mlzr file, vertex and pixel left unchanged" << endl;
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
  if (hTiles) cout << "nTileHitsTotal = " << hTiles->GetSumOfWeights() << endl;
  if (hFibres) cout << "nFibreHitsTotal = " << hFibres->GetSumOfWeights() << endl;

  if (nVtxHits > 0.) present.push_back("vertex");
  if (nPixHits > 0.) present.push_back("pixel");
  }

  string keylist;
  for (size_t i = 0; i < present.size(); ++i) {
    if (i) keylist += ",";
    keylist += present[i];
  }
  if (keylist.empty()) {
    cout << "no subsystem with hits, DataQuality unchanged" << endl;
  } else {
    uploadDataPresent(run, keylist, host, dry);
  }
  if (f) f->Close();

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
