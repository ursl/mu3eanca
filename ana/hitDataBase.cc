#include "hitDataBase.hh"
#include "hitDataIncludes.hh"

// ----------------------------------------------------------------------
// Run with: see derived classes!
//
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
hitDataBase::hitDataBase(TChain *chain, string treeName): fVerbose(0) {
  cout << "==> hitDataBase: constructor..." << endl;
  if (chain == 0) {
    cout << "You need to pass a chain!" << endl;
  }
  fpChain = chain;
  fChainName = treeName;

  fNentries = fpChain->GetEntries();
  cout << "==> hitDataBase: constructor fpChain: " << fpChain << "/" << fpChain->GetName()
       << " entries = " <<   fNentries
       << endl;

  setupTree();
  
}


// ----------------------------------------------------------------------
hitDataBase::~hitDataBase() {
  cout << "==> hitDataBase: destructor ..." << endl;
  if (!fpChain) return;
  delete fpChain->GetCurrentFile();
}


// ----------------------------------------------------------------------
pair<int, int> hitDataBase::colrowFromIdx(int idx) {
  int col = idx/256;
  int row = idx%256;
  return make_pair(col, row);
}


// ----------------------------------------------------------------------
int hitDataBase::idxFromColRow(int col, int row) {
  int idx = col*256 + row;
  return idx; 
}


// ----------------------------------------------------------------------
bool hitDataBase::validNoise(const vector<uint8_t> &v) {
  for (unsigned int i = 0; i < v.size(); ++i) {
    if (0 != v[i]) {
      return true;
    }
  }
  return false;
}


// ----------------------------------------------------------------------
bool hitDataBase::badLVDS(const vector<uint8_t> &v) {
  bool badLVDS(true);
  if (validNoise(v)) {
    if (0 == v[0xff]) badLVDS = false;
  }
  return badLVDS;
}


