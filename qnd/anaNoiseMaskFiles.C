#include "../common/json.h"

using namespace::std;

// ----------------------------------------------------------------------
struct sensor {
  int layer, localLadder, simLadder,  confLadder, simChip, runChip, ladderChip,  direction; 
  TVector3 v;
};

map<int, struct sensor> gDetectorChips;
map<int, vector<pair<int, int> > > gChipNoisyPixels; 


string gJSON("../common/sensors_mapping_220531.json"); 

// ----------------------------------------------------------------------
bool skipChip(int chipID) {

  if (chipID > 119) return true;
  
  // https://mattermost.gitlab.rlp.net/mu3e/pl/qk3t7i7t53gqubqbucjpggzdna
  vector<int> skipList = {
    54, 55, 56, 57, 58, 59,
    114, 115, 116, 117, 118, 119,
    // Layer0:
    62, 68, 10, 11, 15, 20,
    // Layer1:
    95, 36, 96, 101,
    // 3 links not working:
    // (Mask all runs)
    // Layer0:
    71, 70, 69, 12, 13, 14, 16, 17, 81,
    // Layer1:
    27, 34, 106, 50, 52,
    // 3 link errors:
    // (Mask for runs so far?)
    // Layer0:
    5, 78,
    // Layer1:
    41
  };

  if (skipList.end() != find(skipList.begin(), skipList.end(), chipID)) return true;

  return false; 
}

// ----------------------------------------------------------------------
int getValInt(string line) {
  replaceAll(line, ",", "");
  replaceAll(line, " ", "");
  size_t icol = line.rfind(":");
  string snum = line.substr(icol+1);
  cout << "int  snum ->" << snum << "<-" << endl;
  return atoi(snum.c_str());
}

// ----------------------------------------------------------------------
float getValFloat(string line) {
  replaceAll(line, ",", "");
  replaceAll(line, " ", "");
  size_t icol = line.rfind(":");
  string snum = line.substr(icol+1);
  cout << "float snum ->" << snum << "<-" << endl;
  return atof(snum.c_str());
}

// ----------------------------------------------------------------------
vector<string> readEntry(vector<string> lines, int &iLine) {
  cout << "reading from line " << iLine << endl;
  vector<string> result;
  // -- counters for opening and closing braces
  int ibrace(0); 
  // -- start from indicated iLine
  for (unsigned int i = iLine; i < lines.size(); ++i) {
    if (string::npos != lines[i].find("{")) ++ibrace;
    if (string::npos != lines[i].find("}")) --ibrace;
    result.push_back(lines[i]); 
    if (0 == ibrace) {
      iLine = i + 1;
      break;
    }
  }

  return result;
  
}


// ----------------------------------------------------------------------
struct sensor fillEntry(vector<string> lines) {
  struct sensor chip; 
  for (unsigned int i = 0; i < lines.size(); ++i) {
    if (string::npos != lines[i].find("runChip")) {
      chip.runChip = getValInt(lines[i]);
    }
    if (string::npos != lines[i].find("layer")) {
      chip.layer = getValInt(lines[i]);
    }
    if (string::npos != lines[i].find("v")) {
      chip.v.SetX(getValFloat(lines[i+1]));
      chip.v.SetY(getValFloat(lines[i+2]));
      chip.v.SetZ(getValFloat(lines[i+3]));
    }
  }
  return chip;
}


