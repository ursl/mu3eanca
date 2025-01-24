#include "trBase.hh"
#include "trGen.hh"
#include "trIncludes.hh"

// ----------------------------------------------------------------------
// Run with: bin/runTreeReader -t mu3e -f data/mu3e_run_000779.root -D results/
// ----------------------------------------------------------------------


// -- 2 Example events:
//    Note: vertex gives particle origin
//          type 41,42 are positron, electron from conversions
//
// ----------------------------------------------------------------------
// mu3e evt: 10 event: 90 run: 779 type: -1: fNtrajectories = 5:  : fNhit = 4:
// trj  0 ID =    4159 PID =  -13 type =   3 mother ID =       0 vz = -1000.000 vr =   +5.688 time = 34.589
// trj  1 ID =    4163 PID =  -13 type =   3 mother ID =       0 vz = -1000.000 vr =   +5.281 time = 13.2837
// trj  2 ID =    4170 PID =  -13 type =   3 mother ID =       0 vz = -1000.000 vr =  +10.193 time = 45.3868
// trj  3 ID =    4092 PID =  -13 type =   3 mother ID =       0 vz = -1000.000 vr =   +2.402 time = -16.2756
// trj  4 ID =    1425 PID =  -11 type =  11 mother ID =    1416 vz =   +62.250 vr =  +31.640 time = 46.5969
// hit   0 pxhitid = 4525760 timestamp = 1429 hit_mc_i =   81 hit_mc_n =    1
// hit   1 pxhitid = 19239643 timestamp = 1432 hit_mc_i =   82 hit_mc_n =    1
// hit   2 pxhitid = 23329223 timestamp = 1425 hit_mc_i =   83 hit_mc_n =    1
// hit   3 pxhitid = 86360979 timestamp = 1432 hit_mc_i =   84 hit_mc_n =    1
// ----------------------------------------------------------------------
//
// .. snip ..
//
//  ----------------------------------------------------------------------
// mu3e evt: 13 event: 93 run: 779 type: -1: fNtrajectories = 9:  : fNhit = 4:
// trj  0 ID =    4375 PID =  -13 type =   3 mother ID =       0 vz = -1000.000 vr =  +14.799 time = 34.4954
// trj  1 ID =    4169 PID =  -11 type =  11 mother ID =    4163 vz =   -34.456 vr =   +5.889 time = 6.11924
// trj  2 ID =    4381 PID =  +22 type =  60 mother ID =    4169 vz =  +313.954 vr =  +34.922 time = 8.80184
// trj  3 ID =    4407 PID =  -11 type =  41 mother ID =    4381 vz =  +505.057 vr = +507.920 time = 10.7073
// trj  4 ID =    4406 PID =  +11 type =  42 mother ID =    4381 vz =  +505.057 vr = +507.920 time = 10.7073
// trj  5 ID =    3903 PID =  -11 type =  11 mother ID =    3888 vz =   +35.351 vr =   +5.547 time = 34.3446
// trj  6 ID =    4439 PID =  +22 type =  60 mother ID =    3903 vz =  -178.997 vr =  +30.122 time = 36.5057
// trj  7 ID =    4456 PID =  +11 type =  82 mother ID =    4439 vz =  -444.659 vr = +506.566 time = 38.5018
// trj  8 ID =    1886 PID =  -11 type =  11 mother ID =    1876 vz =   +21.899 vr =  +10.668 time = 22.5846


// Mu3eEvent.cc:
// if(ptype >= 0 && dtype >= 0) trajType = 10 * dtype + ptype;

// Mu3eTrajectory.h::Mu3eTrackInfo
// * 0     - gamma (photon)
// * 1,2   - e^+,-
// * 3,4   - mu^+,-
// * 5,6   - pi^+,-
// * 7     - nu_e/nu_mu/nu_tau

// * Decay (origin) type:
// * 1     - michel
// * 2     - radiative
// * 3     - internal conversion
// * 4-8   - qed (ioni, phot, compt, conv)
// * 9     - signal
// * 10    - familon signal
// * 11-13 - decay with dark photon




