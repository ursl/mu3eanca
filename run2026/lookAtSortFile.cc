#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <string.h>
#include <chrono>
#include <algorithm> // for std::lower_bound
#include <dirent.h>
#include <sys/stat.h>

#include <fstream>

#include "Mu3eConditions.hh"
#include "calTileAlignment.hh"
#include "calTileQuality.hh"

#include <nlohmann/json.hpp>
using json = nlohmann::ordered_json;

#include "cdbJSON.hh"
#include "base64.hh"

#include "TApplication.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TFile.h"
#include "TTree.h"
#include "TH2F.h"
#include "TMarker.h"
#include "TMath.h"
#include "TKey.h"
#include "TROOT.h"
#include "TSystem.h"

#include <map>
#include <cstdint>

using namespace std;

// ----------------------------------------------------------------------
// lookAtSortFile
// --------------
// Look at a sort file and print the contents
//
// Examples:
//          ./bin/lookAtSortFile ~/data/mu3e/run2026/sort/v04/012/run12272-sorted.root
//          ./bin/lookAtSortFile -l 1,2 -n 10 -t 50 -s 1000 <sortfile>
//
// A frame is every entry that shares one frameId. Hits are accumulated until
// frameId changes, and only then is that frame drawn.
// hit_pixelid is
//   row = id & 0xFF, col = (id >> 8) & 0xFF, sensor = (id >> 16) & 0xFFFF
// and the sensor word is station<<12 | layer<<10 | ladder<<5 | chip.
// phit packs the same id in bits 61..32, ToT in 31..27 and the hit time in 26..0.
// Three canvases show the central station: L1 (8 ladders, chips 1-6),
// L2 (10 ladders, chips 1-6) and L3 (24 ladders, chips 7-23).
// Columns run along the ladder, rows are ladders.
// Each hit is a filled circle. Red is ToT < 5, green is 5 <= ToT < 16, blue is ToT >= 16.
// ----------------------------------------------------------------------

struct PixHit {
  int sensor;
  int station;
  int layer;
  int ladder;
  int chip;
  int col;
  int row;
  int tot;
  unsigned int timestamp;
};

struct LayerView {
  const char* name;
  int layer;
  int nLadder;
  int chipLo;
  int nChip;
  TCanvas* canvas;
  vector<TH2F*> pads;
  struct Mark {
    int ipad;
    double x;
    double y;
    int color;
  };
  vector<Mark> hits;
  struct Drawn {
    int ipad;
    TMarker* marker;
  };
  vector<Drawn> drawn;
};

// Marker size is in ROOT units (1 is about 8 pixels).
static const double kMarkerSize = 0.9;
static const int kTotColor[3] = {kRed, kGreen, kBlue};

static int totColor(int tot) {
  if (tot < 5) return 0;
  if (tot < 16) return 1;
  return 2;
}

// ----------------------------------------------------------------------
static PixHit decodeHit(uint32_t pixelId, uint32_t timestamp, int tot) {
  PixHit h;
  h.row = int(pixelId & 0xFF);
  h.col = int((pixelId >> 8) & 0xFF);
  h.sensor = int((pixelId >> 16) & 0xFFFF);
  h.station = (h.sensor >> 12) & 0x7;
  h.layer = (h.sensor >> 10) & 0x3;
  h.ladder = (h.sensor >> 5) & 0x1F;
  h.chip = h.sensor & 0x1F;
  h.tot = tot;
  h.timestamp = timestamp;
  return h;
}

// phit: sensor/col/row in bits 61..32, ToT in 31..27, time in 26..0.
static PixHit decodePhit(ULong64_t word) {
  uint32_t pixelId = uint32_t(word >> 32) & 0x3FFFFFFF;
  int tot = int((word >> 27) & 0x1F);
  uint32_t timestamp = uint32_t(word & 0x07FFFFFF);
  return decodeHit(pixelId, timestamp, tot);
}

