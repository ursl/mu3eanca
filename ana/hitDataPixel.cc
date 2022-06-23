#include "hitDataPixel.hh"
#include "hitDataIncludes.hh"

// ----------------------------------------------------------------------
// Run with: see derived classes!
//
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
hitDataPixel::hitDataPixel(TChain *chain, string treeName): hitDataBase(chain, treeName) {
  cout << "==> hitDataPixel: constructor..." << endl;
  if (chain == 0) {
    cout << "You need to pass a chain!" << endl;
  }
  cout << "==> hitDataPixel: constructor fpChain: " << fpChain << "/" << fpChain->GetName()
       << " entries = " <<   fNentries
       << endl;

  readJSON("../common/sensors_mapping_220531.json", "results"); 
}


// ----------------------------------------------------------------------
hitDataPixel::~hitDataPixel() {
  cout << "==> hitDataPixel: destructor ..." << endl;
  if (!fpChain) return;
  delete fpChain->GetCurrentFile();
}


// ----------------------------------------------------------------------
int hitDataPixel::getValInt(string line) {
  bool DBX(false);
  replaceAll(line, ",", "");
  replaceAll(line, " ", "");
  size_t icol = line.rfind(":");
  string snum = line.substr(icol+1);
  if (DBX) cout << "int  snum ->" << snum << "<-" << endl;
  return atoi(snum.c_str());
}

// ----------------------------------------------------------------------
float hitDataPixel::getValFloat(string line) {
  bool DBX(false);
  replaceAll(line, ",", "");
  replaceAll(line, " ", "");
  size_t icol = line.rfind(":");
  string snum = line.substr(icol+1);
  if (DBX) cout << "float snum ->" << snum << "<-" << endl;
  return atof(snum.c_str());
}