// ----------------------------------------------------------------------
trGen::trGen(TChain *chain, string treeName) : trBase(chain, treeName) {
  cout << "==> trGen: constructor..." << endl;

  initMu3e();
  initMu3e_mchits();
  initVariables();
  initAlignment();

}


// ----------------------------------------------------------------------
void trGen::commonVar() {
  fRun = fHeader.run;
  fEvt = fHeader.event;
}


// ----------------------------------------------------------------------
void trGen::startAnalysis() {
  cout << "trGen: startAnalysis: ..." << endl;
}

// ----------------------------------------------------------------------
void trGen::endAnalysis() {
  cout << "trGen: endAnalysis: ..." << endl;
}

// ----------------------------------------------------------------------
void trGen::closeHistFile() {
  cout << "==> trGen::closeHistFile() Writing " << fpHistFile->GetName() << endl;
  fpHistFile->cd();
  fpHistFile->Write();
  fpHistFile->Close();
  delete fpHistFile;
}


// ----------------------------------------------------------------------
void trGen::eventProcessing() {

  initVariables();

  if (fVerbose > -1) {
    cout << "----------------------------------------------------------------------" << endl;
  }

  // -- generic debug output
  if (fVerbose > 90) {
    printBranches();
  }

  fillHist();
  genStudy();
  overlapHitsInVertex();


}


// ----------------------------------------------------------------------
void trGen::overlapHitsInVertex() {
  static bool first (true); 
  if (first) {
    first = false; 

    fpHistFile->cd();
    new TH1D("hSensorRadius", "sensor radius", 1000, 0., 100.);
    new TH1D("hHitDist", "hit distance (for same traj)", 1000, 0., 500.);

    new TH2D("hL1Sensors", "sensors L1", 6, -62., 62., 8, -3.15, 3.15);
    new TH2D("hL2Sensors", "sensors L2", 6, -62., 62., 10, -3.15, 3.15);

    new TH2D("hL1XY", "Layer 1", 700, -35., 35., 700, -35., 35.);
    new TH2D("hL2XY", "Layer 2", 700, -35., 35., 700, -35., 35.);
  }      

  TH2D *hl1 = (TH2D*)fpHistFile->Get("hL1Sensors");
  TH2D *hl2 = (TH2D*)fpHistFile->Get("hL2Sensors");

  TH2D *hxy1 = (TH2D*)fpHistFile->Get("hL1XY");
  TH2D *hxy2 = (TH2D*)fpHistFile->Get("hL2XY");
  
  TH1D *hr = (TH1D*)fpHistFile->Get("hSensorRadius");
  TH1D *hd = (TH1D*)fpHistFile->Get("hHitDist");

  mapTID2PixelID();
  
  // -- plot all hit locations
  for (unsigned i = 0; i < fNhit; ++i) {
    uint32_t id = fhit_pixelid->at(i);

    TVector3 pix3D =  getHitLocation(id);

    int pixID = pixelID(id);
    int pixRow = pixelRow(id);
    int pixCol = pixelCol(id);

    double radius = pix3D.Perp();
    double phi    = pix3D.Phi();
    double z      = pix3D.Z();
    
    hr->Fill(radius); 

    int ix, iy;
    if (radius < 28.) {
      ix = hl1->GetXaxis()->FindBin(z); 
      iy = hl1->GetYaxis()->FindBin(phi); 
      hl1->SetBinContent(ix, iy, pixID);
      hxy1->Fill(pix3D.X(), pix3D.Y());
      //      cout << "  hl1->SetBinContent(" << i << ", " << iy << ", " << pixID << ");" << endl;
    } else if (radius < 34.) {
      ix = hl2->GetXaxis()->FindBin(z); 
      iy = hl2->GetYaxis()->FindBin(phi); 
      hl2->SetBinContent(ix, iy, pixID);
      hxy2->Fill(pix3D.X(), pix3D.Y());
      //      cout << "  hl2->SetBinContent(" << i << ", " << iy << ", " << pixID << ");" << endl;
    }

    if (fVerbose > 1) {
      cout << i << ": 0x" << hex << id  << dec << " pixID = " << pixID
           << " at " << fSensors[pixID].vx << "/" <<  fSensors[pixID].vy << "/" <<  fSensors[pixID].vz 
           << " row/col = " << pixRow << "/" <<  pixCol
           << " radius = " << radius << " phi = " << phi << " z = " << z 
           << " ix = " << ix << " iy = " << iy
           << " mc_i = " << fhit_mc_i->at(i) << " mc_n = " << fhit_mc_n->at(i)
           << " tid = " << ftid
           << endl;
    }
    
  }

  // -- plot distances between hit pairs on tid
  for (auto it: fMapTID2Hits) {
    for (unsigned int i = 0; i < it.second.size(); ++i) {
      TVector3 ri = getHitLocation(it.second[i]);
      for (unsigned int j = i+1; j < it.second.size(); ++j) {
        TVector3 rj = getHitLocation(it.second[j]);
        TVector3 diff = ri - rj;
        double dist = diff.Mag();
        hd->Fill(dist);
      }
    }
  }
  



}