// ----------------------------------------------------------------------
void readJSON(string filename, string dir = ".") {
  vector<string> allLines; 
  ifstream INS;
  string sline;
  INS.open(filename);
  while (getline(INS, sline)) {
    allLines.push_back(sline);
  }   
  cout << "read " << allLines.size() << " lines" << endl;
  
  int iLine(1);
  vector<string> sentry = readEntry(allLines, iLine); 
  struct sensor chip; 
  while (sentry.size() > 0) {
    chip = fillEntry(sentry); 
    gDetectorChips.insert(make_pair(chip.runChip, chip));
    sentry = readEntry(allLines, iLine);
    if (iLine == allLines.size() - 1) break;
  }
  chip = fillEntry(sentry);
  gDetectorChips.insert(make_pair(chip.runChip, chip));
  
  TH2D *hl0 = new TH2D("hl0", "inner layer", 6, -42., 63., 8, -3.15, 3.15);
  TH2D *hl1 = new TH2D("hl1", "outer layer", 6, -42., 63., 10, -3.15, 3.15);

  for (map<int, struct sensor>::iterator it = gDetectorChips.begin(); it != gDetectorChips.end(); ++it) {
    int layer = (it->second.v.Perp() > 40.? 1:0);
    cout << it->second.runChip << ": v = (" << it->second.v.X() << ", " << it->second.v.Y() << ", " << it->second.v.Z() << ")"
         << " phi = " << it->second.v.Phi()
         << " r = " << it->second.v.Perp()
         << endl;
    if (0 == layer) {
      if (1 == it->second.layer) {
        cout << "XXXXXXXXXXXXXXX layer mismatch" << endl;
      }
      hl0->Fill(it->second.v.Z(), it->second.v.Phi(), it->second.runChip);
    } else {
      if (0 == it->second.layer) {
        cout << "XXXXXXXXXXXXXXX layer mismatch" << endl;
      }
      hl1->Fill(it->second.v.Z(), it->second.v.Phi(), it->second.runChip);
    }
  }

  gStyle->SetOptStat(0);
  hl0->SetMinimum(-1.);
  hl0->Draw("textcol");
  c0.SaveAs(Form("%s/l0.pdf", dir.c_str()));
  hl1->Draw("textcol");
  c0.SaveAs(Form("%s/l1.pdf", dir.c_str()));
  
}


// ----------------------------------------------------------------------
bool validNoise(const vector<uint8_t> &v) {
  for (unsigned int i = 0; i < v.size(); ++i) {
    if (0 != v[i]) {
      return true;
    }
  }
  return false;
}


// ----------------------------------------------------------------------
bool badLVDS(const vector<uint8_t> &v) {
  bool badLVDS(true);
  if (validNoise(v)) {
    if (0 == v[0xff]) badLVDS = false;
  }
  return badLVDS;
}


// ----------------------------------------------------------------------
vector<uint8_t> readFile(string filename) {
  // -- open the file
  streampos fileSize;
  ifstream file;
  file.open(filename.c_str(), std::ios::binary);
  if (!file) {
    cout << "file ->" << filename << "<- not found, skipping" << endl;
    vector<uint8_t> fileData = {};
    return fileData;
  }
  
  // -- get its size
  file.seekg(0, std::ios::end);
  fileSize = file.tellg();
  file.seekg(0, std::ios::beg);
  
  // -- read the data
  vector<uint8_t> fileData(fileSize);
  file.read((char*) &fileData[0], fileSize);

  file.close();
  
  return fileData;
}

// ----------------------------------------------------------------------
pair<int, int> colrowFromIdx(int idx) {
  int col = idx/256;
  int row = idx%256;
  return make_pair(col, row);
}

// ----------------------------------------------------------------------
int idxFromColRow(int col, int row) {
  int idx = col*256 + row;
  return idx; 
}


// ----------------------------------------------------------------------
// -- produce summary of a mask file
// ----------------------------------------------------------------------
void summarize(vector<uint8_t> vnoise) {
  for (unsigned int i = 0; i < vnoise.size(); ++i){
    if ((0xda == vnoise[i]) && (0xda == vnoise[i+1])) {
      i += 5; //??
      continue;
    }
    pair<int, int> a = colrowFromIdx(i);
    if (0 != vnoise[i]) {
      cout << Form("pix: %d/%d ", a.first, a.second);
    }
  }
  cout  << endl;
}


// ----------------------------------------------------------------------
// -- produce summary of a mask file
// ----------------------------------------------------------------------
void summaryMaskFile(string filename) {
  vector<uint8_t> vnoise = readFile(filename);
  cout << filename << ": ";
  summarize(vnoise);
}


