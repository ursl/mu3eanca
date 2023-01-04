#include <TH1.h>

// ----------------------------------------------------------------------
void printSensors(string fname = "run000042-sort_100k.root") {
  if (!gFile) {
    TFile *f = TFile::Open(fname.c_str());
  }

  TTree *ts = (TTree*)gFile->Get("alignment/sensors");
  //  ts->Print();
  unsigned int sensor(0);
  int mysensor(0), mySimChipId(0), ldrmax(0);
  unsigned int chip(0), ladder(0), layer(0), rest(0); 
  double vx(0.), vy(0.), vz(0.);
  
  ts->SetBranchAddress("sensor", &sensor);
  ts->SetBranchAddress("vx", &vx);
  ts->SetBranchAddress("vy", &vy);
  ts->SetBranchAddress("vz", &vz);

  Long64_t nentries = ts->GetEntries();
  cout << "Idx  algn/sensorId (mySimId)  .. etc" << endl;

  for (int ievt = 0; ievt < nentries; ++ievt) {
    ts->GetEntry(ievt);
    TVector3 r(vx, vy, vz);

    chip   = (sensor & 0x1f);
    ladder = (sensor >> 5) & 0x1f;
    layer  = (sensor >> 10) & 0x1f;
    rest   = (sensor >> 15);

    mysensor    = layer*1024 + ladder*32 + chip;
    if (0 == layer) {
      ldrmax = 8;
      if (ladder > 4) ladder -= 4; 
    } else if (1 == layer) {
      if (ladder > 5) ladder -= 2; 
      ldrmax = 12;
    } else {
      ldrmax = 1000;
    }
    mySimChipId = layer*48   + ladder*6 + (6-chip);
    
    cout << Form("%3d: sensor = %4d (%4d) chip/ldr/lyr = %2d/%2d/%2d r = %5.2f phi = %6.1f z = %+7.3f x/y = %+8.4f/%+8.4f",
		 ievt, sensor, mySimChipId, chip, ladder, layer,
		 TMath::Sqrt(vx*vx + vy*vy), r.Phi()*TMath::RadToDeg(), vz,
		 vx, vy)
	 << endl;
  }
  
}