// ----------------------------------------------------------------------
void trGen::genStudy() {

  LorentzVector<PxPyPzMVector> p4;
  for (unsigned int i = 0; i < fNtrajectories; ++i) {
    ((TH1D*)fpHistFile->Get("hproc"))->Fill(ftraj_type->at(i));
    // -- Michel decay electrons
    if ((-11 == ftraj_PID->at(i)) && (11 == ftraj_type->at(i))) {
      double px = ftraj_px->at(i);
      double py = ftraj_py->at(i);
      double pz = ftraj_pz->at(i);
      p4  = LorentzVector<PxPyPzMVector>(px, py, pz, MMUON);
      double r = TMath::Sqrt(ftraj_vx->at(i)*ftraj_vx->at(i) + ftraj_vy->at(i)*ftraj_vy->at(i));
      ((TH1D*)fpHistFile->Get("hmichel"))->Fill(p4.Rho());
      ((TH2D*)fpHistFile->Get("vrzmichel"))->Fill(ftraj_vz->at(i), r);
      ((TH2D*)fpHistFile->Get("vxymichel"))->Fill(ftraj_vx->at(i), ftraj_vy->at(i));
    }
    // -- converted photons
    if ((-11 == ftraj_PID->at(i)) && (41 == ftraj_type->at(i))) {
      double r = TMath::Sqrt(ftraj_vx->at(i)*ftraj_vx->at(i) + ftraj_vy->at(i)*ftraj_vy->at(i));
      ((TH1D*)fpHistFile->Get("vrzconv"))->Fill(ftraj_vz->at(i), r);
      ((TH1D*)fpHistFile->Get("vxyconv"))->Fill(ftraj_vx->at(i), ftraj_vy->at(i));
    }
  }
}

// ----------------------------------------------------------------------
void trGen::fillHist() {
  TH1D *hpx = (TH1D*)fpHistFile->Get("hpx");

  for (unsigned int i = 0; i < ftraj_px->size(); ++i) {
    hpx->Fill(ftraj_px->at(i));
  }

  fTree->Fill();
}

