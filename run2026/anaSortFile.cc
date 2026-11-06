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

#include "TCanvas.h"
#include "TStyle.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TMath.h"
#include "TKey.h"
#include "TROOT.h"
#include "TSystem.h"

#include <map>
#include <cstdint>

using namespace std;

// ----------------------------------------------------------------------
// anaSortFile
// -----------
// Occupancy versus frame index.
// A frame is every tree entry that shares one frameId.
// Each histogram bin covers -b frames. Its content is
// (hits in that bin) / (frames in the bin).
// Pixels are per ladder. Fibres (fhit) and tiles (thit) are one count per frame.
//
// Examples:
//          ./bin/anaSortFile ~/data/mu3e/run2026/sort/v04/012/run12272-sorted.root
//          ./bin/anaSortFile -l 1,2 -b 5000 <sortfile>
//
// hit_pixelid is
//   row = id & 0xFF, col = (id >> 8) & 0xFF, sensor = (id >> 16) & 0xFFFF
// and the sensor word is station<<12 | layer<<10 | ladder<<5 | chip.
// phit packs the same id in bits 61..32, ToT in 31..27 and the hit time in 26..0.
// L1: 8 ladders, chips 1-6. L2: 10 ladders, chips 1-6. L3: 24 ladders, chips 7-23.
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

struct LayerGeom {
  const char* name;
  int nLadder;
  int chipLo;
  int nChip;
};

static const LayerGeom kLayer[3] = {
  {"L1",  8, 1,  6},
  {"L2", 10, 1,  6},
  {"L3", 24, 7, 17}
};

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
static int runNumberFromPath(const string& path) {
  string base = path;
  size_t slash = base.rfind('/');
  if (slash != string::npos) base = base.substr(slash + 1);
  size_t r = base.find("run");
  if (r == string::npos) return -1;
  r += 3;
  int n = 0;
  bool any = false;
  while (r < base.size() && base[r] >= '0' && base[r] <= '9') {
    any = true;
    n = n * 10 + (base[r] - '0');
    ++r;
  }
  return any ? n : -1;
}

// ----------------------------------------------------------------------
static string pdfName(int run, const string& tag) {
  if (run >= 0) return Form("run%d-%s.pdf", run, tag.c_str());
  return Form("%s.pdf", tag.c_str());
}

// ----------------------------------------------------------------------
static void styleOcc(TH1F* h, double titleSize, double labelSize) {
  h->SetDirectory(0);
  h->SetXTitle("frame index");
  h->SetYTitle("hits / frame");
  h->SetTitleSize(titleSize + 0.01);
  h->GetXaxis()->SetTitleSize(titleSize);
  h->GetYaxis()->SetTitleSize(titleSize);
  h->GetXaxis()->SetLabelSize(labelSize);
  h->GetYaxis()->SetLabelSize(labelSize);
  h->GetXaxis()->SetTitleOffset(0.95);
  h->GetYaxis()->SetTitleOffset(0.85);
  h->SetLineColor(kBlue + 1);
  h->SetMinimum(0.01);
  h->SetMaximum(100.);
}