// ----------------------------------------------------------------------
// -- fill gChipNoisyPixels for a chipID
// ----------------------------------------------------------------------
void fillNoisyPixels(int chipID, vector<uint8_t> &vnoise,
                     map<int, vector<pair<int, int> > > &map1) {
  vector<pair<int, int> > vnp;

  for (unsigned int i = 0; i < vnoise.size(); ++i){
    if ((0xda == vnoise[i]) && (0xda == vnoise[i+1])) {
      i += 5; //??
      continue;
    }
    if (0xff != vnoise[i]) {
      pair<int, int> a = colrowFromIdx(i);
      vnp.push_back(a);
    }
  }
  map1.insert(make_pair(chipID, vnp));  
}


// ----------------------------------------------------------------------
void noisyPixelsPerRun(string dir = "nmf", bool checkLVDS = true, string name = "noiseMaskFileMay") {

  map<int, vector<pair<int, int> > > map1;
  //  vector<int> runlist = {311, 332, 347}; 

  // vector<int> runlist = {
  //   311, 312, 313,
  //   320, 321, 322, 323, 325,
  //   332, 333, 334, 336, 337,
  //   341, 343, 345, 346, 347, 348
  // };

  vector<int> runlist = {
    311, 312, 313,
    320, 321, 322, 323, 325,
    332, 333, 334, 336, 337,
    341, 343, 345, 346, 347, 348,
    350, 352, 353, 354, 355, 356, 357, 358, 359,
    360, 362, 363, 364, 365, 366
  };

  TH2D *hnmap = new TH2D("hnmap", Form("noisy pixels/chip %s", (checkLVDS?"(no LVDS errors)":"")),
                         120, 0., 120., runlist.size(), 0., runlist.size());
  
  for (int irun = 0; irun < runlist.size(); ++irun) {
    hnmap->GetYaxis()->SetBinLabel(irun+1, Form("%d", runlist[irun]));
    map1.clear();
    for (int i = 0; i < 120; ++i) {
      if (skipChip(i)) continue;
      
      vector<uint8_t> vnoise = readFile(Form("%s/%s-run%d-chipID%d", dir.c_str(), name.c_str(), runlist[irun], i));
      if (checkLVDS && badLVDS(vnoise)) continue;
      if (validNoise(vnoise)) {
        fillNoisyPixels(i, vnoise, map1);
      }
    }

    map<int, vector<pair<int, int> > >::iterator it; 
    for (it = map1.begin(); it != map1.end(); ++it) {
      int npix1 = it->second.size(); 
      hnmap->Fill(it->first, irun, npix1);
    }
  }

  gStyle->SetOptStat(0);
  gPad->SetLogz(1);
  shrinkPad(0.1, 0.1, 0.15);
  hnmap->GetXaxis()->SetTitle("chipID");
  hnmap->GetYaxis()->SetTitle("run number");
  hnmap->Draw("colz");
  string pdfname = Form("noisyPixelsPerRun%s-%s.pdf", (checkLVDS?"-noLVDSerrors":""), name.c_str());
  c0.SaveAs(Form("%s/%s", dir.c_str(), pdfname.c_str())); 
}