// ----------------------------------------------------------------------
void trGen::bookHist() {
  trBase::bookHist();
  cout << "==> trGen: bookHist> " << endl;

  new TH1D("hpx", "hpx", 100, -100., 100.);
  new TH1D("hmichel", "hmichel", 60, 0., 60.);
  new TH2D("vrzmichel", "vmichel (r vs. z)", 200, -1500., 500., 120, 0., 60.);
  new TH2D("vxymichel", "vmichel (x vs. y)", 100, -100., 100., 100, -100., 100.);

  new TH2D("vrzconv", "vconv (r vs. z)", 200, -1500., 500., 120, 0., 60.);
  new TH2D("vxyconv", "vconv (x vs. y)", 100, -100., 100., 100, -100., 100.);

  TH1D *hproc = new TH1D("hproc", "Processes and particles", 100, 0., 100.);
  hproc->GetXaxis()->SetLabelSize(0.016);
  hproc->GetXaxis()->SetBinLabel(hproc->FindBin(3.), "Beam #mu^{+}");

  hproc->GetXaxis()->SetBinLabel(hproc->FindBin(11.), "Michel e^{+}");
  //  hproc->GetXaxis()->SetBinLabel(hproc->FindBin(12.), "Michel e^{-}");

  hproc->GetXaxis()->SetBinLabel(hproc->FindBin(20.), "Radiative #gamma");
  hproc->GetXaxis()->SetBinLabel(hproc->FindBin(21.), "Radiative e^{+}");
  //  hproc->GetXaxis()->SetBinLabel(hproc->FindBin(22.), "Radiative e^{-}");

  hproc->GetXaxis()->SetBinLabel(hproc->FindBin(41.), "Conv. e^{+}");
  hproc->GetXaxis()->SetBinLabel(hproc->FindBin(42.), "Conv. e^{-}");

  //  hproc->GetXaxis()->SetBinLabel(hproc->FindBin(51.), "Some e^{+}");
  hproc->GetXaxis()->SetBinLabel(hproc->FindBin(52.), "Some e^{-}");

  hproc->GetXaxis()->SetBinLabel(hproc->FindBin(60.), "Some #gamma");
  hproc->GetXaxis()->SetBinLabel(hproc->FindBin(70.), "Some #gamma");
  hproc->GetXaxis()->SetBinLabel(hproc->FindBin(82.), "Some e^{-}");

  for (int i = 1; i < 100; ++i) {
    hproc->GetXaxis()->ChangeLabel(i, 60.);
  }
}


// ----------------------------------------------------------------------
void trGen::initVariables() {
  //  cout << "trBase: initVariables: for run = " << fRun << "/evt = " << fEvt << endl;

}


// ----------------------------------------------------------------------
void trGen::printBranches() {

  cout << "----------------------------------------------------------------------" << endl;
  cout << "mu3e evt: " << fChainEvent
       << " event: " << fHeader.event
       << " run: "  << fHeader.run
       << " type: " << fHeader.type
       << ": fNtrajectories = " << fNtrajectories << ":  "
       << ": fNhit = " << fNhit << ":  "
       << endl;
  for (unsigned int i = 0; i < ftraj_ID->size(); ++i) {
    cout << Form("trj %2d", i)
         << Form(" ID = %7d", ftraj_ID->at(i))
         << Form(" PID = %+4d", ftraj_PID->at(i))
         << Form(" type = %3d", ftraj_type->at(i))
         << Form(" mother ID = %7d", ftraj_mother->at(i))
         << Form(" vz = %+9.3f", ftraj_vz->at(i))
         << Form(" vr = %+8.3f", TMath::Sqrt(ftraj_vx->at(i)*ftraj_vx->at(i) + ftraj_vy->at(i)*ftraj_vy->at(i)))
         << " time = " << ftraj_time->at(i)
         << endl;
  }
  for (unsigned int i = 0; i < fNhit; ++i) {
    cout << Form("hit %3d", i)
         << Form(" pxhitid = %4d", fhit_pixelid->at(i))
         << Form(" timestamp = %4d", fhit_timestamp->at(i))
         << Form(" hit_mc_i = %4d", fhit_mc_i->at(i))
         << Form(" hit_mc_n = %4d", fhit_mc_n->at(i))
         << endl;

    //    cout << Form("  mchit det = %2d", 
  }
  cout << "----------------------------------------------------------------------" << endl;
}




// ----------------------------------------------------------------------
trGen::~trGen() {
  cout << "==> trGen: destructor ..." << endl;
  if (!fpChain) return;
  delete fpChain->GetCurrentFile();
}


