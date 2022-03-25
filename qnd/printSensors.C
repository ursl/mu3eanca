#include <TH1.h>

// ----------------------------------------------------------------------
void printSensors(string fname = "run000042-sort_100k.root") {
  if (!gFile) {
    TFile *f = TFile::Open(fname.c_str());
  }

  TTree *ts = (TTree*)gFile->Get("alignment/sensors");
  //  ts->Print();
  unsigned int sensor(0);
  double vx(0.), vy(0.), vz(0.);
  
  ts->SetBranchAddress("sensor", &sensor);
  ts->SetBranchAddress("vx", &vx);
  ts->SetBranchAddress("vy", &vy);
  ts->SetBranchAddress("vz", &vz);

  Long64_t nentries = ts->GetEntries();
  for (int ievt = 0; ievt < nentries; ++ievt) {
    ts->GetEntry(ievt);
    TVector3 r(vx, vy, vz);
    cout << Form("%3d: sensor = %4d r = %5.2f phi = %6.1f z = %+7.3f  x/y = %+8.4f/%+8.4f",
		 ievt, sensor, TMath::Sqrt(vx*vx + vy*vy), r.Phi()*TMath::RadToDeg(), vz,
		 vx, vy)
	 << endl;
  }
  
}
