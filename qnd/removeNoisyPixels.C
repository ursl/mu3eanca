// -- original by MK
// -- changed to reflect new data structure in 2022
void remove_noisy_pixels(const std::string& filename) {

  // open file
  auto * infile = new TFile(filename.c_str(), "OPEN");
  std::stringstream ss;

  // get tree from file
  auto * tree = (TTree *) infile->FindObjectAny("HitData");
  auto * fTree = (TTree*) infile->FindObjectAny("HitData");

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
  for (int i=0; i<128; i++) {
    ss.str("");
    ss << "hitmap" << i;
    hitmaps.push_back(new TH2F(ss.str().c_str(), ss.str().c_str(), 256, 0, 256, 250, 0, 250));
  }

  uint8_t fcol;
  uint8_t frow;
  uint8_t fchipID;
  fTree->SetBranchAddress("col", &fcol);
  fTree->SetBranchAddress("row", &frow);
  fTree->SetBranchAddress("chipID", &fchipID);
  uint64_t fnentries= fTree->GetEntries();
  for (Long64_t i=0; i<fnentries; i++) {
    if(i%(fnentries/100) == 0){
      std::cout<<"processing .. "<<(int) i/(fnentries/100)<< " %\r";
      std::cout.flush();
    }
    fTree->GetEntry(i);
    if (fchipID >= 128)
      continue;
    hitmaps.at(fchipID)->Fill(fcol,frow);
  }
  std::cout<<std::endl;

  // find noisy pixels per chipID
  std::map<int, std::vector<std::pair<uint8_t, uint8_t>>> noisy_pixels;
  std::uint64_t hits_total = 0;
  std::uint64_t noise_limit = 0;

  for (int chipID : unique_chipIDs) {
    hits_total = 0;
    if (chipID >= 128)
      continue;
    for (int32_t nx = 5; nx <= 245; nx++) {
      for (int32_t ny = 5; ny <= 245; ny++) {
        hits_total = hits_total + hitmaps.at(chipID)->GetBinContent(nx, ny);
      }
    }
    hits_total = hits_total * 1.111111; // correct for missing pixels at the chip border
    noise_limit = 10 * hits_total/64000;
    noisy_pixels[chipID] = std::vector<std::pair<uint8_t, uint8_t> >();

    std::cout << "chipID: "<< chipID<<std::endl;

    float tot_noisy_pixels = 0;
    for (int32_t nx = 1; nx <= hitmaps.at(chipID)->GetXaxis()->GetNbins(); nx++) {
      for (int32_t ny = 1; ny <= hitmaps.at(chipID)->GetYaxis()->GetNbins(); ny++) {
        if (hitmaps.at(chipID)->GetBinContent(nx, ny) > noise_limit) {
          std::cout << "Chip ID " << chipID << ", Found noisy pixel at " << nx-1 << ", " << ny-1 << std::endl;
          tot_noisy_pixels++;
          // -1 since bin 1/1 of GetBinContent is pixel 0/0
          noisy_pixels[chipID].push_back(std::pair<uint8_t, uint8_t>(nx-1, ny-1));
        }
      }
    }
    std::cout << "Found total of " << tot_noisy_pixels << " (" << tot_noisy_pixels*100/64000 << "%)\n";
  }

  // read tree in
  TFile file(filename.c_str());

  uint32_t runID;
  uint32_t MIDASEventID;
  uint8_t  col;
  uint8_t  row;
  uint8_t  fpgaID;
  uint8_t  chipID;
  uint8_t  chipIDRaw;
  uint8_t  tot;
  uint32_t hitTime;
  uint32_t headerTime;
  uint32_t headerTimeMajor;
  uint32_t subHeaderTime;
  uint8_t  isMUSR;

  tree->SetBranchAddress("runID", &runID);
  tree->SetBranchAddress("MIDASEventID", &MIDASEventID);
  tree->SetBranchAddress("col", &col);
  tree->SetBranchAddress("row", &row);
  tree->SetBranchAddress("fpgaID", &fpgaID);
  tree->SetBranchAddress("chipID", &chipID);
  tree->SetBranchAddress("chipIDRaw", &chipIDRaw);
  tree->SetBranchAddress("tot", &tot);
  tree->SetBranchAddress("hitTime", &hitTime);
  tree->SetBranchAddress("headerTime", &headerTime);
  tree->SetBranchAddress("headerTimeMajor", &headerTimeMajor);
  tree->SetBranchAddress("subHeaderTime", &subHeaderTime);
  tree->SetBranchAddress("isMUSR", &isMUSR);

  // create new tree
  std::string outfile = filename;
  boost::replace_all(outfile,".root","_cleanup.root");
  auto * f = new TFile(outfile.c_str(),"RECREATE");

  auto * treeNew = new TTree("T","Hit data");

  TBranch * b_runID = treeNew->Branch("runID", &runID);
  TBranch * b_MIDASEventID = treeNew->Branch("MIDASEventID", &MIDASEventID);
  TBranch * b_col = treeNew->Branch("col", &col);
  TBranch * b_row = treeNew->Branch("row", &row);
  TBranch * b_fpgaID = treeNew->Branch("fpgaID", &fpgaID);
  TBranch * b_chipID = treeNew->Branch("chipID", &chipID);
  TBranch * b_chipIDRaw = treeNew->Branch("chipIDRaw", &chipIDRaw);
  TBranch * b_tot = treeNew->Branch("tot", &tot);
  TBranch * b_hitTime = treeNew->Branch("hitTime", &hitTime);
  TBranch * b_headerTime = treeNew->Branch("headerTime", &headerTime);
  TBranch * b_headerTimeMajor = treeNew->Branch("headerTimeMajor", &headerTimeMajor);
  TBranch * b_subHeaderTime = treeNew->Branch("subHeaderTime", &subHeaderTime);
  TBranch * b_isMUSR = treeNew->Branch("isMUSR", &isMUSR);
    

  uint64_t nentries= tree->GetEntries();
  uint32_t prev_MIDASEventID = 0;
  uint32_t eventSize = 0;
  std::cout << "filtering start " << std::endl;

  const clock_t begin_time = clock();

  for(uint64_t i = 0; i<nentries-1; i++){
    // get next root event
    tree->GetEntry(i);
    if (chipID >= 128)
      continue;
    if(chipIDRaw > 11)
      continue;
    if(i%(nentries/100) == 0){
      std::cout<<" processing .. "<<(int) i/(nentries/100)<< " %,  "<<  (int) 60*i/(float(clock () - begin_time)/(CLOCKS_PER_SEC))<<" Events/min,";
      std::cout<<"  ETA: "<<(int) ((nentries-i)/ ((int) 60*i/(float(clock () - begin_time)/(CLOCKS_PER_SEC))))<<" minutes           "<<"\r";
      std::cout.flush();
    }

    if(!(noisy_pixels.find(chipID) != noisy_pixels.end() && std::binary_search(noisy_pixels.at(chipID).begin(), noisy_pixels.at(chipID).end(), std::pair<uint8_t, uint8_t>(col, row)))){
      treeNew->Fill();
    }

  }
  f->Write();
}
