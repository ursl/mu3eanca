
using namespace::std;

map<int, vector<pair<int, int> > > gChipNoisyPixels; 
map<int, int> gBadChips; 


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
bool unclean(const vector<uint8_t> &v, int maxNoise) {
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
vector<uint8_t> readMaskFile(string filename) {
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
void readNoiseMaskFiles(int runnumber, string dir = "nmf") {
  int maxNoise(1000);
  string name("noiseMaskFilenmf"); 
  gChipNoisyPixels.clear();
  for (int i = 0; i < 120; ++i) {
    vector<uint8_t> vnoise = readMaskFile(Form("%s/%s-run%d-chipID%d", dir.c_str(), name.c_str(), runnumber, i));
    if (validNoise(vnoise)) {
      fillNoisyPixels(i, vnoise, gChipNoisyPixels);
    }
    gBadChips[i] = 0;  // every chip must be in here
    if (skipChip(i))               gBadChips[i] |= 1; 
    if (badLVDS(vnoise))           gBadChips[i] |= 2; 
    if (unclean(vnoise, maxNoise)) gBadChips[i] |= 4; 
  }
}

// ----------------------------------------------------------------------
void anaHitData(string rootdir, int runnumber, string dir = ".") {

  cout << "anaHitData reading noise mask file for run " << runnumber << " from directory nmf" << endl;
  readNoiseMaskFiles(runnumber, "nmf");
  
  cout << "anaHitData open " << Form("%s/dataTree%05d.root", rootdir.c_str(), runnumber) << endl;
  auto * infile = new TFile(Form("%s/dataTree%05d.root", rootdir.c_str(), runnumber), "OPEN");
  std::stringstream ss;

  // get tree from file
  TTree* tree = (TTree *) infile->FindObjectAny("HitData");
  
  // create hitmap histo
  TH2D *hitmap = new TH2D(Form("hitmap_run%d", runnumber), Form("hits, run %d", runnumber), 256, 0, 256, 250, 0, 250);
  TH1D *hittot = new TH1D(Form("hittot_run%d", runnumber), Form("hits, run %d", runnumber), 32, 0, 256.);
  hittot->SetMinimum(0);
  TH1D *badchiptot = new TH1D(Form("badchiptot_run%d", runnumber), Form("bad chips, run %d", runnumber), 32, 0, 256.);
  badchiptot->SetMinimum(0);
  TH1D *noisytot = new TH1D(Form("noisytot_run%d", runnumber), Form("noisy pixels, run %d", runnumber), 32, 0, 256.);
  noisytot->SetMinimum(0);
  std::vector<TH2D *> hitmaps;
  std::vector<TH1D *> hittots;
  for (int i=0; i<120; i++) {
    ss.str("");
    ss << Form("hitmap_run%d_chipID%d", runnumber, i);
    hitmaps.push_back(new TH2D(ss.str().c_str(), ss.str().c_str(), 256, 0, 256, 250, 0, 250));
    // cout << "created " << ss.str().c_str() << endl;
    ss.str("");
    ss << Form("hittot_run%d_chipID%d", runnumber, i);
    hittots.push_back(new TH1D(ss.str().c_str(), ss.str().c_str(), 32, 0, 256.));
    // cout << "created " << ss.str().c_str() << endl;

  }

  vector<unsigned int>  *v_runID(0), *v_MIDASEventID(0), *v_ts2(0), *v_hitTime(0),
    *v_headerTime(0), *v_headerTimeMajor(0), *v_subHeaderTime(0), *v_trigger(0),
    *v_isInCluster(0);
  vector<unsigned char>  *v_col(0), *v_row(0), *v_chipID(0), *v_fpgaID(0), *v_chipIDRaw(0),
    *v_tot(0), *v_isMUSR(0), *v_hitType(0), *v_layer(0);
  TBranch *b_runID(0), *b_col(0), *b_row(0), *b_chipID(0), *b_MIDASEventID(0),
    *b_ts2(0), *b_hitTime(0),
    *b_headerTime(0), *b_headerTimeMajor(0), *b_subHeaderTime(0), *b_trigger(0),
    *b_isInCluster(0),
    *b_fpgaID(0), *b_chipIDRaw(0), *b_tot(0), *b_isMUSR(0), *b_hitType(0), *b_layer(0);

  tree->SetBranchAddress("runID", &v_runID, &b_runID);
  tree->SetBranchAddress("MIDASEventID", &v_MIDASEventID, &b_MIDASEventID);
  tree->SetBranchAddress("col", &v_col, &b_col);
  tree->SetBranchAddress("row", &v_row, &b_row);
  tree->SetBranchAddress("fpgaID", &v_fpgaID, &b_fpgaID);
  tree->SetBranchAddress("chipID", &v_chipID, &b_chipID);
  tree->SetBranchAddress("chipIDRaw", &v_chipIDRaw, &b_chipIDRaw);
  tree->SetBranchAddress("tot", &v_tot, &b_tot);
  tree->SetBranchAddress("ts2", &v_ts2, &b_ts2);
  tree->SetBranchAddress("hitTime", &v_hitTime, &b_hitTime);
  tree->SetBranchAddress("headerTime", &v_headerTime, &b_headerTime);
  tree->SetBranchAddress("headerTimeMajor", &v_headerTimeMajor, &b_headerTimeMajor);
  tree->SetBranchAddress("subHeaderTime", &v_subHeaderTime, &b_subHeaderTime);
  tree->SetBranchAddress("isMUSR", &v_isMUSR, &b_isMUSR);
  tree->SetBranchAddress("hitType", &v_hitType, &b_hitType);
  tree->SetBranchAddress("trigger", &v_trigger, &b_trigger);
  tree->SetBranchAddress("layer", &v_layer, &b_layer);
  tree->SetBranchAddress("isInCluster", &v_isInCluster, &b_isInCluster);

  uint64_t fnentries = tree->GetEntries();
  cout << "----------------------------------------------------------------------" << endl;
  int VERBOSE(0);
  for (Long64_t ievt = 0; ievt < fnentries; ++ievt) {
    VERBOSE = 0; 
    //    if (0 == ievt%100) VERBOSE = 1;
    Long64_t tentry = tree->LoadTree(ievt);
    b_runID->GetEntry(tentry);  
    b_MIDASEventID->GetEntry(tentry);  
    b_col->GetEntry(tentry);  
    b_row->GetEntry(tentry);  
    b_fpgaID->GetEntry(tentry);  
    b_chipID->GetEntry(tentry);  
    b_chipIDRaw->GetEntry(tentry);  
    b_tot->GetEntry(tentry);  
    b_ts2->GetEntry(tentry);  
    b_hitTime->GetEntry(tentry);  
    b_headerTime->GetEntry(tentry);  
    b_headerTimeMajor->GetEntry(tentry);  
    b_subHeaderTime->GetEntry(tentry);  
    b_isMUSR->GetEntry(tentry);  
    b_hitType->GetEntry(tentry);  
    b_trigger->GetEntry(tentry);  
    b_layer->GetEntry(tentry);  
    b_isInCluster->GetEntry(tentry);  
    if (VERBOSE) cout << "processing event .. " << ievt << " with nhit = " << v_col->size()
                      << " tentry = " << tentry
                      <<  " MIDASEventID = "
                      << (v_MIDASEventID->size() > 0? Form("%d", v_MIDASEventID->at(0)): "n/a")
                      << " sizes = " << v_MIDASEventID->size() << "/" << v_col->size()
                      << endl;
    for (int ihit = 0; ihit < v_col->size(); ++ihit) {
      int chipID = v_chipID->at(ihit); 
      // -- weed out scintillator
      if (120 == chipID) {
        cout << "scintillator hit" << endl;
        continue;
      }  
      int row    = v_row->at(ihit);
      int col    = v_col->at(ihit); 
      int tot    = v_tot->at(ihit); 
      if (gBadChips[chipID] > 0) {
        badchiptot->Fill(tot);
        continue;
      }

      // -- this must stay because chipID > 119 are NOT in gBadChips
      if (skipChip(chipID)) continue;
      
      vector<pair<int, int> > vnoise = gChipNoisyPixels[chipID];
      if (vnoise.end() != find(vnoise.begin(), vnoise.end(), make_pair(col, row))) {
        //        cout << "noisy pixel on chip = " << chipID << " at col/row = " << col << "/" << row << endl;
        noisytot->Fill(tot);
        continue;
      }
      //      cout << "filling chipID = " << chipID << " col/row = " << col << "/" << row << endl;
      hitmaps.at(chipID)->Fill(col, row);
      hittots.at(chipID)->Fill(tot);
        
      hitmap->Fill(col, row);
      hittot->Fill(tot);
    }    
  }

  c0.Clear();
  hittot->GetXaxis()->SetTitle("tot");
  hittot->GetYaxis()->SetTitle("entries/bin");
  hpl(hittot, "bluefill");
  string pdfname(Form("hittot-run%d.pdf", runnumber));
  c0.SaveAs(Form("%s/%s", dir.c_str(), pdfname.c_str()));

  c0.Clear();
  noisytot->GetXaxis()->SetTitle("tot");
  noisytot->GetYaxis()->SetTitle("entries/bin");
  hpl(noisytot, "bluefill");
  pdfname = Form("noisytot-run%d.pdf", runnumber);
  c0.SaveAs(Form("%s/%s", dir.c_str(), pdfname.c_str()));

  c0.Clear();
  badchiptot->GetXaxis()->SetTitle("tot");
  badchiptot->GetYaxis()->SetTitle("entries/bin");
  hpl(badchiptot, "bluefill");
  pdfname = Form("badchiptot-run%d.pdf", runnumber);
  c0.SaveAs(Form("%s/%s", dir.c_str(), pdfname.c_str()));


  c0.Clear();
  gStyle->SetOptStat(0);
  hitmap->GetXaxis()->SetTitle("col");
  hitmap->GetYaxis()->SetTitle("row");
  hpl(hitmap, "colz");
  pdfname = Form("hitmap-run%d.pdf", runnumber);
  c0.SaveAs(Form("%s/%s", dir.c_str(), pdfname.c_str()));

}