// ----------------------------------------------------------------------
static void bookLayer(LayerView& v, int x0, int y0, int ww, int hh) {
  v.canvas = new TCanvas(Form("c%s", v.name), v.name, x0, y0, ww, hh);
  v.canvas->Divide(v.nChip, v.nLadder, 0.001, 0.001);
  int nPad = v.nLadder * v.nChip;
  v.pads.resize(nPad, 0);
  for (int ladder = 0; ladder < v.nLadder; ++ladder) {
    for (int ichip = 0; ichip < v.nChip; ++ichip) {
      int chip = v.chipLo + ichip;
      int ipad = ladder * v.nChip + ichip;
      TH2F* h = new TH2F(Form("h%s_L%d_C%d", v.name, ladder, chip),
                         Form("%d:%d", ladder, chip),
                         256, 0., 256., 250, 0., 250.);
      h->SetDirectory(0);
      h->GetXaxis()->SetLabelSize(0);
      h->GetYaxis()->SetLabelSize(0);
      h->GetXaxis()->SetTickLength(0);
      h->GetYaxis()->SetTickLength(0);
      h->SetTitleSize(0.15);
      v.pads[ipad] = h;
      v.canvas->cd(ipad + 1);
      gPad->SetLeftMargin(0.02);
      gPad->SetRightMargin(0.02);
      gPad->SetTopMargin(0.14);
      gPad->SetBottomMargin(0.02);
      h->Draw("AXIS");
    }
  }
  v.canvas->Modified();
  v.canvas->Update();
}