// ----------------------------------------------------------------------
void cmpNoiseMasks(string dir = "nmf",
                   string name1 = "noiseMaskFile",
                   string name2 = "noiseMaskFile-run352",
                   string pdfname = "diff.pdf") {
  gStyle->SetHistMinimumZero();
  int OK(0); 

  map<int, vector<pair<int, int> > > map1;
  map<int, vector<pair<int, int> > > map2;

  readJSON(gJSON, dir);
  
  for (int i = 0; i < 120; ++i) {
    if (skipChip(i)) continue;
    vector<uint8_t> vnoise = readFile(Form("%s/%s-chipID%d", dir.c_str(), name1.c_str(), i));
    if (badLVDS(vnoise)) continue;
    if (validNoise(vnoise)) {
      fillNoisyPixels(i, vnoise, map1);
    }
  }
  
  for (int i = 0; i < 120; ++i) {
    if (skipChip(i)) continue;
    vector<uint8_t> vnoise = readFile(Form("%s/%s-chipID%d", dir.c_str(), name2.c_str(), i));
    if (badLVDS(vnoise)) continue;
    if (validNoise(vnoise)) {
      fillNoisyPixels(i, vnoise, map2);
    }
  }

  double hMax(20000.);
  TH1D *h1 = new TH1D("hnmap1", Form("noisy pixels/chips (%s)", name1.c_str()), 100, 0., hMax);
  TH1D *h2 = new TH1D("hnmap2", Form("noisy pixels/chips (%s)", name2.c_str()), 100, 0., hMax);
  TH1D *h0 = new TH1D("hndiff", Form("difference (%s - %s)", name2.c_str(), name1.c_str()), 100, -5000., 5000.);
  h0->SetNdivisions(508, "X");
  
  TH1D *hn1 = new TH1D("hn1", "", 120, 0., 120.);
  hn1->SetLineColor(kBlack);
  hn1->SetLineWidth(2);
  TH1D *hn2 = new TH1D("hn2", "", 120, 0., 120.);
  hn2->SetLineColor(kRed);
  hn2->SetLineWidth(2);

  map<int, vector<pair<int, int> > >::iterator it; 
  for (it = map1.begin(); it != map1.end(); ++it) {
    int npix1 = it->second.size(); 
    hn1->Fill(it->first, npix1);
    h1->Fill(npix1);
    int npix2(npix1);
    if (map2.find(it->first) != map2.end()) {
      npix2 = map2[it->first].size();
      hn2->Fill(it->first, npix2);
      h2->Fill(npix2);
      h0->Fill(npix2-npix1);
    } else {
      cout << it->first << " not found in map2" << endl;
    }
    if (npix1 - npix2 != 0) cout << "npix1 = " << npix1 << " npix2 = " << npix2 << endl;
  }

  gPad->SetLogy(1);
  hn1->SetMinimum(0.5);
  hn1->Draw("hist");  
  hn2->Draw("histsame");  

  tl->SetTextSize(0.05);
  tl->SetTextColor(kBlack);
  tl->DrawLatexNDC(0.1, 0.94, "noisy pixels/chip");
  tl->SetTextSize(0.04);
  tl->SetTextColor(kBlack);
  tl->DrawLatexNDC(0.47, 0.95, name1.c_str());
  tl->SetTextColor(kRed);
  tl->DrawLatexNDC(0.47, 0.91, name2.c_str());
  
  c0.SaveAs(Form("%s/%s", dir.c_str(), pdfname.c_str())); 

  gPad->SetLogy(0);
  gStyle->SetOptStat(1);
  //  gROOT->ForceStyle();
  h0->SetMinimum(0.);
  h0->Draw();
  tl->SetTextSize(0.03);
  tl->SetTextColor(kBlack);
  tl->DrawLatexNDC(0.6, 0.85, Form("overflow: "));
  tl->DrawLatexNDC(0.8, 0.85, Form("%5.1f", h0->GetBinContent(h0->GetNbinsX())));
  tl->DrawLatexNDC(0.6, 0.80, Form("underflow:"));
  tl->DrawLatexNDC(0.8, 0.80, Form("%5.1f", h0->GetBinContent(0)));


  string dname = "diff-" + pdfname;  
  c0.SaveAs(Form("%s/%s", dir.c_str(), dname.c_str())); 
  
}