// --------------------------------------------------------------------------------------------------
void trGen::readCuts(string filename, int dump) {
  char  buffer[200];
  fCutFile = filename;
  if (dump) cout << "==> trGen: Reading " << fCutFile << " for cut settings" << endl;
  sprintf(buffer, "%s", fCutFile.c_str());
  ifstream is(buffer);
  char CutName[100];
  float CutValue;
  int ok(0);

  string fn(fCutFile);

  if (dump) {
    cout << "====================================" << endl;
    cout << "==> trGen: Cut file  " << fCutFile << endl;
    cout << "------------------------------------" << endl;
  }

  TH1D *hcuts = new TH1D("hcuts", "", 1000, 0., 1000.);
  hcuts->GetXaxis()->SetBinLabel(1, fn.c_str());
  int ibin;

  while (is.getline(buffer, 200, '\n')) {
    ok = 0;
    if (buffer[0] == '#') {continue;}
    if (buffer[0] == '/') {continue;}
    sscanf(buffer, "%s %f", CutName, &CutValue);

    if (!strcmp(CutName, "TYPE")) {
      TYPE = int(CutValue); ok = 1;
      if (dump) cout << "TYPE:           " << TYPE << endl;
    }

    if (!strcmp(CutName, "PTLO")) {
      PTLO = CutValue; ok = 1;
      if (dump) cout << "PTLO:           " << PTLO << " GeV" << endl;
      ibin = 11;
      hcuts->SetBinContent(ibin, PTLO);
      hcuts->GetXaxis()->SetBinLabel(ibin, "p_{T}^{min}(l) [GeV]");
    }

    if (!strcmp(CutName, "PTHI")) {
      PTHI = CutValue; ok = 1;
      if (dump) cout << "PTHI:           " << PTHI << " GeV" << endl;
      ibin = 12;
      hcuts->SetBinContent(ibin, PTHI);
      hcuts->GetXaxis()->SetBinLabel(ibin, "p_{T}^{max}(l) [GeV]");
    }


    if (!ok) cout << "==> trBase: ERROR: Don't know about variable " << CutName << endl;
  }

  if (dump)  cout << "------------------------------------" << endl;
}


// ----------------------------------------------------------------------
void trGen::mapTID2PixelID() {
  fMapTID2Hits.clear();

  if (fVerbose > 9) {
    for (unsigned i = fhit_mc_i->at(0); i < fhit_mc_i->at(fNhit-1); ++i) {
      fpChain2->GetEntry(i);
      cout << "det: " << fdet << " tid: " << ftid << endl;
    }
  }

  for (unsigned i = 0; i < fNhit; ++i) {
    int pixelid = fhit_pixelid->at(i);
    int idx     = fhit_mc_i->at(i);
    fpChain2->GetEntry(idx);
    if (fMapTID2Hits.find(ftid) == fMapTID2Hits.end()) {
      // -- not found
      vector<int> a; 
      a.push_back(pixelid);
      fMapTID2Hits.insert({ftid, a});
    } else {
      // -- found
      fMapTID2Hits[ftid].push_back(pixelid);
    }
  }

  if (fVerbose > 9) {
    for (auto it: fMapTID2Hits) {
      cout << it.first << ": ";
      for (auto iv: it.second) {
        cout << iv << ", ";
      }
      cout << endl;
    }
  }

}


// ----------------------------------------------------------------------
TVector3  trGen::getHitLocation(uint32_t id) {
  // int pixID  = (id & (0xffff << 16)) >> 16;
  // int pixRow = (id & 0x000000ff);
  // int pixCol = (id & 0x0000ff00) >> 8;

  int pixID = pixelID(id);
  int pixRow = pixelRow(id);
  int pixCol = pixelCol(id);

  // v + drow * (0.5 + row) + dcol * (0.5 + col)
  TVector3 v(fSensors[pixID].vx, fSensors[pixID].vy, fSensors[pixID].vz);
  TVector3 dcol(fSensors[pixID].colx, fSensors[pixID].coly, fSensors[pixID].colz);
  TVector3 drow(fSensors[pixID].rowx, fSensors[pixID].rowy, fSensors[pixID].rowz);
  TVector3 pix3D = v + drow * (0.5 + pixRow) + dcol * (0.5 + pixCol);
  
  return pix3D;

}
