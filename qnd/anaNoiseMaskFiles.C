// -- original by MK
// -- changed to reflect new data structure in 2022

using namespace::std;


map<int, vector<pair<int, int> > > gChipNoisyPixels; 


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
void fillAllNoisyPixels(string dir = ".") {
  int OK(0); 
  gChipNoisyPixels.clear();

  for (int i = 0; i < 128; ++i) {
    vector<uint8_t> vnoise = readFile(Form("%s/noiseMaskFile-chipID%d", dir.c_str(), i));
    if (validNoise(vnoise)) {
      fillNoisyPixels(i, vnoise);
    }
  }

  TH1D *h1 = new TH1D("hnoise", Form("noisy pixels/chips (%s)", dir.c_str()), 50, 0., 1000.);
  map<int, vector<pair<int, int> > >::iterator it; 
  for (it = gChipNoisyPixels.begin(); it != gChipNoisyPixels.end(); ++it) {
    cout << "chip " << it->first << " it->size() = " << it->second.size() << endl;
    h1->Fill(it->second.size());
  }
  h1->Draw();
  if (dir == ".") {
    c0.SaveAs("nNoisyPixels.pdf");
  } else {
    c0.SaveAs(Form("nNoisyPixels-%s.pdf", dir.c_str()));
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
    for (int i = 0; i < 128; ++i) {
      vector<uint8_t> vnoise = readFile(Form("%s/noiseMaskFile-chipID%d", dirs[idir].c_str(), i));
      if (validNoise(vnoise)) {
        fillNoisyPixels(i, vnoise);
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