// ----------------------------------------------------------------------
void fillAllNoisyPixels(string dir = "nmf", bool checkLVDS = false, string name = "noiseMaskFileMay") {
  gStyle->SetHistMinimumZero();
  int OK(0); 
  gChipNoisyPixels.clear();

  readJSON(gJSON, dir);
  
  for (int i = 0; i < 120; ++i) {
    if (skipChip(i)) continue;
    vector<uint8_t> vnoise = readFile(Form("%s/%s-chipID%d", dir.c_str(), name.c_str(), i));
    if (checkLVDS && badLVDS(vnoise)) continue;
    if (validNoise(vnoise)) {
      fillNoisyPixels(i, vnoise, gChipNoisyPixels);
    }
  }

  int noiseMax(-1), npix(0);
  map<int, vector<pair<int, int> > >::iterator it; 
  for (it = gChipNoisyPixels.begin(); it != gChipNoisyPixels.end(); ++it) {
    npix = it->second.size(); 
    cout << "chip " << it->first << " it->size() = " << npix << endl;
    if (npix > noiseMax) {
      cout << "noiseMax = " << noiseMax << " -> " << npix << endl;
      noiseMax = npix;
    }
  }

  int hMax = (noiseMax/10000 + 1)*10000;
  cout << "noiseMax = " << noiseMax << " -> " << hMax << endl;

  TH2D *hl0 = new TH2D("hl0n", Form("noisy pixels inner layer %s", (checkLVDS?"(no LVDS errors)":"")),
                       6, -42., 63., 8, -3.15, 3.15);
  TH2D *hl1 = new TH2D("hl1n", Form("noisy pixels outer layer %s", (checkLVDS?"(no LVDS errors)":"")),
                       6, -42., 63., 10, -3.15, 3.15);

  TH2D *hChip = new TH2D("hChip", Form("noisy pixels %s", (checkLVDS?"(no LVDS errors)":"")),
                         256, 0., 256., 250, 0., 250.);
  TH2D *hChipClean = new TH2D("hChipClean", Form("noisy pixels 'clean' %s", (checkLVDS?"(no LVDS errors)":"")),
                         256, 0., 256., 250, 0., 250.);

  
  TH1D *h1 = new TH1D("hnoise", Form("noisy pixels/chip %s", (checkLVDS?"(no LVDS errors)":"")),
                      100, 0., hMax);
  h1->SetNdivisions(508, "X");
  for (it = gChipNoisyPixels.begin(); it != gChipNoisyPixels.end(); ++it) {
    //    cout << "chip " << it->first << " it->size() = " << it->second.size() << endl;
    h1->Fill(it->second.size());
    for (unsigned int ipix = 0; ipix < it->second.size(); ++ipix) {
      hChip->Fill(it->second[ipix].first, it->second[ipix].second); 
      if (it->second.size() < 1000) {
        hChipClean->Fill(it->second[ipix].first, it->second[ipix].second); 
      }
    }
    
    if (0 == gDetectorChips[it->first].layer) {
      hl0->Fill(gDetectorChips[it->first].v.Z(), gDetectorChips[it->first].v.Phi(), it->second.size());
    }

    if (1 == gDetectorChips[it->first].layer) {
      hl1->Fill(gDetectorChips[it->first].v.Z(), gDetectorChips[it->first].v.Phi(), it->second.size());
    }
  }
  hpl(h1, "fillblue");
  tl->SetTextSize(0.05);
  tl->DrawLatexNDC(0.4, 0.85, Form("mean: "));
  tl->DrawLatexNDC(0.7, 0.85, Form("%5.1f", h1->GetMean()));
  tl->DrawLatexNDC(0.4, 0.80, Form("RMS: "));
  tl->DrawLatexNDC(0.7, 0.80, Form("%5.1f", h1->GetRMS()));
  tl->DrawLatexNDC(0.4, 0.75, Form("overflow:"));
  tl->DrawLatexNDC(0.7, 0.75, Form("%5.1f", h1->GetBinContent(h1->GetNbinsX())));
  
  if (dir == ".") {
    c0.SaveAs("nNoisyPixels.pdf");
  } else {
    string pdfname = Form("nNoisyPixels%s-%s.pdf", (checkLVDS?"-noLVDSerrors":""), name.c_str());
    c0.SaveAs(Form("%s/%s", dir.c_str(), pdfname.c_str()));
  }

  hl0->Draw("coltext");
  if (dir == ".") {
    c0.SaveAs("nNoisyPixels-zphi-l0.pdf");
  } else {
    string pdfname = Form("nNoisyPixels-zphi-l0%s-%s.pdf", (checkLVDS?"-noLVDSerrors":""), name.c_str());
    c0.SaveAs(Form("%s/%s", dir.c_str(), pdfname.c_str()));
  }

  hl1->Draw("coltext");
  if (dir == ".") {
    c0.SaveAs("nNoisyPixels-zphi-l1.pdf");
  } else {
    string pdfname = Form("nNoisyPixels-zphi-l1%s-%s.pdf", (checkLVDS?"-noLVDSerrors":""), name.c_str());
    c0.SaveAs(Form("%s/%s", dir.c_str(), pdfname.c_str()));
  }


  hChip->Draw("colz");
  if (dir == ".") {
    c0.SaveAs("mapNoisyPixels.pdf");
  } else {
    string pdfname = Form("mapNoisyPixels%s-%s.pdf", (checkLVDS?"-noLVDSerrors":""), name.c_str());
    c0.SaveAs(Form("%s/%s", dir.c_str(), pdfname.c_str()));
  }


  hChipClean->Draw("colz");
  if (dir == ".") {
    c0.SaveAs("mapNoisyPixelsClean.pdf");
  } else {
    string pdfname = Form("mapNoisyPixelsClean%s-%s.pdf", (checkLVDS?"-noLVDSerrors":""), name.c_str()); 
    c0.SaveAs(Form("%s/%s", dir.c_str(), pdfname.c_str()));
  }
}




