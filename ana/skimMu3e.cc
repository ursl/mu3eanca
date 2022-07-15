#include "skimMu3e.hh"
#include "hitDataIncludes.hh"

// ----------------------------------------------------------------------
// Run with: see derived classes!
//
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
skimMu3e::skimMu3e(TChain *chain, string treeName): fVerbose(0) {
  cout << "==> skimMu3e: constructor..." << endl;
  if (chain == 0) {
    cout << "You need to pass a chain!" << endl;
  }
  fpChain = chain;
  fChainName = treeName;
  fNentries = fpChain->GetEntries();

  cout << "==> skimMu3e: constructor fpChain: " << fpChain << "/" << fpChain->GetName()
       << " treename = " << fChainName
       << " entries = " <<   fNentries
       << endl;

  setupTree();

  fModeNoiseLimit = 1;
  fNoiseLevel = 1.5;
  fSuffix = ""; 

}


// ----------------------------------------------------------------------
skimMu3e::~skimMu3e() {
  cout << "==> skimMu3e: destructor ..." << endl;
  if (!fpChain) return;
  delete fpChain->GetCurrentFile();
}


// ----------------------------------------------------------------------
pair<int, int> skimMu3e::colrowFromIdx(int idx) {
  int col = idx/256;
  int row = idx%256;
  return make_pair(col, row);
}


// ----------------------------------------------------------------------
int skimMu3e::idxFromColRow(int col, int row) {
  int idx = col*256 + row;
  return idx; 
}


// ----------------------------------------------------------------------
bool skimMu3e::validNoise(const vector<uint8_t> &v) {
  for (unsigned int i = 0; i < v.size(); ++i) {
    if (0 != v[i]) {
      return true;
    }
  }
  return false;
}


// ----------------------------------------------------------------------
bool skimMu3e::badLVDS(const vector<uint8_t> &v) {
  bool badLVDS(true);
  if (validNoise(v)) {
    if (0 == v[0xff]) badLVDS = false;
  }
  return badLVDS;
}


// ----------------------------------------------------------------------
bool skimMu3e::unclean(const vector<uint8_t> &v, int maxNoise) {
  int cnt(0);
  if (validNoise(v)) {
    for (unsigned int i = 0; i < v.size(); ++i) {
      if ((0xda == v[i]) && (0xda == v[i+1])) {
        i += 5; //??
        continue;
      }
      if (0 == v[i]) ++cnt;
    }
    if (cnt < maxNoise) return false;
  }
  return true;
}