// ----------------------------------------------------------------------
bool hitDataBase::unclean(const vector<uint8_t> &v, int maxNoise) {
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
void hitDataBase::fillNoisyPixels(int chipID, vector<uint8_t> &vnoise,
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
vector<uint8_t> hitDataBase::readMaskFile(string filename) {
  // -- open the file
  streampos fileSize;
  ifstream file;
  file.open(filename.c_str(), std::ios::binary);
  if (!file) {
    cout << "file ->" << filename << "<- not found, skipping" << endl;
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
void hitDataBase::readNoiseMaskFiles(int runnumber, string dir) {
  int maxNoise(1000);
  string name("noiseMaskFilenmf"); 
  fChipNoisyPixels.clear();
  for (int i = 0; i < 120; ++i) {
    vector<uint8_t> vnoise;
    if (runnumber > 0) {
      vnoise = readMaskFile(Form("%s/%s-run%d-chipID%d", dir.c_str(), name.c_str(), runnumber, i));
    } else {
      vnoise = readMaskFile(Form("%s/%s-chipID%d", dir.c_str(), name.c_str(), i));
    }
    if (validNoise(vnoise)) {
      fillNoisyPixels(i, vnoise, fChipNoisyPixels);
    }
    fChipQuality[i] = 0;  // every chip must be in here
    if (skipChip(i))               fChipQuality[i] |= 2; 
    if (badLVDS(vnoise))           fChipQuality[i] |= 4; 
    if (unclean(vnoise, maxNoise)) fChipQuality[i] |= 8; 
  }

  // -- scintillator 
  fChipQuality[120] = 1;  

  // -- add fake chipID
  for (int i = 121; i < 130; ++i) {
    fChipQuality[i] = 4;  
  }
}

// ----------------------------------------------------------------------
bool hitDataBase::skipChip(int chipID) {
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
void hitDataBase::startAnalysis() {
  cout << "hitDataBase::startAnalysis() wrong class " << endl;
}

// ----------------------------------------------------------------------
void hitDataBase::endAnalysis() {
  cout << "hitDataBase::endAnalysis() wrong class " << endl;
}


// ----------------------------------------------------------------------
void hitDataBase::eventProcessing() {
  cout << "hitDataBase::eventProcessing() wrong class " << endl;
}



// ----------------------------------------------------------------------
void hitDataBase::fillHist() {
  cout << "hitDataBase::fillHist() wrong class " << endl;
}

// ----------------------------------------------------------------------
void hitDataBase::bookHist(int i) {
  static bool first(true);
  cout << "==> hitDataBase: bookHist> run " << i << endl;

  fpHistFile->cd();

  // -- Reduced Tree
  if (first) {
    fTree = new TTree("events", "events");
    fTree->Branch("run",      &fRun,       "run/I");
    fTree->Branch("evt",      &fEvt,       "evt/I");
    first = false;
  }

}


// ----------------------------------------------------------------------
void hitDataBase::initVariables() {
  cout << "hitDataBase::initVariables() wrong class " << endl;
}




// ----------------------------------------------------------------------
void hitDataBase::openHistFile(string filename) {
  fpHistFile = new TFile(filename.c_str(), "RECREATE");
  fpHistFile->cd();
  cout << "==> hitDataBase: Opened " << fpHistFile->GetName() << endl;
}

// ----------------------------------------------------------------------
void hitDataBase::closeHistFile() {
  if (fpHistFile) {
    cout << "==> hitDataBase: Writing " << fpHistFile->GetName() << endl;
    fpHistFile->cd();
    fpHistFile->Write();
    fpHistFile->Close();
    delete fpHistFile;
  } else {
    cout << "no output histogram file defined" << endl;
  }
}

// --------------------------------------------------------------------------------------------------
void hitDataBase::readCuts(string filename, int dump) {
  char  buffer[200];
  fCutFile = filename;
  if (dump) cout << "==> hitDataBase: Reading " << fCutFile << " for cut settings" << endl;
  sprintf(buffer, "%s", fCutFile.c_str());
  ifstream is(buffer);
  char CutName[100];
  float CutValue;
  int ok(0);

  string fn(fCutFile);

  if (dump) {
    cout << "====================================" << endl;
    cout << "==> hitDataBase: Cut file  " << fCutFile << endl;
    cout << "------------------------------------" << endl;
  }

  TH1D *hcuts = new TH1D("hcuts", "", 1000, 0., 1000.);
  hcuts->GetXaxis()->SetBinLabel(1, fn.c_str());

  while (is.getline(buffer, 200, '\n')) {
    ok = 0;
    if (buffer[0] == '#') {continue;}
    if (buffer[0] == '/') {continue;}
    sscanf(buffer, "%s %f", CutName, &CutValue);

    if (!ok) cout << "==> hitDataBase: ERROR: Don't know about variable " << CutName << endl;
  }

  if (dump)  cout << "------------------------------------" << endl;
}


// ----------------------------------------------------------------------
void hitDataBase::setupTree() {
  fv_runID = 0, fv_MIDASEventID = 0, fv_ts2 = 0, fv_hitTime = 0,
    fv_headerTime = 0, fv_headerTimeMajor = 0, fv_subHeaderTime = 0, fv_trigger = 0,
    fv_isInCluster = 0;
  fv_col = 0, fv_row = 0, fv_chipID = 0, fv_fpgaID = 0, fv_chipIDRaw = 0,
    fv_tot = 0, fv_isMUSR = 0, fv_hitType = 0, fv_layer = 0;
  fb_runID = 0, fb_col = 0, fb_row = 0, fb_chipID = 0, fb_MIDASEventID = 0,
    fb_ts2 = 0, fb_hitTime = 0,
    fb_headerTime = 0, fb_headerTimeMajor = 0, fb_subHeaderTime = 0, fb_trigger = 0,
    fb_isInCluster = 0,
    fb_fpgaID = 0, fb_chipIDRaw = 0, fb_tot = 0, fb_isMUSR = 0, fb_hitType = 0, fb_layer = 0;


  fpChain->SetBranchAddress("runID", &fv_runID, &fb_runID);
  fpChain->SetBranchAddress("MIDASEventID", &fv_MIDASEventID, &fb_MIDASEventID);
  fpChain->SetBranchAddress("col", &fv_col, &fb_col);
  fpChain->SetBranchAddress("row", &fv_row, &fb_row);
  fpChain->SetBranchAddress("fpgaID", &fv_fpgaID, &fb_fpgaID);
  fpChain->SetBranchAddress("chipID", &fv_chipID, &fb_chipID);
  fpChain->SetBranchAddress("chipIDRaw", &fv_chipIDRaw, &fb_chipIDRaw);
  fpChain->SetBranchAddress("tot", &fv_tot, &fb_tot);
  fpChain->SetBranchAddress("ts2", &fv_ts2, &fb_ts2);
  fpChain->SetBranchAddress("hitTime", &fv_hitTime, &fb_hitTime);
  fpChain->SetBranchAddress("headerTime", &fv_headerTime, &fb_headerTime);
  fpChain->SetBranchAddress("headerTimeMajor", &fv_headerTimeMajor, &fb_headerTimeMajor);
  fpChain->SetBranchAddress("subHeaderTime", &fv_subHeaderTime, &fb_subHeaderTime);
  fpChain->SetBranchAddress("isMUSR", &fv_isMUSR, &fb_isMUSR);
  fpChain->SetBranchAddress("hitType", &fv_hitType, &fb_hitType);
  fpChain->SetBranchAddress("trigger", &fv_trigger, &fb_trigger);
  fpChain->SetBranchAddress("layer", &fv_layer, &fb_layer);
  fpChain->SetBranchAddress("isInCluster", &fv_isInCluster, &fb_isInCluster);
}


// ----------------------------------------------------------------------
int hitDataBase::loop(int nevents, int start) {
  int maxEvents(0);

  cout << "==> hitDataBase: Chain " << fChainName << " has a total of " << fNentries << " events" << endl;

  // -- Setup for restricted running (not yet foolproof, i.e. bugfree)
  if (nevents < 0) {
    maxEvents = fNentries;
  } else {
    cout << "==> hitDataBase: Running over " << nevents << " events" << endl;
    maxEvents = nevents;
  }
  if (start < 0) {
    start = 0;
  } else {
    cout << "==> hitDataBase: Starting at event " << start << endl;
    if (maxEvents >  fNentries) {
      cout << "==> hitDataBase: Requested to run until event " << maxEvents << ", but will run only to end of chain at ";
      maxEvents = fNentries;
      cout << maxEvents << endl;
    } else {
      cout << "==> hitDataBase: Requested to run until event " << maxEvents << endl;
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
    fb_runID->GetEntry(tentry);  
    fb_MIDASEventID->GetEntry(tentry);  
    fb_col->GetEntry(tentry);  
    fb_row->GetEntry(tentry);  
    fb_fpgaID->GetEntry(tentry);  
    fb_chipID->GetEntry(tentry);  
    fb_chipIDRaw->GetEntry(tentry);  
    fb_tot->GetEntry(tentry);  
    fb_ts2->GetEntry(tentry);  
    fb_hitTime->GetEntry(tentry);  
    fb_headerTime->GetEntry(tentry);  
    fb_headerTimeMajor->GetEntry(tentry);  
    fb_subHeaderTime->GetEntry(tentry);  
    fb_isMUSR->GetEntry(tentry);  
    fb_hitType->GetEntry(tentry);  
    fb_trigger->GetEntry(tentry);  
    fb_layer->GetEntry(tentry);  
    fb_isInCluster->GetEntry(tentry);  
    if (VERBOSE) cout << "processing event .. " << ievt << " with nhit = " << fv_col->size()
                      << " tentry = " << tentry
                      <<  " run = "
                      << (fv_runID->size() > 0? fv_runID->at(0): 0)
                      <<  " MIDASEventID = "
                      << (fv_MIDASEventID->size() > 0? Form("%d", fv_MIDASEventID->at(0)): "n/a")
                      << " sizes = " << fv_MIDASEventID->size() << "/" << fv_col->size()
                      << endl;
    
    fRun =  (fv_runID->size() > 0? fv_runID->at(0): 0);
    fEvt = (fv_MIDASEventID->size() > 0? fv_MIDASEventID->at(0): -1);
    if (fRun != oldRun) {
      oldRun = fRun;
      if (fRun > 0) {
        bookHist(fRun);
        readNoiseMaskFiles(fRun, "nmf");
      }
    }
    
    eventProcessing();

  }

  return 0;

}

// ----------------------------------------------------------------------
ostream & operator << (ostream& o, const struct hID& h) {
  return o << Form("run/chip/name = %d/%d/%s", h.run, h.chipID, h.name.c_str());
}