// ----------------------------------------------------------------------
void compareNsig() {
  int OK(0); 

  gStyle->SetOptStat(0);
  
  vector<string> dirs = {"msig20", "msig10", "msig5"};
  TH1D *h1 = new TH1D("hnoise", "noisy pixels/chips", 50, 0., 2000.);
  TH1D *hs = new TH1D("hsummary", "average number of noisy pixels/chips", dirs.size(), 0., dirs.size());
  cout << "dirs.size() = " << dirs.size() << endl;
  
  for (unsigned int idir = 0; idir < dirs.size(); ++idir) {
    gChipNoisyPixels.clear();
    h1->Reset();
    string blabel = dirs[idir];
    replaceAll(blabel, "msig", "msig=");
    
    hs->GetXaxis()->SetBinLabel(idir+1, blabel.c_str());
    for (int i = 0; i < 120; ++i) {
      if (skipChip(i)) continue;
      vector<uint8_t> vnoise = readFile(Form("%s/noiseMaskFile-chipID%d", dirs[idir].c_str(), i));
      if (badLVDS(vnoise)) continue;
      if (validNoise(vnoise)) {
        fillNoisyPixels(i, vnoise, gChipNoisyPixels);
      }
    }
    
    map<int, vector<pair<int, int> > >::iterator it; 
    for (it = gChipNoisyPixels.begin(); it != gChipNoisyPixels.end(); ++it) {
      cout << "chip " << it->first << " it->size() = " << it->second.size() << endl;
      h1->Fill(it->second.size());
    }
    hs->SetBinContent(idir+1, h1->GetMean());
    cout << "setting hs bin contents " << h1->GetMean() << endl;
  }
  hs->SetMinimum(0.);
  hs->SetMaximum(1.3*hs->GetMaximum());
  hs->Draw();

  tl->SetTextSize(0.03);
  tl->DrawLatex(0.16, 0.85, "noise level #equiv mean(nhit) + msig * RMS(nhit) + 0.5");
  
  c0.SaveAs("meanNumberNoisyPixels-msig.pdf");
}


// ----------------------------------------------------------------------
void plotAll(string dir = "nmf0", string name = "noiseMaskFilenmf0") {
  fillAllNoisyPixels(dir, false, name);
  fillAllNoisyPixels(dir, true, name);

  noisyPixelsPerRun(dir, false, name);
  noisyPixelsPerRun(dir, true, name);

}
