// ----------------------------------------------------------------------
// -- original by MK
// -- changed to reflect new data structure in 2022
// 
// -- Usage examples:
//           produceAllMergedNoiseFiles()
//           produceAllMergedNoiseFiles(1, 1.5)
//           produceAllMergedNoiseFiles(2, 10)
//           ** after editing vector<int> runlist to contain all desired runs ** 
// ----------------------------------------------------------------------
using namespace::std;

// -- dump vnoise into noise mask filev
void writeNoiseMaskFile(vector<uint8_t> vnoise, int runnumber, int chipID, string name, string dir);


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
// -- read noise information for all chips from a tree for one run
// ----------------------------------------------------------------------
void writeNoisyPixelsMaskFiles(string rootdir, int runnumber, int modeNoiseLimit, double noiseLevel, string name, string dir = ".") {
  cout << "writeNoisyPixelsMaskFiles for " << Form("%s/dataTree%05d.root", rootdir.c_str(), runnumber) << endl;

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
  auto * infile = new TFile(Form("%s/dataTree%05d.root", rootdir.c_str(), runnumber), "OPEN");
  std::stringstream ss;

  // get tree from file
  auto * tree = (TTree *) infile->FindObjectAny("HitData");

  TH1F *hErrors = new TH1F("hErrors", "hErrors", 120, 0., 120.);
  TH1F *hTotal  = new TH1F("hTotal", "hTotal", 120, 0., 120.);
  TH1F *hRatio  = new TH1F("hRatio", Form("Error ratio (row > 249) run %d", runnumber), 120, 0., 120.); hRatio->Sumw2(kTRUE);

  hErrors->Reset();
  hTotal->Reset();
  hRatio->Reset();
  
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
  for (int i=0; i<120; i++) {
    ss.str("");
    ss << Form("hitmap_run%d_chipID%d", runnumber, i);
    hitmaps.push_back(new TH2F(ss.str().c_str(), ss.str().c_str(), 256, 0, 256, 250, 0, 250));
    ss.str("");
    ss << Form("noisemap_run%d_chipID%d", runnumber, i);
    noisemaps.push_back(new TH1F(ss.str().c_str(), ss.str().c_str(), 2000, 0, 2000));
    cout << "created " << ss.str().c_str() << endl;
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
                      <<  " MIDASEventID = "
		      << (v_MIDASEventID->size() > 0? Form("%d", v_MIDASEventID->at(0)): "n/a")
                      << " sizes = " << v_MIDASEventID->size() << "/" << v_col->size()
                      << endl;
    for (int ihit = 0; ihit < v_col->size(); ++ihit) {
      if (v_chipID->at(ihit) > 119) {
        cout << Form("LVDS error: col/row/chip = %d/%d/%d", v_col->at(ihit), v_row->at(ihit), v_chipID->at(ihit)) << endl;
        continue;
      }
      if (VERBOSE) cout << Form("col/row/chip = %d/%d/%d", v_col->at(ihit), v_row->at(ihit), v_chipID->at(ihit))
                        << endl;
      hitmaps.at(v_chipID->at(ihit))->Fill(v_col->at(ihit), v_row->at(ihit));

      hTotal->Fill(v_chipID->at(ihit));
      if (v_row->at(ihit) > 249) {
        hErrors->Fill(v_chipID->at(ihit));
      }
      if (v_col->at(ihit) > 255) {
        hErrors->Fill(v_chipID->at(ihit));
      }
    }    
  }

  hRatio->Divide(hErrors, hTotal);
  
  printNonZero(hErrors);

  // LVDS error: col/row/chip = 13/208/126
  //   1    0.000  ..    1.000 :        1.000 +/-        1.000
  //  42   41.000  ..   42.000 :     3209.000 +/-       56.648
  //  90   89.000  ..   90.000 :     7016.000 +/-       83.762

  
  // find noisy pixels per chipID
  std::map<int, std::vector<std::pair<uint8_t, uint8_t>>> noisy_pixels;
  
  vector<string> vPrint;
  
  bool DBX(false);
  for (int chipID : unique_chipIDs) {
    // -- skip scintillator and bad chipIDs
    if (chipID >= 120) continue;
    if (skipList.end() != find(skipList.begin(), skipList.end(), chipID)) {
      //      continue;
    }
    
    noisy_pixels[chipID] = std::vector<std::pair<uint8_t, uint8_t> >();
    

    // -- try to find noise level
    TH2F *h2 = hitmaps.at(chipID);
    TH1F *h1 = noisemaps.at(chipID);
    for (int32_t ny = 1; ny <= h2->GetYaxis()->GetNbins(); ny++) {
      for (int32_t nx = 1; nx <= h2->GetXaxis()->GetNbins(); nx++) {
        noisemaps.at(chipID)->Fill(h2->GetBinContent(nx, ny));
      }
    }
    // -- determine noise_limit, based on modeNoiseLevel and noiseLevel
    double noise_limit(0.);
    if (1 == modeNoiseLimit) {
      noise_limit = noiseLevel;
    } else if (2 == modeNoiseLimit) {
      noise_limit = h1->GetMean() + noiseLevel*h1->GetRMS() + 0.5;
    }
    cout << "chipID " << chipID
         << " (maximum: " << h2->GetMaximum() << ") mean(nhit) = " << h1->GetMean()
	 << " RMS = " << h1->GetRMS()
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
      if (hErrors->GetBinContent(chipID+1) > 0) {
        vNoise.push_back(0x01);
      } else {
        vNoise.push_back(0x00);
      }
    }
    std::cout << " with a  total of " << tot_noisy_pixels << " (" << tot_noisy_pixels*100/64000 << "%)\n";
    writeNoiseMaskFile(vNoise, runnumber, chipID, name, dir);

    vPrint.push_back(Form("chipID %3d, n. level = %5.3f, N(n. pixels) = %d %s",
                          chipID, noise_limit, tot_noisy_pixels, spix.c_str()));
    vNoise.clear();
  }

  ofstream o(Form("%s/summaryNoiseMaskFile%s-run%d.txt", dir.c_str(), name.c_str(), runnumber)); 
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
void writeNoiseMaskFile(vector<uint8_t> noise, int runnumber, int chipID, string name, string dir) {
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


// ----------------------------------------------------------------------
// -- produce summary of a mask file
// ----------------------------------------------------------------------
void summarize(vector<uint8_t> vnoise) {
  for (unsigned int i = 0; i < vnoise.size(); ++i){
    pair<int, int> a = colrowFromIdx(i);
    if ((0xda == vnoise[i]) && (0xda == vnoise[i+1])) {
      i += 5; //??
      continue;
    }
    if (0xff != vnoise[i]) {
      cout << Form("pix: %d/%d ", a.first, a.second);
    }
  }
  cout  << endl;
}

// ----------------------------------------------------------------------
// -- produce summary of a mask file
// ----------------------------------------------------------------------
void summaryMaskFile(string filename, string dir = ".") {
  vector<uint8_t> vnoise = readMaskFile(Form("%s/%s", dir.c_str(), filename.c_str()));
  cout << filename << ": ";
  summarize(vnoise);
}



// ----------------------------------------------------------------------
// -- adds a run to the vector<uint8_t>
// ----------------------------------------------------------------------
void addNoiseMaskFile(vector<uint8_t> &vnoise, int runnumber, int chipID, string name, string dir = ".") {
  vector<uint8_t> vread = readMaskFile(Form("%s/noiseMaskFile%s-run%d-chipID%d", dir.c_str(), name.c_str(), runnumber, chipID));

  if (0 == vread.size()) {
    return;
  }

  for (unsigned int i = 0; i < vread.size(); ++i){
    pair<int, int> a = colrowFromIdx(i);
    if ((0xda == vnoise[i]) && (0xda == vnoise[i+1])) {
      i += 5; //??
      continue;
    }

    if ((0xff == vnoise[i]) && (0xff != vread[i])) {
      cout << Form("run %d change setting chip/col/row = %3d/%3d/%3d from %x to %x",
                   runnumber, chipID, a.first, a.second, vnoise[i], vread[i])
           << endl;
      vnoise[i] = vread[i];
    }
  }

}


// ----------------------------------------------------------------------
// -- combines all runs into one mask file
// ----------------------------------------------------------------------
void mergeNoiseFiles(int chipID, vector<int> runlist, string name, string dir) {
  vector<uint8_t> vnoise;
  for (int i = 0; i < 256*256; ++i) vnoise.push_back(0xff);

  for (unsigned int irun = 0; irun < runlist.size(); ++irun) {
    addNoiseMaskFile(vnoise, runlist[irun], chipID, name, dir);
  }

  writeNoiseMaskFile(vnoise, -1, chipID, name, dir);
}


// ----------------------------------------------------------------------
// -- main function reading all trees (producing per-run files) and
// -- merging all runs into single mask file
// ----------------------------------------------------------------------
void produceAllMergedNoiseFiles(int modeNoiseLimit = -1, double noiseLevel = 1.5,
                                string name = "", string dir = ".",
                                string rootdir = "/Users/ursl/data/mu3e/run2022/root_output_files/") {
  if (modeNoiseLimit < 1) {
    cout << "produceAllMergedNoiseFiles(int modeNoiseLimit, double noiseLevel, string name)" << endl;
    cout << " modeNoiseLimit = 1: noiseLevel provides absolute number of noise threshold, noise_limit = noiseLevel" << endl;
    cout << " modeNoiseLimit = 2: noiseLevel gives MSIGMA for noise_limit = <nhit> + noiseLevel*h1->RMS(nhit) + 0.5" << endl;
    cout << " -> make your choice and try again!" << endl;
    return;
  }
  //  vector<int> runlist = {215, 216, 220};
  //  vector<int> runlist = {311, 332};
  //  vector<int> runlist = {311, 332, 347};
  //  vector<int> runlist = {311, 332, 347, 360, 361, 362, 363, 364, 365, 366};

  // vector<int> runlist = {
  //   311, 312, 313,
  //   320, 321, 322, 323, 325,
  //   332, 333, 334, 336, 337,
  //   341, 343, 345, 346, 347, 348
  // };

  
  // -- all of May 2022
  // vector<int> runlist = {
  //   311, 312, 313,
  //   320, 321, 322, 323, 325,
  //   332, 333, 334, 336, 337,
  //   341, 343, 345, 346, 347, 348,
  //   350, 352, 353, 354, 355, 356, 357, 358, 359,
  //   360, 362, 363, 364, 365, 366
  // };

  // -- threshold lowering/masking attempt in June 2022
  vector<int> runlist = {311, 332, 347, 442, 443, 510, 512, 514, 518, 519, 520, 522, 524};

  
  for (unsigned int irun = 0; irun < runlist.size(); ++irun) {
    writeNoisyPixelsMaskFiles(rootdir, runlist[irun], modeNoiseLimit, noiseLevel, name, dir);
  }

  
  for (int i = 0; i < 120; ++i) {
    mergeNoiseFiles(i, runlist, name, dir);
  }


  for (int i = 0; i < 120; ++i) {
    summaryMaskFile(Form("noiseMaskFile%s-chipID%d", name.c_str(), i), dir);
  }

}

// ----------------------------------------------------------------------
void plotLVDS(int run) {
  TCanvas c0;
  TLatex *tl = new TLatex();
  gStyle->SetOptStat(0);
  TH1D *hRatio = (TH1D*)gROOT->FindObject("hRatio"); hRatio->SetLineColor(kRed);
  TH1D *hTotal = (TH1D*)gROOT->FindObject("hTotal");
  hRatio->Draw("hist");
  hTotal->Scale(1./hTotal->GetEntries());
  hTotal->Draw("samehist");
  tl->SetTextSize(0.04);
  tl->SetTextColor(kRed);   tl->DrawLatexNDC(0.7, 0.2, "error ratio");
  tl->SetTextColor(kBlack); tl->DrawLatexNDC(0.7, 0.15, "hits (a.u.)");
  c0.SaveAs(Form("rowOverflow-%d.pdf", run));

  for (int i = 1; i < hRatio->GetNbinsX(); ++i) {
    if (hRatio->GetBinContent(i) > 0.) {
      hRatio->SetBinContent(i, 0.);
    } else {
      // -- set to one only for chips that have hits
      if (hTotal->GetBinContent(i) > 0.) {
        cout << "Chip w/o row overflows: " << i-1 << endl;
        hRatio->SetBinContent(i, 1.);
      }
    }
  }

  hRatio->SetTitle("chips without row overflows");
  hRatio->SetLineColor(kBlack);
  hRatio->Draw("hist");
  c0.SaveAs(Form("noRowOverflows-%d.pdf", run));
  
}
