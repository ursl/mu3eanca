// -- original by MK
// -- changed to reflect new data structure in 2022

using namespace::std;


map<int, vector<pair<int, int> > > gChipNoisyPixels; 

// ----------------------------------------------------------------------
vector<uint8_t> readFile(string filename) {
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
    pair<int, int> a = colrowFromIdx(i);
    if ((0 != vnoise[i]) && (0xda != vnoise[i])) {
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
// -- adds a run to the vector<uint8_t>
// ----------------------------------------------------------------------
int addNoiseMaskFile(vector<uint8_t> &vnoise, int runnumber, int chipID) {
  vector<uint8_t> vread = readFile(Form("noiseMaskFile-run%d-chipID%d", runnumber, chipID));

  if (0 == vread.size()) {
    return -1;
  }

  for (unsigned int i = 0; i < vread.size(); ++i){
    pair<int, int> a = colrowFromIdx(i);
    if ((0 == vnoise[i]) && (0 != vread[i])) {
      if (0xda != vread[i]) cout << Form("run %d setting col/row = %3d/%3d to %x",
                                         runnumber, a.first, a.second, vread[i])
                                 << endl;
      vnoise[i] = vread[i];
    }
  }
  return 0;
}


// ----------------------------------------------------------------------
// -- combines all runs into one mask file
// ----------------------------------------------------------------------
vector<uint8_t> mergeNoiseFiles(int chipID, int& fine) {
  vector<int> runlist = {215, 216, 220};
  vector<uint8_t> vnoise;
  for (int i = 0; i < 256*256; ++i) vnoise.push_back(0);

  for (unsigned int irun = 0; irun < runlist.size(); ++irun) {
    int ok = addNoiseMaskFile(vnoise, runlist[irun], chipID);
  }

  // -- check that file(s) read (if at all) had non-zero entries
  fine = 0; 
  for (unsigned int i = 0; i < vnoise.size(); ++i){
    if (0 != vnoise[i]) {
      fine = 1;
      break;
    }
  }
  
  return vnoise; 
}


// ----------------------------------------------------------------------
// -- fill gChipNoisyPixels for a chipID
// ----------------------------------------------------------------------
void fillNoisyPixels(int chipID, vector<uint8_t> &vnoise) {
  vector<pair<int, int> > vnp;
  for (unsigned int i = 0; i < vnoise.size(); ++i){
    if ((0 != vnoise[i]) && (0xda != vnoise[i])) {
      pair<int, int> a = colrowFromIdx(i);
      vnp.push_back(a);
    }
  }
  gChipNoisyPixels.insert(make_pair(chipID, vnp));  
}


// ----------------------------------------------------------------------
void fillAllNoisyPixels() {
  int OK(0); 
  for (int i = 0; i < 128; ++i) {
    vector<uint8_t> vnoise = mergeNoiseFiles(i, OK);
    if (1 == OK) {
      fillNoisyPixels(i, vnoise);
    } else {
      cout << "did not find a masknoisefile for chip " << i << ", not filling noisy pixels" << endl;
    }
  }

  TH1D *h1 = new TH1D("hnoise", "noisy pixels/chips", 100, 0., 100.);
  map<int, vector<pair<int, int> > >::iterator it; 
  for (it = gChipNoisyPixels.begin(); it != gChipNoisyPixels.end(); ++it) {
    cout << "chip " << it->first << " it->size() = " << it->second.size() << endl;
    h1->Fill(it->second.size());
  }

  h1->Draw();

  c0.SaveAs("nNoisyPixels.pdf");
            
}
