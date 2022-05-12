// -- original by MK
// -- changed to reflect new data structure in 2022

using namespace::std;

// -- dump vnoise into noise mask file
void writeNoiseMaskFile(vector<uint8_t> vnoise, int runnumber, int chipID);

// -- read in noise mask file and merge its contents into vnoise
void readNoiseMaskFile(vector<uint8_t> &vnoise, int runnumber, int chipID);


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
void writeNoisyPixelsMaskFiles(int runnumber) {
  cout << "start remove_noisy_pixels for " << Form("dataTree%05d.root", runnumber) << endl;

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

  
  // open file
  auto * infile = new TFile(Form("dataTree%05d.root", runnumber), "OPEN");
  std::stringstream ss;

  // get tree from file
  auto * tree = (TTree *) infile->FindObjectAny("HitData");

  // get unique chipID
  TH1F * cID = new TH1F("chipID", "chipID", 200, 0, 200);
  tree->Project("chipID", "chipID");
  std::vector<int> unique_chipIDs;
  for (int i = 0; i<=200; i++) {
    if (cID->GetBinContent(i) > 0) {
      unique_chipIDs.emplace_back(i-1);
    }
  }

  // create hitmap histo
  std::vector<TH2F *> hitmaps;
  std::vector<TH1F *> noisemaps;
  for (int i=0; i<128; i++) {
    ss.str("");
    ss << "hitmap" << i;
    hitmaps.push_back(new TH2F(ss.str().c_str(), ss.str().c_str(), 256, 0, 256, 250, 0, 250));
    ss.str("");
    ss << "noisemap" << i;
    noisemaps.push_back(new TH1F(ss.str().c_str(), ss.str().c_str(), 2000, 0, 2000));
    cout << "created " << ss.str().c_str() << endl;
  }

  vector<unsigned int>  *v_runID(0), *v_MIDASEventID(0), *v_ts2(0), *v_hitTime(0),
    *v_headerTime(0), *v_headerTimeMajor(0), *v_subHeaderTime(0), *v_trigger(0),
    *v_isInCluster(0);
  vector<unsigned char>  *v_col(0), *v_row(0), *v_chipID(0), *v_fpgaID(0), *v_chipIDRaw(0), *v_tot(0), *v_isMUSR(0), *v_hitType(0), *v_layer(0);
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
    if (0 == ievt%100) VERBOSE = 1;
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
                      <<  " MIDASEventID = "  << (v_MIDASEventID->size() > 0? Form("%d", v_MIDASEventID->at(0)): "n/a")
                      << " sizes = " << v_MIDASEventID->size() << "/" << v_col->size()
                      << endl;
    for (int ihit = 0; ihit < v_col->size(); ++ihit) {
      if (VERBOSE) cout << Form("col/row/chip = %d/%d/%d", v_col->at(ihit), v_row->at(ihit), v_chipID->at(ihit)) << endl;
      hitmaps.at(v_chipID->at(ihit))->Fill(v_col->at(ihit), v_row->at(ihit));
    }    
  }


  // find noisy pixels per chipID
  std::map<int, std::vector<std::pair<uint8_t, uint8_t>>> noisy_pixels;
  std::uint64_t hits_total = 0;
  int noise_limit(0);

  vector<string> vPrint;
  
  bool DBX(false);
  for (int chipID : unique_chipIDs) {
    if (skipList.end() != find(skipList.begin(), skipList.end(), chipID)) {
      continue;
    }

    if (DBX && chipID != 18) continue;
    
    hits_total = 0;
    if (chipID >= 128)
      continue;
    for (int32_t nx = 5; nx <= 245; nx++) {
      for (int32_t ny = 5; ny <= 245; ny++) {
        hits_total = hits_total + hitmaps.at(chipID)->GetBinContent(nx, ny);
      }
    }
    hits_total = hits_total * 1.111111; // correct for missing pixels at the chip border
    noise_limit = 10. * hits_total/64000;
    noisy_pixels[chipID] = std::vector<std::pair<uint8_t, uint8_t> >();
    

    // -- try to find noise level
    TH2F *h2 = hitmaps.at(chipID);
    TH1F *h1 = noisemaps.at(chipID);
    for (int32_t ny = 1; ny <= h2->GetYaxis()->GetNbins(); ny++) {
      for (int32_t nx = 1; nx <= h2->GetXaxis()->GetNbins(); nx++) {
        noisemaps.at(chipID)->Fill(h2->GetBinContent(nx, ny));
      }
    }
    // -- noise defined as mean(nhits) + NSIGMA*sigma
    int NSIGMA(20);
    noise_limit = h1->GetMean() + NSIGMA*h1->GetRMS() + 0.5;
    cout << "chipID " << chipID
         << " (maximum: " << h2->GetMaximum() << ") mean(nhit) = " << h1->GetMean() << " RMS = " << h1->GetRMS()
         << " noise level = " << noise_limit
         << endl;
    
    
    int tot_noisy_pixels = 0;
    vector<uint8_t> vNoise; 
    string spix(Form("(max = %d), pix: ", static_cast<int>(h2->GetMaximum())));
    for (int32_t nx = 1; nx <= hitmaps.at(chipID)->GetXaxis()->GetNbins(); nx++) {
      if (DBX) cout << "filling col " << nx-1 << endl;
      for (int32_t ny = 1; ny <= hitmaps.at(chipID)->GetYaxis()->GetNbins(); ny++) {
        int nhit = static_cast<int>(hitmaps.at(chipID)->GetBinContent(nx, ny)); 
        // -- if the noise limit is 0 then there will be NO noisy pixels
        if ((noise_limit > 0) && (nhit > noise_limit)) {
          std::cout << "Chip ID " << chipID << ", Found noisy pixel at " << nx-1 << ", " << ny-1
                    << " nhits = " << nhit << " noise_limit = " << noise_limit
                    << std::endl;
          tot_noisy_pixels++;
          noisy_pixels[chipID].push_back(std::pair<uint8_t, uint8_t>(nx-1, ny-1));
          vNoise.push_back(0xff);
          spix += string(Form("%d/%d(%d) ", nx-1, ny-1, nhit));
        } else {
          vNoise.push_back(0x0);
        }
      }
      // -- fill up to 255
      for (int32_t ny = 251; ny <= 256; ny++) {
        if (DBX) cout << "+filling row " << ny-1 << endl;
        vNoise.push_back(0xda);
      }
    }
    std::cout << " with a  total of " << tot_noisy_pixels << " (" << tot_noisy_pixels*100/64000 << "%)\n";
    writeNoiseMaskFile(vNoise, runnumber, chipID);

    vPrint.push_back(Form("chipID %3d, n. level = %2d, N(n. pixels) = %d %s", chipID, noise_limit, tot_noisy_pixels, spix.c_str()));
    vNoise.clear();
  }

  ofstream o(Form("summaryNoiseMaskFile-run%d.txt", runnumber)); 
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
void writeNoiseMaskFile(vector<uint8_t> noise, int runnumber, int chipID) {
  ofstream o(Form("noiseMaskFile-run%d-chipID%d", runnumber, chipID)); 
  for (unsigned int i = 0; i < noise.size(); ++i) {
    o << noise[i];
  }
  o.close();
}

// ----------------------------------------------------------------------
void readNoiseMaskFile(vector<uint8_t> &vnoise, int runnumber, int chipID) {
  ifstream is(Form("noiseMaskFile-run%d-chipID%d", runnumber, chipID)); 

  if (is.is_open() ) {
    char mychar;
    int i(0); 
    while (is) {
      mychar = is.get();
      if (0 != mychar) {
        vnoise[i] = mychar;
      }
      ++i;
    }
  }

  is.close();
}