// ----------------------------------------------------------------------
int main(int argc, char** argv) {
  string sortfile;
  int nShow = 30;
  int nSkip = 0;
  int holdMs = 250;
  bool showLayer[3] = {true, true, true};
  bool layersSet = false;

  for (int i = 1; i < argc; ++i) {
    string arg = argv[i];
    if (arg == "-l") {
      if (i + 1 >= argc) {
        cout << "Usage: " << argv[0] << " [-l 1,2,3] [-n N] [-s N] [-t ms] <sortfile>" << endl;
        return 1;
      }
      showLayer[0] = showLayer[1] = showLayer[2] = false;
      layersSet = true;
      stringstream spec(argv[++i]);
      string item;
      while (getline(spec, item, ',')) {
        if (item.empty()) continue;
        int lay = atoi(item.c_str());
        if (lay < 1 || lay > 3) {
          cout << "Error: layer must be 1, 2, or 3, got " << item << endl;
          return 1;
        }
        showLayer[lay - 1] = true;
      }
    } else if (arg == "-n") {
      if (i + 1 >= argc) {
        cout << "Usage: " << argv[0] << " [-l 1,2,3] [-n N] [-s N] [-t ms] <sortfile>" << endl;
        return 1;
      }
      nShow = atoi(argv[++i]);
      if (nShow <= 0) {
        cout << "Error: -n expects a positive number of frames" << endl;
        return 1;
      }
    } else if (arg == "-t") {
      if (i + 1 >= argc) {
        cout << "Usage: " << argv[0] << " [-l 1,2,3] [-n N] [-s N] [-t ms] <sortfile>" << endl;
        return 1;
      }
      holdMs = atoi(argv[++i]);
      if (holdMs < 0) {
        cout << "Error: -t expects a hold time in milliseconds" << endl;
        return 1;
      }
    } else if (arg == "-s") {
      if (i + 1 >= argc) {
        cout << "Usage: " << argv[0] << " [-l 1,2,3] [-n N] [-s N] [-t ms] <sortfile>" << endl;
        return 1;
      }
      nSkip = atoi(argv[++i]);
      if (nSkip < 0) {
        cout << "Error: -s expects a number of frames to skip" << endl;
        return 1;
      }
    } else if (sortfile.empty()) {
      sortfile = arg;
    } else {
      cout << "Error: unexpected argument " << arg << endl;
      cout << "Usage: " << argv[0] << " [-l 1,2,3] [-n N] [-s N] [-t ms] <sortfile>" << endl;
      return 1;
    }
  }
  if (sortfile.empty() || (layersSet && !showLayer[0] && !showLayer[1] && !showLayer[2])) {
    cout << "Usage: " << argv[0] << " [-l 1,2,3] [-n N] [-s N] [-t ms] <sortfile>" << endl;
    return 1;
  }

  TFile* file = TFile::Open(sortfile.c_str());
  if (!file) {
    cout << "Error: could not open file " << sortfile << endl;
    return 1;
  }
  TTree* tree = (TTree*) file->Get("mu3e");
  if (!tree) {  
    cout << "Error: could not get tree from file " << sortfile << endl;
    return 1;
  }

  tree->SetBranchStatus("*", 0);
  const char* keep[] = {"frameId", "Nhit", "phit", "hit_pixelid", "hit_timestamp", "hit_tot"};
  for (const char* b : keep) {
    if (tree->GetBranch(b)) tree->SetBranchStatus(b, 1);
  }

  ULong64_t frameId = 0;
  int nHit = 0;
  vector<ULong64_t>* phit = 0;
  vector<unsigned int>* pixelId = 0;
  vector<unsigned int>* timestamp = 0;
  vector<int>* tot = 0;
  if (tree->GetBranch("frameId")) tree->SetBranchAddress("frameId", &frameId);
  if (tree->GetBranch("Nhit")) tree->SetBranchAddress("Nhit", &nHit);
  if (tree->GetBranch("phit")) tree->SetBranchAddress("phit", &phit);
  if (tree->GetBranch("hit_pixelid")) tree->SetBranchAddress("hit_pixelid", &pixelId);
  if (tree->GetBranch("hit_timestamp")) tree->SetBranchAddress("hit_timestamp", &timestamp);
  if (tree->GetBranch("hit_tot")) tree->SetBranchAddress("hit_tot", &tot);
  if (nSkip > 0) {
    tree->SetBranchStatus("*", 0);
    if (tree->GetBranch("frameId")) tree->SetBranchStatus("frameId", 1);
  }

  TApplication app("lookAtSortFile", 0, 0);
  gStyle->SetOptStat(0);

  LayerView views[3] = {
    {"L1", 0,  8, 1,  6, 0, {}},
    {"L2", 1, 10, 1,  6, 0, {}},
    {"L3", 2, 24, 7, 17, 0, {}}
  };
  const int layerW[3] = {980, 980, 1500};
  const int layerH[3] = {1100, 1300, 1600};
  int xCanvas = 10;
  for (int i = 0; i < 3; ++i) {
    if (!showLayer[i]) continue;
    bookLayer(views[i], xCanvas, 10, layerW[i], layerH[i]);
    xCanvas += layerW[i] + 30;
  }

  int shown = 0;
  int nSkipped = 0;
  int nEntriesInFrame = 0;
  int nHitsInFrame = 0;
  int nLayer[3] = {0, 0, 0};
  Long64_t frameBegin = 0;
  ULong64_t currentFrame = 0;
  bool inFrame = false;
  bool stop = false;
  Long64_t nEntries = tree->GetEntries();

  auto clearFrame = [&]() {
    for (auto& v : views) {
      for (auto& d : v.drawn) {
        if (d.marker && v.canvas) {
          v.canvas->cd(d.ipad + 1);
          gPad->GetListOfPrimitives()->Remove(d.marker);
          gPad->Modified();
        }
        delete d.marker;
      }
      v.drawn.clear();
      v.hits.clear();
    }
    nEntriesInFrame = 0;
    nHitsInFrame = 0;
    nLayer[0] = nLayer[1] = nLayer[2] = 0;
  };

  // Draw the frame accumulated so far. Returns false when a canvas is gone.
  auto flushFrame = [&]() -> bool {
    if (nLayer[0] + nLayer[1] + nLayer[2] == 0) return true;
    Long64_t frameEnd = frameBegin + nEntriesInFrame - 1;
    bool alive = true;
    for (auto& v : views) {
      if (!v.canvas) continue;
      for (auto& hit : v.hits) {
        v.canvas->cd(hit.ipad + 1);
        TMarker* m = new TMarker(hit.x, hit.y, kFullCircle);
        m->SetMarkerColor(hit.color);
        m->SetMarkerSize(kMarkerSize);
        m->Draw();
        v.drawn.push_back({hit.ipad, m});
      }
      v.canvas->SetTitle(Form("%s  frame %llu", v.name, (unsigned long long)currentFrame));
      v.canvas->Modified();
      v.canvas->Update();
      if (!gROOT->GetListOfCanvases()->FindObject(v.canvas->GetName())) alive = false;
    }
    gSystem->ProcessEvents();
    cout << "frame " << currentFrame
         << "  entries " << frameBegin << "-" << frameEnd
         << "  Nhit " << nHitsInFrame;
    for (int i = 0; i < 3; ++i) {
      if (showLayer[i]) cout << "  L" << (i + 1) << " " << nLayer[i];
    }
    cout << endl;
    ++shown;
    if (holdMs > 0) gSystem->Sleep(holdMs);
    return alive;
  };

  for (Long64_t entry = 0; entry < nEntries && shown < nShow && !stop; ++entry) {
    tree->GetEntry(entry);
    if (inFrame && frameId != currentFrame) {
      if (nSkipped < nSkip) {
        ++nSkipped;
        clearFrame();
        if (nSkipped == nSkip) {
          for (const char* b : keep) {
            if (tree->GetBranch(b)) tree->SetBranchStatus(b, 1);
          }
          tree->GetEntry(entry);
        }
      } else {
        if (!flushFrame()) { stop = true; break; }
        if (shown >= nShow) break;
        clearFrame();
      }
    }
    if (!inFrame || frameId != currentFrame) {
      currentFrame = frameId;
      frameBegin = entry;
      inFrame = true;
    }
    ++nEntriesInFrame;
    nHitsInFrame += nHit;

    if (nSkipped < nSkip) continue;
    size_t n = 0;
    if (pixelId) n = pixelId->size();
    else if (phit) n = phit->size();
    for (size_t i = 0; i < n; ++i) {
      PixHit hit;
      if (pixelId && i < pixelId->size()) {
        uint32_t ts = (timestamp && i < timestamp->size()) ? (*timestamp)[i] : 0;
        int t = (tot && i < tot->size()) ? (*tot)[i] : 0;
        hit = decodeHit((*pixelId)[i], ts, t);
      } else {
        hit = decodePhit((*phit)[i]);
      }
      if (hit.station != 0 || hit.layer < 0 || hit.layer > 2) continue;
      if (!showLayer[hit.layer]) continue;
      if (hit.col < 0 || hit.col > 255 || hit.row < 0 || hit.row > 249) continue;
      LayerView& v = views[hit.layer];
      int ichip = hit.chip - v.chipLo;
      if (hit.ladder < 0 || hit.ladder >= v.nLadder) continue;
      if (ichip < 0 || ichip >= v.nChip) continue;
      int ipad = hit.ladder * v.nChip + ichip;
      v.hits.push_back({ipad, hit.col + 0.5, hit.row + 0.5, kTotColor[totColor(hit.tot)]});
      ++nLayer[hit.layer];
    }
  }
  if (!stop && inFrame && nSkipped >= nSkip && shown < nShow) flushFrame();
  if (nSkip > 0 && nSkipped < nSkip) {
    if (inFrame) ++nSkipped;
    cout << "tree has " << nSkipped << " frame" << (nSkipped == 1 ? "" : "s") << endl;
  } else if (nSkip > 0) {
    cout << "skipped " << nSkipped << " frames" << endl;
  }

  cout << "showed " << shown << " frames" << endl;
  bool anyCanvas = false;
  for (auto& v : views) {
    if (v.canvas && gROOT->GetListOfCanvases()->FindObject(v.canvas->GetName())) anyCanvas = true;
  }
  if (shown > 0 && anyCanvas) app.Run();
  return 0;
}