// ----------------------------------------------------------------------
vector<string> hitDataPixel::readEntry(vector<string> lines, int &iLine) {
  bool DBX(false);
  if (DBX)  cout << "reading from line " << iLine << endl;
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
struct sensor hitDataPixel::fillEntry(vector<string> lines) {
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
void hitDataPixel::readJSON(string filename, string dir) {
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
    fDetectorChips.insert(make_pair(chip.runChip, chip));
    sentry = readEntry(allLines, iLine);
    if (iLine == allLines.size() - 1) break;
  }
  chip = fillEntry(sentry);
  fDetectorChips.insert(make_pair(chip.runChip, chip));
  
  TH2D *hl0 = new TH2D("hl0", "inner layer", 6, -42., 63., 8, -3.15, 3.15);
  TH2D *hl1 = new TH2D("hl1", "outer layer", 6, -42., 63., 10, -3.15, 3.15);

  for (map<int, struct sensor>::iterator it = fDetectorChips.begin(); it != fDetectorChips.end(); ++it) {
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

  // gStyle->SetOptStat(0);
  // hl0->SetMinimum(-1.);
  // hl0->Draw("textcol");
  // c0.SaveAs(Form("%s/l0.pdf", dir.c_str()));
  // hl1->Draw("textcol");
  // c0.SaveAs(Form("%s/l1.pdf", dir.c_str()));
  
}


// ----------------------------------------------------------------------
void hitDataPixel::bookHist(int runnumber) {
  static bool first(true);
  hitDataBase::bookHist(runnumber);
  cout << "==> hitDataPixel: bookHist> run " << runnumber << endl;

  bool DBX(false);
  string n("");
  struct hID a; 
  if (first) {
    first = false;
    fTree->Branch("chipID",     &fChipID,  "chipID/I");
    fTree->Branch("layer",      &flayer,   "layer/I");
    fTree->Branch("col",        &fcol,     "col/I");
    fTree->Branch("row",        &frow,     "row/I");
    fTree->Branch("tot",        &ftot,     "tot/I");
    fTree->Branch("tot2",       &ftot2,    "tot2/I");
    fTree->Branch("qual",       &fqual,    "qual/I");
    fTree->Branch("nchip",      &fChipHits,"nchip/I");
    fTree->Branch("nevt",       &fEvtHits, "nevt/I");
    fTree->Branch("fpgaID",     &ffpgaID, "fpgaID/I");
    fTree->Branch("headerTime", &fheaderTime, "headerTime/I");

    // -- create unified hitmap histo
    n = Form("hitmap_run%d", 0);
    a = hID(0, -1, "hitmap");
    fChipHistograms.insert(make_pair(a, new TH2D(n.c_str(), n.c_str(), 256, 0, 256, 250, 0, 250)));
    if (DBX) cout << "book hID: " << a << ": " << n << " ptr: " << fChipHistograms[a] << endl;
    
    n = Form("hittot_run%d", 0);
    a = hID(0, -1, "hit_tot");
    fChipHistograms.insert(make_pair(a, new TH1D(n.c_str(), n.c_str(), 32, 0, 256.)));
    if (DBX) cout << "book hID: " << a << ": " << n << " ptr: " << fChipHistograms[a] << endl;
    
    n = Form("badchiptot_run%d", 0);
    a = hID(0, -1, "badchiptot");
    fChipHistograms.insert(make_pair(a, new TH1D(n.c_str(), n.c_str(), 32, 0, 256.)));
    if (DBX) cout << "book hID: " << a << ": " << n << " ptr: " << fChipHistograms[a] << endl;
    
    n = Form("noisytot_run%d", 0);
    a = hID(0, -1, "noisytot");
    fChipHistograms.insert(make_pair(a, new TH1D(n.c_str(), n.c_str(), 32, 0, 256.)));
    if (DBX) cout << "book hID: " << a << ": " << n << " ptr: " << fChipHistograms[a] << endl;

  }

  for (int i = 0; i < 120; ++i) {
    n = Form("hittot_run%d_chipID%d", runnumber, i);
    a = hID(fRun, i, "hit_tot");
    fChipHistograms.insert(make_pair(a, new TH1D(n.c_str(), n.c_str(), 32, 0, 256.)));
    if (DBX) cout << "book hID: " << a << ": " << n << " ptr: " << fChipHistograms[a] << endl;
    n = Form("hitmap_run%d_chipID%d", runnumber, i);
    a = hID(fRun, i, "hitmap"); 
    fChipHistograms.insert(make_pair(a, new TH2D(n.c_str(), n.c_str(), 256, 0, 256, 250, 0, 250)));
    if (1) cout << "book hID: " << a << ": " << n << " ptr: " << fChipHistograms[a] << endl;
    
  }
  
  // -- create per-run unified histograms
  if (runnumber > 0) {

    for (int i = 0; i < 120; ++i) {
      for (int ij = i; ij < 120; ++ij) {
        n = Form("dt_run%d_chipID%d", runnumber, i*1000+ij);
        a = hID(fRun, i*1000+ij, "dt");
        fCorrelations.insert(make_pair(a, new TH1D(n.c_str(), n.c_str(), 200, -1000., 1000.)));
        if (1) cout << "book hID: " << a << ": " << n << " ptr: " << fCorrelations[a] << endl;
      }
    }

    n = Form("hitmap_run%d", runnumber);
    a = hID(fRun, -1, "hitmap");
    fChipHistograms.insert(make_pair(a, new TH2D(n.c_str(), n.c_str(), 256, 0, 256, 250, 0, 250)));
    if (DBX) cout << "book hID: " << a << ": " << n << " ptr: " << fChipHistograms[a] << endl;
    
    n = Form("hittot_run%d", runnumber);
    a = hID(fRun, -1, "hit_tot");
    fChipHistograms.insert(make_pair(a, new TH1D(n.c_str(), n.c_str(), 32, 0, 256.)));
    if (DBX) cout << "book hID: " << a << ": " << n << " ptr: " << fChipHistograms[a] << endl;
  }
  

  
}


// ----------------------------------------------------------------------
int hitDataPixel::countChipHits(int chipid) {
  int cnt(0); 
  for (int ihit = 0; ihit < fv_col->size(); ++ihit) {
    if (chipid == fv_chipID->at(ihit)) ++cnt;
  }
  return cnt;
}

// ----------------------------------------------------------------------
int hitDataPixel::getLayer(int chipid) {
  int layer(0); 
  static map<int, int> layerCache;
  if (layerCache.end() == layerCache.find(chipid)) {
    for (map<int, struct sensor>::iterator it = fDetectorChips.begin(); it != fDetectorChips.end(); ++it) {
      if (chipid == it->second.runChip) {
        layer = it->second.layer;
        layerCache[chipid] = layer;
        break;
      }
    }
  } else {
    layer = layerCache[chipid];
  }
  return layer;
}


// ----------------------------------------------------------------------
void hitDataPixel::eventProcessing() {
  struct hID adt(fRun, 0, "dt");

  struct hID ahm(fRun, 0, "hitmap");
  struct hID aht(fRun, 0, "hit_tot");
  struct hID ahm0(0, -1, "hitmap");
  struct hID aht0(0, -1, "hit_tot");
  struct hID ahm0r(fRun, -1, "hitmap");
  struct hID aht0r(fRun, -1, "hit_tot");

  struct hID ahn0(0, -1, "noisytot");


  // -- count good hits in this event
  fEvtHits = 0;
  for (int ihit = 0; ihit < fv_col->size(); ++ihit) {
    fChipID = fv_chipID->at(ihit); 
    fheaderTime = fv_headerTime->at(ihit); 
    ffpgaID = fv_fpgaID->at(ihit); 
    if (120 == fChipID) {
      continue;
    }  
    if (fChipQuality[fChipID] > 0) {
      continue;
    }
    
    frow  = fv_row->at(ihit);
    fcol  = fv_col->at(ihit); 

    if (fChipNoisyPixels[fChipID].end() != find(fChipNoisyPixels[fChipID].begin(),
                                                fChipNoisyPixels[fChipID].end(),
                                                make_pair(fcol, frow))) {
      continue;
    } 
    ++fEvtHits;
  }

  for (int ihit = 0; ihit < fv_fpgaID->size(); ++ihit) {
    fChipID = fv_chipID->at(ihit); 
    fheaderTime = fv_headerTime->at(ihit); 
    ffpgaID = fv_fpgaID->at(ihit); 
    if (fChipQuality[fChipID] > 0) {
      continue;
    }
    for (int ijhit = ihit+1; ijhit < fv_fpgaID->size(); ++ijhit) {
      int chipID     = fv_chipID->at(ijhit); 
      int fpgaID2    = fv_fpgaID->at(ijhit); 
      int headerTime = fv_headerTime->at(ijhit); 
      int dt = fheaderTime - headerTime; 
      if (fChipQuality[chipID] > 0) {
        continue;
      }
      if (fpgaID2 == ffpgaID) {
        int cid = fChipID*1000+chipID; 
        if (chipID < fChipID) {
          cid = chipID*1000+fChipID;
        }
        adt.setRunChip(fRun, cid);
        //        cout << adt << " " << fCorrelations[adt] << endl;
        ((TH1D*)fCorrelations[adt])->Fill(dt);
      }
    }
  }
  
  // -- now do real analysis
  for (int ihit = 0; ihit < fv_col->size(); ++ihit) {
    fChipID = fv_chipID->at(ihit); 
    aht.setRunChip(fRun, fChipID);
    
    // -- weed out scintillator
    if (120 == fChipID) {
      continue;
    }  

    fChipHits = countChipHits(fChipID);
    
    frow  = fv_row->at(ihit);
    fcol  = fv_col->at(ihit); 
    ftot  = fv_tot->at(ihit); 
    ftot2 = fv_tot->at(ihit); 
    flayer = getLayer(fChipID);
    
    if (fChipQuality[fChipID] > 0) {
      continue;
    }
    
    if (fChipNoisyPixels[fChipID].end() != find(fChipNoisyPixels[fChipID].begin(),
                                               fChipNoisyPixels[fChipID].end(),
                                               make_pair(fcol, frow))) {
      continue;
    } else {
      
    }
    
    ahm.setRunChip(fRun, fChipID); ((TH2D*)fChipHistograms[ahm])->Fill(fcol, frow);
    ahm.setRunChip(0, -1);        ((TH2D*)fChipHistograms[ahm])->Fill(fcol, frow);
    ahm.setRunChip(fRun, -1);     ((TH2D*)fChipHistograms[ahm])->Fill(fcol, frow);
    ahm.setRunChip(0, fChipID);    ((TH2D*)fChipHistograms[ahm])->Fill(fcol, frow);
    
    aht.setRunChip(fRun, fChipID); ((TH1D*)fChipHistograms[aht])->Fill(ftot);
    aht.setRunChip(0, -1);        ((TH1D*)fChipHistograms[aht])->Fill(ftot);
    aht.setRunChip(fRun, -1);     ((TH1D*)fChipHistograms[aht])->Fill(ftot);
    aht.setRunChip(0, fChipID);    ((TH1D*)fChipHistograms[aht])->Fill(ftot);
    
    //    cout << "filling tree " << fheaderTime << endl;
    fTree->Fill();
    
  }
}


// ----------------------------------------------------------------------
void hitDataPixel::runEndAnalysis(int runnumber) {}