// ----------------------------------------------------------------------
int main(int argc, char** argv) {
  string sortfile;
  int binWidth = 1000;
  bool showLayer[3] = {true, true, true};
  bool layersSet = false;

  for (int i = 1; i < argc; ++i) {
    string arg = argv[i];
    if (arg == "-l") {
      if (i + 1 >= argc) {
        cout << "Usage: " << argv[0] << " [-l 1,2,3] [-b frames] <sortfile>" << endl;
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
    } else if (arg == "-b") {
      if (i + 1 >= argc) {
        cout << "Usage: " << argv[0] << " [-l 1,2,3] [-b frames] <sortfile>" << endl;
        return 1;
      }
      binWidth = atoi(argv[++i]);
      if (binWidth <= 0) {
        cout << "Error: -b expects a positive number of frames per bin" << endl;
        return 1;
      }
    } else if (sortfile.empty()) {
      sortfile = arg;
    } else {
      cout << "Error: unexpected argument " << arg << endl;
      cout << "Usage: " << argv[0] << " [-l 1,2,3] [-b frames] <sortfile>" << endl;
      return 1;
    }
  }
  if (sortfile.empty() || (layersSet && !showLayer[0] && !showLayer[1] && !showLayer[2])) {
    cout << "Usage: " << argv[0] << " [-l 1,2,3] [-b frames] <sortfile>" << endl;
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
  if (tree->GetBranch("frameId")) tree->SetBranchStatus("frameId", 1);
  bool havePixel = tree->GetBranch("hit_pixelid") != 0;
  bool havePhit = tree->GetBranch("phit") != 0;
  if (havePixel) tree->SetBranchStatus("hit_pixelid", 1);
  else if (havePhit) tree->SetBranchStatus("phit", 1);
  bool haveFhit = tree->GetBranch("fhit") != 0;
  bool haveThit = tree->GetBranch("thit") != 0;
  if (haveFhit) tree->SetBranchStatus("fhit", 1);
  if (haveThit) tree->SetBranchStatus("thit", 1);

  ULong64_t frameId = 0;
  vector<ULong64_t>* phit = 0;
  vector<unsigned int>* pixelId = 0;
  vector<ULong64_t>* fhit = 0;
  vector<ULong64_t>* thit = 0;
  if (tree->GetBranch("frameId")) tree->SetBranchAddress("frameId", &frameId);
  if (havePixel) tree->SetBranchAddress("hit_pixelid", &pixelId);
  else if (havePhit) tree->SetBranchAddress("phit", &phit);
  if (haveFhit) tree->SetBranchAddress("fhit", &fhit);
  if (haveThit) tree->SetBranchAddress("thit", &thit);

  // sumHits[layer][ladder][bin] is the hit count in that group of frames.
  vector<long long> sumHits[3][24];
  vector<long long> sumFhit, sumThit;
  vector<int> nFramesInBin;
  int frameHits[3][24] = {{0}};
  int frameFhit = 0;
  int frameThit = 0;
  int framesInBin = 0;
  long long nFrames = 0;
  ULong64_t currentFrame = 0;
  bool inFrame = false;

  auto commitFrame = [&]() {
    if (framesInBin == 0) {
      nFramesInBin.push_back(0);
      for (int il = 0; il < 3; ++il) {
        if (!showLayer[il]) continue;
        for (int lad = 0; lad < kLayer[il].nLadder; ++lad) sumHits[il][lad].push_back(0);
      }
      sumFhit.push_back(0);
      sumThit.push_back(0);
    }
    int b = (int)nFramesInBin.size() - 1;
    nFramesInBin[b]++;
    for (int il = 0; il < 3; ++il) {
      if (!showLayer[il]) continue;
      for (int lad = 0; lad < kLayer[il].nLadder; ++lad) {
        sumHits[il][lad][b] += frameHits[il][lad];
        frameHits[il][lad] = 0;
      }
    }
    sumFhit[b] += frameFhit;
    sumThit[b] += frameThit;
    frameFhit = 0;
    frameThit = 0;
    framesInBin++;
    if (framesInBin == binWidth) framesInBin = 0;
    ++nFrames;
    if (nFrames % 200000 == 0) cout << "frames " << nFrames << endl;
  };

  Long64_t nEntries = tree->GetEntries();
  for (Long64_t entry = 0; entry < nEntries; ++entry) {
    tree->GetEntry(entry);
    if (inFrame && frameId != currentFrame) commitFrame();
    if (!inFrame || frameId != currentFrame) {
      currentFrame = frameId;
      inFrame = true;
    }

    size_t n = 0;
    if (pixelId) n = pixelId->size();
    else if (phit) n = phit->size();
    for (size_t i = 0; i < n; ++i) {
      PixHit hit = pixelId ? decodeHit((*pixelId)[i], 0, 0) : decodePhit((*phit)[i]);
      if (hit.station != 0 || hit.layer < 0 || hit.layer > 2) continue;
      if (!showLayer[hit.layer]) continue;
      const LayerGeom& g = kLayer[hit.layer];
      if (hit.ladder < 0 || hit.ladder >= g.nLadder) continue;
      int ichip = hit.chip - g.chipLo;
      if (ichip < 0 || ichip >= g.nChip) continue;
      ++frameHits[hit.layer][hit.ladder];
    }
    if (fhit) frameFhit += (int)fhit->size();
    if (thit) frameThit += (int)thit->size();
  }
  if (inFrame) commitFrame();

  int nBins = (int)nFramesInBin.size();
  cout << "frames " << nFrames << "  bin " << binWidth << "  bins " << nBins << endl;
  if (nBins == 0) return 0;

  gROOT->SetBatch(true);
  gStyle->SetOptStat(0);

  int run = runNumberFromPath(sortfile);
  const int nCol[3] = {2, 2, 4};
  const int nRow[3] = {4, 5, 6};
  for (int il = 0; il < 3; ++il) {
    if (!showLayer[il]) continue;
    const LayerGeom& g = kLayer[il];
    TCanvas* c = new TCanvas(Form("c%s", g.name),
                             Form("%s  hits/frame  (%d frames/bin)", g.name, binWidth),
                             900, 1000);
    c->Divide(nCol[il], nRow[il]);
    for (int lad = 0; lad < g.nLadder; ++lad) {
      int ladderNo = lad + 1;
      TH1F* h = new TH1F(Form("h%s_lad%d", g.name, ladderNo),
                         Form("%s ladder %d", g.name, ladderNo),
                         nBins, 0., nBins * (double)binWidth);
      styleOcc(h, 0.07, 0.06);
      for (int b = 0; b < nBins; ++b) {
        double occ = nFramesInBin[b] > 0 ? double(sumHits[il][lad][b]) / nFramesInBin[b] : 0.;
        h->SetBinContent(b + 1, occ);
      }
      c->cd(ladderNo);
      gPad->SetLeftMargin(0.16);
      gPad->SetBottomMargin(0.16);
      gPad->SetRightMargin(0.04);
      gPad->SetTopMargin(0.10);
      gPad->SetLogy(1);
      h->Draw("HIST");
    }
    string pdf = pdfName(run, string(g.name) + "-phit-vs-frame");
    c->Print(pdf.c_str());
    cout << "wrote " << pdf << endl;
  }

  auto drawOne = [&](const char* tag, const vector<long long>& sum) {
    TCanvas* c = new TCanvas(Form("c%s", tag),
                             Form("%s  hits/frame  (%d frames/bin)", tag, binWidth),
                             800, 600);
    TH1F* h = new TH1F(Form("h%s", tag), tag,
                       nBins, 0., nBins * (double)binWidth);
    styleOcc(h, 0.05, 0.045);
    for (int b = 0; b < nBins; ++b) {
      double occ = nFramesInBin[b] > 0 ? double(sum[b]) / nFramesInBin[b] : 0.;
      h->SetBinContent(b + 1, occ);
    }
    c->SetLeftMargin(0.12);
    c->SetBottomMargin(0.12);
    c->SetLogy(1);
    h->Draw("HIST");
    string pdf = pdfName(run, string(tag) + "-vs-frame");
    c->Print(pdf.c_str());
    cout << "wrote " << pdf << endl;
  };
  drawOne("fhit", sumFhit);
  drawOne("thit", sumThit);
  return 0;
}