// ----------------------------------------------------------------------
void skimMu3e::fillNoisyPixels(int chipID, vector<uint8_t> &vnoise,
                                  map<int, vector<pair<int, int> > > &map1) {
  vector<pair<int, int> > vnp;
  for (unsigned int i = 0; i < vnoise.size(); ++i){
    if ((0xda == vnoise[i]) && (0xda == vnoise[i+1])) {
      i += 5; // skip the trailer
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
vector<uint8_t> skimMu3e::readMaskFile(string filename) {
  // -- open the file
  streampos fileSize;
  ifstream file;
  file.open(filename.c_str(), std::ios::binary);
  if (!file) {
    // cout << "file ->" << filename << "<- not found, skipping" << endl;
    vector<uint8_t> fileData;
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
void skimMu3e::readNoiseMaskFiles(int runnumber, string dir) {
  int maxNoise(1000);
  string name("noiseMaskFile"); 
  fChipNoisyPixels.clear();
  cout << "reading noise mask files for run " << runnumber << endl;
  cout << "chipIDs w/o files: "; 
  for (int i = 0; i < 120; ++i) {
    vector<uint8_t> vnoise;
    if (runnumber > 0) {
      vnoise = readMaskFile(Form("%s/%s-run%d-chipID%d", dir.c_str(), name.c_str(), runnumber, i));
    } else {
      vnoise = readMaskFile(Form("%s/%s-chipID%d", dir.c_str(), name.c_str(), i));
    }
    if (validNoise(vnoise)) {
      fillNoisyPixels(i, vnoise, fChipNoisyPixels);
    } else {
      cout << i << " "; 
    }
    fChipQuality[i] = 0;  // every chip must be in here
    if (skipChip(i))               fChipQuality[i] |= 2; 
    if (badLVDS(vnoise))           fChipQuality[i] |= 4; 
    if (unclean(vnoise, maxNoise)) fChipQuality[i] |= 8; 
  }
  cout << endl;
  
  // -- scintillator 
  fChipQuality[120] = 1;  

  // -- add fake chipID
  for (int i = 121; i < 130; ++i) {
    fChipQuality[i] = 4;  
  }
}

// ----------------------------------------------------------------------
bool skimMu3e::skipChip(int chipID) {
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
void skimMu3e::startAnalysis() {
  cout << "skimMu3e::startAnalysis() wrong class " << endl;
}

// ----------------------------------------------------------------------
void skimMu3e::endAnalysis() {
  cout << "skimMu3e::endAnalysis() wrong class " << endl;
}


// ----------------------------------------------------------------------
void skimMu3e::eventProcessing() {
  int VERBOSE(1);
  // -- count good hits in this event
  for (int ihit = 0; ihit < fv_hit_pixelid->size(); ++ihit) {
    unsigned int pixelId = fv_hit_pixelid->at(ihit); 

    const uint32_t sensorId = pixelId >> 16;
    frow = pixelId & 0xFF;
    fcol = (pixelId >> 8) & 0xFF;

    if (fChipID > 119) {
      if (VERBOSE) cout << Form("LVDS error: col/row/chip = %d/%d/%d", fcol, frow, fChipID)
                        << endl;
      continue;
    }
    if (VERBOSE) cout << Form("col/row/chip = %d/%d/%d", fcol, frow, fChipID)
                      << endl;
    fhitmaps.at(fChipID)->Fill(fcol, frow);
 
    fhTotal->Fill(fChipID);
    if (frow > 249) {
      fhErrors->Fill(fChipID);
    }
    if (fcol > 255) {
      fhErrors->Fill(fChipID);
    }
  }    
 
}



// ----------------------------------------------------------------------
void skimMu3e::fillHist() {
  cout << "skimMu3e::fillHist() wrong class " << endl;
}

// ----------------------------------------------------------------------
void skimMu3e::bookHist(int runnumber) {
  static bool first(true);
  cout << "==> skimMu3e: bookHist> run " << runnumber << endl;

  fpHistFile->cd();

  if (runnumber < 1) return;
  
  stringstream ss;
  if (first) {
    first = false;
    // -- Reduced Tree
    fTree = new TTree("events", "events");
    fTree->Branch("run",      &fRun,       "run/I");
    fTree->Branch("evt",      &fEvt,       "evt/I");
    cout << "==> skimMu3e: bookHist> run " << runnumber << endl;
    
    // -- create one per chipID, irrespective of it is present. Ignore the original setup ("get unique chipID")
    for (int i = 0; i < 120; ++i) {
      funique_chipIDs.emplace_back(i);
    }
 
    // -- create hitmap histo
    for (int i=0; i<120; i++) {
      ss.str("");
      ss << Form("hitmap_run%d_chipID%d", runnumber, i);
      fhitmaps.push_back(new TH2F(ss.str().c_str(), ss.str().c_str(), 256, 0, 256, 250, 0, 250));
      ss.str("");
      ss << Form("noisemap_run%d_chipID%d", runnumber, i);
      fnoisemaps.push_back(new TH1F(ss.str().c_str(), ss.str().c_str(), 2000, 0, 2000));
      // cout << "created " << ss.str().c_str() << endl;
    }
    
    fhErrors = new TH1F("hErrors", "hErrors", 120, 0., 120.);
    fhTotal  = new TH1F("hTotal", "hTotal", 120, 0., 120.);
    fhRatio  = new TH1F("hRatio", Form("Error ratio (row > 249) run %d", fRun), 120, 0., 120.);
  } else {
    fpHistFile->cd();
    fpHistFile->Write();
 
    for (int i=0; i<120; i++) {
      delete fhitmaps[i];
      delete fnoisemaps[i];
    }
    fhitmaps.clear();
    fnoisemaps.clear();
    
    for (int i=0; i<120; i++) {
      ss.str("");
      ss << Form("hitmap_run%d_chipID%d", runnumber, i);
      fhitmaps.push_back(new TH2F(ss.str().c_str(), ss.str().c_str(), 256, 0, 256, 250, 0, 250));
      ss.str("");
      ss << Form("noisemap_run%d_chipID%d", runnumber, i);
      fnoisemaps.push_back(new TH1F(ss.str().c_str(), ss.str().c_str(), 2000, 0, 2000));
      // cout << "created " << ss.str().c_str() << endl;
    }
  }
  
  fhRatio->Sumw2(kTRUE);
  fhErrors->Reset();
  fhTotal->Reset();
  fhRatio->Reset();
  
}


// ----------------------------------------------------------------------
void skimMu3e::initVariables() {
  cout << "skimMu3e::initVariables() wrong class " << endl;
}




// ----------------------------------------------------------------------
void skimMu3e::openHistFile(string filename) {
  fpHistFile = new TFile(filename.c_str(), "RECREATE");
  fpHistFile->cd();
  cout << "==> skimMu3e: Opened " << fpHistFile->GetName() << endl;
}

// ----------------------------------------------------------------------
void skimMu3e::closeHistFile() {
  if (fpHistFile) {
    cout << "==> skimMu3e: Writing " << fpHistFile->GetName() << endl;
    fpHistFile->cd();
    fpHistFile->Write();
    fpHistFile->Close();
    delete fpHistFile;
  } else {
    cout << "no output histogram file defined" << endl;
  }
}

// --------------------------------------------------------------------------------------------------
void skimMu3e::readCuts(string filename, int dump) {
  char  buffer[200];
  fCutFile = filename;
  if (dump) cout << "==> skimMu3e: Reading " << fCutFile << " for cut settings" << endl;
  sprintf(buffer, "%s", fCutFile.c_str());
  ifstream is(buffer);
  char CutName[100];
  float CutValue;
  int ok(0);

  string fn(fCutFile);

  if (dump) {
    cout << "====================================" << endl;
    cout << "==> skimMu3e: Cut file  " << fCutFile << endl;
    cout << "------------------------------------" << endl;
  }

  TH1D *hcuts = new TH1D("hcuts", "", 1000, 0., 1000.);
  hcuts->GetXaxis()->SetBinLabel(1, fn.c_str());

  while (is.getline(buffer, 200, '\n')) {
    ok = 0;
    if (buffer[0] == '#') {continue;}
    if (buffer[0] == '/') {continue;}
    sscanf(buffer, "%s %f", CutName, &CutValue);

    if (!ok) cout << "==> skimMu3e: ERROR: Don't know about variable " << CutName << endl;
  }

  if (dump)  cout << "------------------------------------" << endl;
}


// ----------------------------------------------------------------------
void skimMu3e::setupTree() {
  cout << "skimMu3e::setupTree()" << endl;
  fv_hit_pixelid = 0, fv_hit_timestamp = 0;

  fpChain->SetBranchAddress("hit_pixelid", &fv_hit_pixelid, &fb_hit_pixelid);
  fpChain->SetBranchAddress("hit_timestamp", &fv_hit_timestamp, &fb_hit_timestamp);
}


// ----------------------------------------------------------------------
int skimMu3e::loop(int nevents, int start, bool readMaskFiles) {
  int maxEvents(0);

  cout << "==> skimMu3e: Chain " << fChainName
       << " has a total of " << fNentries << " events, "
       << (readMaskFiles? " reading mask files": " NOT reading mask files")
       << endl;
  
  // -- Setup for restricted running (not yet foolproof, i.e. bugfree)
  if (nevents < 0) {
    maxEvents = fNentries;
  } else {
    cout << "==> skimMu3e: Running over " << nevents << " events" << endl;
    maxEvents = nevents;
  }
  if (start < 0) {
    start = 0;
  } else {
    cout << "==> skimMu3e: Starting at event " << start << endl;
    if (maxEvents >  fNentries) {
      cout << "==> skimMu3e: Requested to run until event " << maxEvents << ", but will run only to end of chain at ";
      maxEvents = fNentries;
      cout << maxEvents << endl;
    } else {
      cout << "==> skimMu3e: Requested to run until event " << maxEvents << endl;
    }
  }

  // -- The main loop
  int step(50000);
  if (maxEvents < 1000000) step = 10000;
  if (maxEvents < 100000)  step = 1000;
  if (maxEvents < 10000)   step = 500;
  if (maxEvents < 1000)    step = 100;

  uint64_t fnentries = fpChain->GetEntries();
  cout << "----------------------------------------------------------------------" << endl;
  int VERBOSE(0), oldRun(0);
  for (Long64_t ievt = 0; ievt < fnentries; ++ievt) {
    VERBOSE = 0; 
    if (0 == ievt%step) VERBOSE = 1;
    Long64_t tentry = fpChain->LoadTree(ievt);
    fb_hit_pixelid->GetEntry(tentry);  
    fb_hit_timestamp->GetEntry(tentry);  

    if (VERBOSE) cout << "processing event .. " << ievt << " with nhit = " << fv_hit_pixelid->size()
                      << " tentry = " << tentry
                      << endl;
    
    eventProcessing();

  }

  // -- dump final results for the final run
  runEndAnalysis(1);
  
  return 0;

}


// ----------------------------------------------------------------------=
void skimMu3e::runEndAnalysis(int runnumber) {
  cout << "==> runEndAnalysis for run = " << runnumber << endl;
  fhRatio->Divide(fhErrors, fhTotal);
  
  printNonZero(fhErrors);

  // -- find noisy pixels per chipID
  std::map<int, std::vector<std::pair<uint8_t, uint8_t>>> noisy_pixels;
  vector<string> vPrint;
  
  bool DBX(false);
  for (int chipID : funique_chipIDs) {
    // -- skip scintillator and bad chipIDs
    if (chipID >= 120) continue;
    
    noisy_pixels[chipID] = std::vector<std::pair<uint8_t, uint8_t> >();
    
    // -- try to find noise level
    TH2F *h2 = fhitmaps.at(chipID);
    TH1F *h1 = fnoisemaps.at(chipID);
    for (int32_t ny = 1; ny <= h2->GetYaxis()->GetNbins(); ny++) {
      for (int32_t nx = 1; nx <= h2->GetXaxis()->GetNbins(); nx++) {
        fnoisemaps.at(chipID)->Fill(h2->GetBinContent(nx, ny));
      }
    }
    // -- determine noise_limit, based on modeNoiseLevel and noiseLevel
    double noise_limit(0.);
    if (1 == fModeNoiseLimit) {
      noise_limit = fNoiseLevel;
    } else if (2 == fModeNoiseLimit) {
      noise_limit = h1->GetMean() + fNoiseLevel*h1->GetRMS() + 0.5;
    }
    cout << "chipID " << chipID
         << " (maximum: " << h2->GetMaximum() << ") mean(nhit) = " << h1->GetMean()
         << " RMS = " << h1->GetRMS()
         << " noise level = " << noise_limit
         << endl;
        
    int tot_noisy_pixels = 0;
    vector<uint8_t> vNoise; 
    string spix(Form("(max = %d), pix: ", static_cast<int>(h2->GetMaximum())));
    for (int32_t nx = 1; nx <= fhitmaps.at(chipID)->GetXaxis()->GetNbins(); nx++) {
      if (DBX) cout << "filling col " << nx-1 << endl;
      for (int32_t ny = 1; ny <= fhitmaps.at(chipID)->GetYaxis()->GetNbins(); ny++) {
        int nhit = static_cast<int>(fhitmaps.at(chipID)->GetBinContent(nx, ny)); 
        // -- if the noise limit is 0 then there will be NO noisy pixels
        if ((noise_limit > 0) && (nhit > noise_limit)) {
          if (DBX) std::cout << "Chip ID " << chipID << ", Found noisy pixel at " << nx-1 << ", " << ny-1
                             << " nhits = " << nhit << " noise_limit = " << noise_limit
                             << std::endl;
          tot_noisy_pixels++;
          noisy_pixels[chipID].push_back(std::pair<uint8_t, uint8_t>(nx-1, ny-1));
          vNoise.push_back(0x0);
          spix += string(Form("%d/%d(%d) ", nx-1, ny-1, nhit));
        } else {
          vNoise.push_back(0xff);
        }
      }
      // -- fill up to 255
      // for (int32_t ny = 251; ny <= 256; ny++) {
      //   if (DBX) cout << "+filling row " << ny-1 << endl;
      //   vNoise.push_back(0xda);
      // }
 
      // -- store 0xdada as end-of-col marker
      vNoise.push_back(0xda);
      vNoise.push_back(0xda);
      // -- store number of col
      vNoise.push_back(0xda);
      vNoise.push_back(nx-1);
      // -- store LVDS error flag
      vNoise.push_back(0xda);
      if (fhErrors->GetBinContent(chipID+1) > 0) {
        vNoise.push_back(0x01);
      } else {
        vNoise.push_back(0x00);
      }
    }
    std::cout << " with a  total of " << tot_noisy_pixels << " (" << tot_noisy_pixels*100/64000 << "%)\n";
    writeNoiseMaskFile(vNoise, runnumber, chipID, fSuffix, fOutputDirectoryName);
 
    vPrint.push_back(Form("chipID %3d, n. level = %5.3f, N(n. pixels) = %d %s",
                          chipID, noise_limit, tot_noisy_pixels, spix.c_str()));
    vNoise.clear();
  }
 
  ofstream o(Form("%s/summaryNoiseMaskFile%s-run%d.txt", fOutputDirectoryName.c_str(), fSuffix.c_str(), runnumber)); 
  cout << "Summary of noisy (n.) pixels for run "  << runnumber << endl;
  o << "Summary of noisy (n.) pixels for run "  << runnumber << endl;
  for (unsigned int i = 0; i < vPrint.size(); ++i) {
    cout << vPrint[i] << endl;
    o << vPrint[i] << endl;
  }
  o.close();
    
  return;
}

// ----------------------------------------------------------------------
// -- dump vector<uint8_t> into mask file
// ----------------------------------------------------------------------
void skimMu3e::writeNoiseMaskFile(vector<uint8_t> noise, int runnumber, int chipID,
                                      string name, string dir) {
  string filename("");
  if (runnumber > 0) {
    filename = Form("%s/noiseMaskFile%s-run%d-chipID%d", dir.c_str(), name.c_str(), runnumber, chipID);
  } else {
    filename = Form("%s/noiseMaskFile%s-chipID%d", dir.c_str(), name.c_str(), chipID);
  }
 
  ofstream o(filename); 
  for (unsigned int i = 0; i < noise.size(); ++i) {
    o << noise[i];
  }
  o.close();
}
