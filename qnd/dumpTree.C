struct data {
  int i;
  float px;
};

// ----------------------------------------------------------------------
void dumpTree() {
  TFile *f = TFile::Open("dumpTree.root", "RECREATE");
  TTree *t = new TTree("redTree", "redTree");

  struct data tdata;

  t->Branch("i", &tdata.i,"i/I");
  t->Branch("vx",&tdata.px,"px/F");  

  for (Int_t i=0; i<10000; i++) {
    tdata.i = i; 
    tdata.px = gRandom->Gaus(i, 20.);
    t->Fill();
  }

  f->Write();
  
}

// ----------------------------------------------------------------------
void readTree() {
  TFile *f = TFile::Open("dumpTree.root");
  TTree *t = (TTree*)gFile->Get("redTree");
  t->Draw("px:i", "", "colz");
}
