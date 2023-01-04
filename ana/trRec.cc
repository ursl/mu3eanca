#include "trBase.hh"
#include "trRec.hh"
#include "trIncludes.hh"

// ----------------------------------------------------------------------
// Run with: bin/runTreeReader -t segs -f data/mu3e_trirec_000779.root -D results/
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
trRec::trRec(TChain *chain, string treeName) : trBase(chain, treeName) {
  cout << "==> trRec: constructor..." << endl;

  if (string::npos != fChainName.find("segs")) {
    fMode = SEGS;
  } else if (string::npos != fChainName.find("frames")) {
    fMode = FRAMES;
  }
  if (FRAMES == fMode) {
    initFrames();
  } else if (SEGS == fMode) {
    initSegs();
  }

}

// ----------------------------------------------------------------------
void trRec::commonVar() {
  if (FRAMES == fMode) {
    fRun = frunId;
    fEvt = feventId;
  } else if (SEGS == fMode) {
    fRun = fSegsInt["runId"];
    fEvt = fSegsInt["eventId"];
  }
}

// ----------------------------------------------------------------------
void trRec::startAnalysis() {
  cout << "trRec: startAnalysis: ..." << endl;
}

// ----------------------------------------------------------------------
void trRec::endAnalysis() {
  cout << "trRec: endAnalysis: ..." << endl;
}

// ----------------------------------------------------------------------
void trRec::closeHistFile() {
  cout << "==> trRec::closeHistFile() Writing " << fpHistFile->GetName() << endl;
  fpHistFile->cd();
  fpHistFile->Write();
  fpHistFile->Close();
  delete fpHistFile;
}


// ----------------------------------------------------------------------
void trRec::eventProcessing() {
  initVariables();

  if (fVerbose > 9) {
    if (FRAMES == fMode) {
      printFramesBranches();
    } else if (SEGS == fMode) {
      printSegsBranches();
    }
  }

  recStudy();

}


// ----------------------------------------------------------------------
void trRec::recStudy() {
  // -- translation of trGen study. tree variables are not filled, though!
  if (SEGS == fMode) { //ROOT::Math::PxPyPzMVector
    LorentzVector<PxPyPzMVector> p4;
    ((TH1D*)fpHistFile->Get("hproc"))->Fill(fSegsInt["mc_type"]);
    // -- Michel decay electrons
    if ((-11 == fSegsInt["mc_pid"]) && (11 == fSegsInt["mc_type"])) {
      //	p4.SetXYZM(ftraj_px->at(i), ftraj_py->at(i), ftraj_pz->at(i), MMUON);

      double px = fSegsFloat["mc_pt"] * TMath::Cos(fSegsFloat["mc_phi"]);
      double py = fSegsFloat["mc_pt"] * TMath::Sin(fSegsFloat["mc_phi"]);
      double pz = fSegsFloat["mc_pt"] * TMath::Tan(fSegsFloat["mc_lam"]);
      p4  = LorentzVector<PxPyPzMVector>(px, py, pz, MMUON);

      double r = fSegsFloat["mc_pt"];
      ((TH1D*)fpHistFile->Get("hmichel"))->Fill(p4.Rho());
      ((TH2D*)fpHistFile->Get("vrzmichel"))->Fill(fSegsFloat["mc_vz"], r);
      ((TH2D*)fpHistFile->Get("vxymichel"))->Fill(fSegsFloat["mc_vx"], fSegsFloat["mc_vy"]);
    }
    // -- converted photons
    if ((-11 == fSegsInt["mc_pid"]) && (41 == fSegsInt["mc_type"])) {
      double r = fSegsFloat["mc_pt"];
      ((TH1D*)fpHistFile->Get("vrzconv"))->Fill(fSegsFloat["mc_vx"], r);
      ((TH1D*)fpHistFile->Get("vxyconv"))->Fill(fSegsFloat["mc_vx"], fSegsFloat["mc_vy"]);
    }
  }

  // -- Gavin's plots
  if (SEGS == fMode) {
    static int first(1);
    if (1 == first) {
      first = 0;
      new TH1D("hsegAll", "all tracks", 120, -60., 60.);
      new TH1D("hsegFirst", "tracks of first curler turn ", 120, -60., 60.);
      new TH1D("hsegMC", "tracks with matching MC hits", 120, -60., 60.);
      new TH1D("hsegMCprime", "tracks with matching MC hits of first turn", 120, -60., 60.);
    }
    if (4 == fSegsInt["nhit"]) {
      ((TH1D*)fpHistFile->Get("hsegAll"))->Fill(fSegsFloat["p"]);
      if (0 == fSegsInt["seq"]) {
	((TH1D*)fpHistFile->Get("hsegFirst"))->Fill(fSegsFloat["p"]);
      }
      if (1 == fSegsInt["mc"]) {
	((TH1D*)fpHistFile->Get("hsegMC"))->Fill(fSegsFloat["p"]);
      }
      if (1 == fSegsInt["mc_prime"]) {
	((TH1D*)fpHistFile->Get("hsegMCprime"))->Fill(fSegsFloat["p"]);
      }
    }

  }
}



// ----------------------------------------------------------------------
void trRec::fillHist() {

  fTree->Fill();
}

// ----------------------------------------------------------------------
void trRec::bookHist() {
  trBase::bookHist();
  cout << "==> trRec: bookHist> " << endl;

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


  //  fTree->Branch("p",  &fRTD.p,  "p/D");

}


// ----------------------------------------------------------------------
void trRec::initVariables() {
  //  cout << "trBase: initVariables: for run = " << fRun << "/evt = " << fEvt << endl;

}


// ----------------------------------------------------------------------
void trRec::printFramesBranches() {

  cout << "----------------------------------------------------------------------" << endl;
  cout << "frames evt: " << fChainEvent
       << " eventId: " << fEvt
       << " run: "  << fRun
       << " geom_vertex_found: " << fgeom_vertex_found
       << " true_vertex_found: " << ftrue_vertex_found
       << endl;

  for (unsigned int i = 0; i < fmc_p->size(); ++i) {
    cout << Form("mctrk %2d", i)
	 << Form(" pid = %4d", fmc_pid->at(i))
	 << Form(" tid = %+7d", fmc_tid->at(i))
	 << Form(" type = %3d", fmc_type->at(i))
	 << Form(" mid = %7d", fmc_mid->at(i))
	 << Form(" vz = %+9.3f", fmc_vz->at(i))
	 << Form(" vr = %+8.3f", TMath::Sqrt(fmc_vx->at(i)*fmc_vx->at(i) + fmc_vy->at(i)*fmc_vy->at(i)))
	 << " t0 = " << fmc_t0->at(i)
	 << endl;
  }
  cout << "----------------------------------------------------------------------" << endl;


}

// ----------------------------------------------------------------------
void trRec::printSegsBranches() {

  cout << "----------------------------------------------------------------------" << endl;
  cout << "frames evt: " << fChainEvent
       << " eventId: " << fEvt
       << " run: "  << fRun
       << endl;

  cout << "eventId: " << fSegsInt["eventId"]
       << " runId: " << fSegsInt["runId"]
       << " p: " <<  fSegsFloat["p"]
       << " chi2:" << fSegsFloat["chi2"]
       << " n:" << fSegsInt["n"]
    ;
  for (int i = 0; i < fSegsInt["n"]; ++i) {
    //  for (int i = 0; i < SEGSN; ++i) {
    cout << " rt[" << i << "]:" << fSegsFloatV["rt"][i];
    cout << " lam01[" << i << "]:" << fSegsFloatV["lam01"][i];
  }
  cout << endl;
}




// ----------------------------------------------------------------------
trRec::~trRec() {
  cout << "==> trRec: destructor ..." << endl;
  if (!fpChain) return;
  delete fpChain->GetCurrentFile();
}


// ----------------------------------------------------------------------
void trRec::initFrames() {
  cout << "==> trRec::initFrames() ... " << endl;
  // -- NB: The following lines are essential. Else there will be a crash when compiler does not do default zeroing!
  fmc = fmc_prime = fmc_type = 0;
  fmc_pid = fmc_tid = fmc_mid = 0;
  fmc_p = fmc_pt = fmc_phi = fmc_lam = fmc_theta = 0;
  fmc_vx = fmc_vy = fmc_vz = fmc_vr = fmc_vt = fmc_t0 = 0;
  fnhit = fhid0 = fsid0 = 0;
  fx0 = fy0 = fz0 = ft0 = ft0_err = 0;
  fdt = fdt_si = ft0_tl = ft0_fb = ft0_si = 0;
  fr = frerr2 = fp = fperr2 = fchi2 = ftan01 = flam01 = fn_shared_hits = fn_shared_segs = 0;

  initBranch("n", &fn);
  initBranch("n3", &fn3);
  initBranch("n4", &fn4);
  initBranch("n6", &fn6);
  initBranch("n8", &fn8);

  initBranch("geom_vertex_found", &fgeom_vertex_found);
  initBranch("true_vertex_found", &ftrue_vertex_found);

  initBranch("eventId", &feventId);
  initBranch("runId", &frunId);
  initBranch("weight", &fweight);

  initBranch("mc", &fmc);
  initBranch("mc_prime", &fmc_prime);
  initBranch("mc_type", &fmc_type);

  initBranch("mc_pid", &fmc_pid);
  initBranch("mc_tid", &fmc_tid);
  initBranch("mc_mid", &fmc_mid);

  initBranch("mc_p", &fmc_p);
  initBranch("mc_pt", &fmc_pt);
  initBranch("mc_phi", &fmc_phi);
  initBranch("mc_lam", &fmc_lam);
  initBranch("mc_theta", &fmc_theta);

  initBranch("mc_vx", &fmc_vx);
  initBranch("mc_vy", &fmc_vy);
  initBranch("mc_vz", &fmc_vz);
  initBranch("mc_vr", &fmc_vr);
  initBranch("mc_vt", &fmc_vt);
  initBranch("mc_t0", &fmc_t0);

  initBranch("nhit", &fnhit);
  initBranch("hid0", &fhid0);
  initBranch("sid0", &fsid0);

  initBranch("x0", &fx0);
  initBranch("y0", &fy0);
  initBranch("z0", &fz0);
  initBranch("t0", &ft0);
  initBranch("t0_err", &ft0_err);

  initBranch("dt", &fdt);
  initBranch("dt_si", &fdt_si);
  initBranch("t0_tl", &ft0_tl);
  initBranch("t0_fb", &ft0_fb);
  initBranch("t0_si", &ft0_si);

  initBranch("r", &fr);
  initBranch("rerr2", &frerr2);
  initBranch("p", &fp);
  initBranch("perr2", &fperr2);
  initBranch("chi2", &fchi2);
  initBranch("tan01", &ftan01);
  initBranch("lam01", &flam01);
  initBranch("n_shared_hits", &fn_shared_hits);
  initBranch("n_shared_segs", &fn_shared_segs);
}



// ----------------------------------------------------------------------
void trRec::initSegs() {
  cout << "==> trRec::initSegs() ... " << endl;
  float v[SEGSN];
  for (int i = 0 ; i < SEGSN; ++i) v[i] = -99.;

  fSegsInt.insert(make_pair("eventId", 0)); initBranch("eventId", &(fSegsInt["eventId"]));
  fSegsInt.insert(make_pair("runId", 0));   initBranch("runId", &(fSegsInt["runId"]));
  fSegsInt.insert(make_pair("n", 0));       initBranch("n", &(fSegsInt["n"]));
  fSegsInt.insert(make_pair("ndf", 0));     initBranch("ndf", &(fSegsInt["ndf"]));
  fSegsInt.insert(make_pair("nhit", 0));    initBranch("nhit", &(fSegsInt["nhit"]));

  fSegsFloat.insert(make_pair("p", 0)); initBranch("p", &(fSegsFloat["p"]));
  fSegsFloat.insert(make_pair("perr2", 0)); initBranch("perr2", &(fSegsFloat["perr2"]));
  fSegsFloat.insert(make_pair("r", 0)); initBranch("r", &(fSegsFloat["r"]));
  fSegsFloat.insert(make_pair("rerr2", 0)); initBranch("rerr2", &(fSegsFloat["rerr2"]));
  fSegsFloat.insert(make_pair("chi2", 0)); initBranch("chi2", &(fSegsFloat["chi2"]));
  fSegsFloat.insert(make_pair("p_", 0)); initBranch("p_", &(fSegsFloat["p_"]));
  fSegsFloat.insert(make_pair("perr2_", 0)); initBranch("perr2_", &(fSegsFloat["perr2_"]));
  fSegsFloat.insert(make_pair("r_", 0)); initBranch("r_", &(fSegsFloat["r_"]));
  fSegsFloat.insert(make_pair("rerr2_", 0)); initBranch("rerr2_", &(fSegsFloat["rerr2_"]));
  fSegsFloat.insert(make_pair("chi2_", 0)); initBranch("chi2_", &(fSegsFloat["chi2_"]));

  fSegsInt.insert(make_pair("ns", 0));    initBranch("ns", &(fSegsInt["ns"]));
  fSegsInt.insert(make_pair("nh", 0));    initBranch("nh", &(fSegsInt["nh"]));
  fSegsInt.insert(make_pair("nsg", 0));   initBranch("nsg", &(fSegsInt["nsg"]));
  fSegsInt.insert(make_pair("nhg", 0));   initBranch("nhg", &(fSegsInt["nhg"]));
  fSegsInt.insert(make_pair("s3n", 0));   initBranch("s3n", &(fSegsInt["s3n"]));
  fSegsInt.insert(make_pair("s4n", 0));   initBranch("s4n", &(fSegsInt["s4n"]));
  fSegsInt.insert(make_pair("s6n", 0));   initBranch("s6n", &(fSegsInt["s6n"]));
  fSegsInt.insert(make_pair("s8n", 0));   initBranch("s8n", &(fSegsInt["s8n"]));
  fSegsInt.insert(make_pair("s6n_0", 0)); initBranch("s6n_0", &(fSegsInt["s6n_0"]));
  fSegsInt.insert(make_pair("s6n_1", 0)); initBranch("s6n_1", &(fSegsInt["s6n_1"]));
  fSegsInt.insert(make_pair("seq", 0));   initBranch("seq", &(fSegsInt["seq"]));

  fSegsFloat.insert(make_pair("prop_z_dist", 0));   initBranch("prop_z_dist", &(fSegsFloat["prop_z_dist"]));
  fSegsFloat.insert(make_pair("prop_phi_dist", 0)); initBranch("prop_phi_dist", &(fSegsFloat["prop_phi_dist"]));
  fSegsFloat.insert(make_pair("prop_z", 0)); initBranch("prop_z", &(fSegsFloat["prop_z"]));
  fSegsFloat.insert(make_pair("dt", 0)); initBranch("dt", &(fSegsFloat["dt"]));
  fSegsFloat.insert(make_pair("t", 0)); initBranch("t", &(fSegsFloat["t"]));
  fSegsFloat.insert(make_pair("terr", 0)); initBranch("terr", &(fSegsFloat["terr"]));

  fSegsInt.insert(make_pair("mc", 0)); initBranch("mc", &(fSegsInt["mc"]));
  fSegsInt.insert(make_pair("mc_prime", 0)); initBranch("mc_prime", &(fSegsInt["mc_prime"]));
  fSegsInt.insert(make_pair("mc_type", 0)); initBranch("mc_type", &(fSegsInt["mc_type"]));
  fSegsInt.insert(make_pair("mc_pid", 0)); initBranch("mc_pid", &(fSegsInt["mc_pid"]));
  fSegsInt.insert(make_pair("mc_tid", 0)); initBranch("mc_tid", &(fSegsInt["mc_tid"]));
  fSegsInt.insert(make_pair("mc1", 0)); initBranch("mc1", &(fSegsInt["mc1"]));
  fSegsInt.insert(make_pair("mc2", 0)); initBranch("mc2", &(fSegsInt["mc2"]));
  fSegsInt.insert(make_pair("mc_tid1", 0)); initBranch("mc_tid1", &(fSegsInt["mc_tid1"]));
  fSegsInt.insert(make_pair("mc_tid2", 0)); initBranch("mc_tid2", &(fSegsInt["mc_tid2"]));
  fSegsFloat.insert(make_pair("mc_p", 0)); initBranch("mc_p", &(fSegsFloat["mc_p"]));
  fSegsFloat.insert(make_pair("mc_pt", 0)); initBranch("mc_pt", &(fSegsFloat["mc_pt"]));
  fSegsFloat.insert(make_pair("mc_phi", 0)); initBranch("mc_phi", &(fSegsFloat["mc_phi"]));
  fSegsFloat.insert(make_pair("mc_lam", 0)); initBranch("mc_lam", &(fSegsFloat["mc_lam"]));
  fSegsFloat.insert(make_pair("mc_theta", 0)); initBranch("mc_theta", &(fSegsFloat["mc_theta"]));
  fSegsFloat.insert(make_pair("mc_vx", 0)); initBranch("mc_vx", &(fSegsFloat["mc_vx"]));
  fSegsFloat.insert(make_pair("mc_vy", 0)); initBranch("mc_vy", &(fSegsFloat["mc_vy"]));
  fSegsFloat.insert(make_pair("mc_vz", 0)); initBranch("mc_vz", &(fSegsFloat["mc_vz"]));
  fSegsFloat.insert(make_pair("mc_vr", 0)); initBranch("mc_vr", &(fSegsFloat["mc_vr"]));
  fSegsFloat.insert(make_pair("mc_vt", 0)); initBranch("mc_vt", &(fSegsFloat["mc_vt"]));
  fSegsFloat.insert(make_pair("mc_t0", 0)); initBranch("mc_t0", &(fSegsFloat["mc_t0"]));
  fSegsFloat.insert(make_pair("mc_vpca_offset", 0)); initBranch("mc_vpca_offset", &(fSegsFloat["mc_vpca_offset"]));
  fSegsFloat.insert(make_pair("mc_vpca_phi", 0)); initBranch("mc_vpca_phi", &(fSegsFloat["mc_vpca_phi"]));

  fSegsFloat.insert(make_pair("zpca_x", 0)); initBranch("zpca_x", &(fSegsFloat["zpca_x"]));
  fSegsFloat.insert(make_pair("zpca_y", 0)); initBranch("zpca_y", &(fSegsFloat["zpca_y"]));
  fSegsFloat.insert(make_pair("zpca_z", 0)); initBranch("zpca_z", &(fSegsFloat["zpca_z"]));
  fSegsFloat.insert(make_pair("zpca_r", 0)); initBranch("zpca_r", &(fSegsFloat["zpca_r"]));


  fSegsFloatV.insert(make_pair("rt", new float[SEGSN]));  initBranch("rt", fSegsFloatV["rt"]);
  fSegsFloatV.insert(make_pair("rt01", new float[SEGSN]));  initBranch("rt01", fSegsFloatV["rt01"]);
  fSegsFloatV.insert(make_pair("rt12", new float[SEGSN]));  initBranch("rt12", fSegsFloatV["rt12"]);
  fSegsFloatV.insert(make_pair("lam01", new float[SEGSN]));  initBranch("lam01", fSegsFloatV["lam01"]);
  fSegsFloatV.insert(make_pair("lam12", new float[SEGSN]));  initBranch("lam12", fSegsFloatV["lam12"]);
  fSegsFloatV.insert(make_pair("tan01", new float[SEGSN]));  initBranch("tan01", fSegsFloatV["tan01"]);
  fSegsFloatV.insert(make_pair("tan10", new float[SEGSN]));  initBranch("tan10", fSegsFloatV["tan10"]);
  fSegsFloatV.insert(make_pair("tan12", new float[SEGSN]));  initBranch("tan12", fSegsFloatV["tan12"]);
  fSegsFloatV.insert(make_pair("tan21", new float[SEGSN]));  initBranch("tan21", fSegsFloatV["tan21"]);
  fSegsFloatV.insert(make_pair("phims", new float[SEGSN]));  initBranch("phims", fSegsFloatV["phims"]);
  fSegsFloatV.insert(make_pair("lamms", new float[SEGSN]));  initBranch("lamms", fSegsFloatV["lamms"]);

  vector<string> pats;
  pats.push_back("00");
  pats.push_back("01");
  pats.push_back("10");
  pats.push_back("11");
  pats.push_back("20");
  pats.push_back("21");

  for (unsigned int i = 0; i < pats.size(); ++i) {
    fSegsFloatV.insert(make_pair("x" + pats[i], new float[SEGSN]));    initBranch("x" + pats[i], fSegsFloatV["x" + pats[i]]);
    fSegsFloatV.insert(make_pair("y" + pats[i], new float[SEGSN]));    initBranch("y" + pats[i], fSegsFloatV["y" + pats[i]]);
    fSegsFloatV.insert(make_pair("z" + pats[i], new float[SEGSN]));    initBranch("z" + pats[i], fSegsFloatV["z" + pats[i]]);
    fSegsFloatV.insert(make_pair("hid" + pats[i], new float[SEGSN]));  initBranch("hid" + pats[i], fSegsFloatV["hid" + pats[i]]);
    fSegsFloatV.insert(make_pair("s" + pats[i], new float[SEGSN]));    initBranch("s" + pats[i], fSegsFloatV["s" + pats[i]]);
  }

  fSegsFloatV.insert(make_pair("rms2_mcs", new float[SEGSN])); initBranch("rms2_mcs", fSegsFloatV["rms2_mcs"]);
  fSegsFloatV.insert(make_pair("sigma_phi", new float[SEGSN])); initBranch("sigma_phi", fSegsFloatV["sigma_phi"]);
  fSegsFloatV.insert(make_pair("sigma_lam", new float[SEGSN])); initBranch("sigma_lam", fSegsFloatV["sigma_lam"]);
  fSegsFloatV.insert(make_pair("edep", new float[SEGSN])); initBranch("edep", fSegsFloatV["edep"]);
  fSegsFloatV.insert(make_pair("phi01x", new float[SEGSN])); initBranch("phi01x", fSegsFloatV["phi01x"]);
  fSegsFloatV.insert(make_pair("phi12x", new float[SEGSN])); initBranch("phi12x", fSegsFloatV["phi12x"]);
  fSegsFloatV.insert(make_pair("d01", new float[SEGSN])); initBranch("d01", fSegsFloatV["d01"]);
  fSegsFloatV.insert(make_pair("d12", new float[SEGSN])); initBranch("d12", fSegsFloatV["d12"]);
  fSegsFloatV.insert(make_pair("r01c", new float[SEGSN])); initBranch("r01c", fSegsFloatV["r01c"]);
  fSegsFloatV.insert(make_pair("r12c", new float[SEGSN])); initBranch("r12c", fSegsFloatV["r12c"]);
  fSegsFloatV.insert(make_pair("phi01c", new float[SEGSN])); initBranch("phi01c", fSegsFloatV["phi01c"]);
  fSegsFloatV.insert(make_pair("phi12c", new float[SEGSN])); initBranch("phi12c", fSegsFloatV["phi12c"]);
  fSegsFloatV.insert(make_pair("phi01", new float[SEGSN])); initBranch("phi01", fSegsFloatV["phi01"]);
  fSegsFloatV.insert(make_pair("phi12", new float[SEGSN])); initBranch("phi12", fSegsFloatV["phi12"]);
  fSegsFloatV.insert(make_pair("alpha01", new float[SEGSN])); initBranch("alpha01", fSegsFloatV["alpha01"]);
  fSegsFloatV.insert(make_pair("alpha12", new float[SEGSN])); initBranch("alpha12", fSegsFloatV["alpha12"]);
  fSegsFloatV.insert(make_pair("lam01c", new float[SEGSN])); initBranch("lam01c", fSegsFloatV["lam01c"]);
  fSegsFloatV.insert(make_pair("lam12c", new float[SEGSN])); initBranch("lam12c", fSegsFloatV["lam12c"]);
  fSegsFloatV.insert(make_pair("beta01", new float[SEGSN])); initBranch("beta01", fSegsFloatV["beta01"]);
  fSegsFloatV.insert(make_pair("beta12", new float[SEGSN])); initBranch("beta12", fSegsFloatV["beta12"]);
  fSegsFloatV.insert(make_pair("ds", new float[SEGSN])); initBranch("ds", fSegsFloatV["ds"]);
  fSegsFloatV.insert(make_pair("dot0", new float[SEGSN])); initBranch("dot0", fSegsFloatV["dot0"]);
  fSegsFloatV.insert(make_pair("dot1", new float[SEGSN])); initBranch("dot1", fSegsFloatV["dot1"]);
  fSegsFloatV.insert(make_pair("dot2", new float[SEGSN])); initBranch("dot2", fSegsFloatV["dot2"]);
  fSegsFloatV.insert(make_pair("delta_rz", new float[SEGSN])); initBranch("delta_rz", fSegsFloatV["delta_rz"]);
  fSegsFloatV.insert(make_pair("sid00", new float[SEGSN])); initBranch("sid00", fSegsFloatV["sid00"]);
  fSegsFloatV.insert(make_pair("row00", new float[SEGSN])); initBranch("row00", fSegsFloatV["row00"]);
  fSegsFloatV.insert(make_pair("col00", new float[SEGSN])); initBranch("col00", fSegsFloatV["col00"]);
  fSegsFloatV.insert(make_pair("ein00", new float[SEGSN])); initBranch("ein00", fSegsFloatV["ein00"]);
  fSegsFloatV.insert(make_pair("eout00", new float[SEGSN])); initBranch("eout00", fSegsFloatV["eout00"]);
  fSegsFloatV.insert(make_pair("sid01", new float[SEGSN])); initBranch("sid01", fSegsFloatV["sid01"]);
  fSegsFloatV.insert(make_pair("row01", new float[SEGSN])); initBranch("row01", fSegsFloatV["row01"]);
  fSegsFloatV.insert(make_pair("col01", new float[SEGSN])); initBranch("col01", fSegsFloatV["col01"]);
  fSegsFloatV.insert(make_pair("ein01", new float[SEGSN])); initBranch("ein01", fSegsFloatV["ein01"]);
  fSegsFloatV.insert(make_pair("eout01", new float[SEGSN])); initBranch("eout01", fSegsFloatV["eout01"]);
  fSegsFloatV.insert(make_pair("sid10", new float[SEGSN])); initBranch("sid10", fSegsFloatV["sid10"]);
  fSegsFloatV.insert(make_pair("row10", new float[SEGSN])); initBranch("row10", fSegsFloatV["row10"]);
  fSegsFloatV.insert(make_pair("col10", new float[SEGSN])); initBranch("col10", fSegsFloatV["col10"]);
  fSegsFloatV.insert(make_pair("ein10", new float[SEGSN])); initBranch("ein10", fSegsFloatV["ein10"]);
  fSegsFloatV.insert(make_pair("eout10", new float[SEGSN])); initBranch("eout10", fSegsFloatV["eout10"]);
  fSegsFloatV.insert(make_pair("sid11", new float[SEGSN])); initBranch("sid11", fSegsFloatV["sid11"]);
  fSegsFloatV.insert(make_pair("row11", new float[SEGSN])); initBranch("row11", fSegsFloatV["row11"]);
  fSegsFloatV.insert(make_pair("col11", new float[SEGSN])); initBranch("col11", fSegsFloatV["col11"]);
  fSegsFloatV.insert(make_pair("ein11", new float[SEGSN])); initBranch("ein11", fSegsFloatV["ein11"]);
  fSegsFloatV.insert(make_pair("eout11", new float[SEGSN])); initBranch("eout11", fSegsFloatV["eout11"]);
  fSegsFloatV.insert(make_pair("sid20", new float[SEGSN])); initBranch("sid20", fSegsFloatV["sid20"]);
  fSegsFloatV.insert(make_pair("row20", new float[SEGSN])); initBranch("row20", fSegsFloatV["row20"]);
  fSegsFloatV.insert(make_pair("col20", new float[SEGSN])); initBranch("col20", fSegsFloatV["col20"]);
  fSegsFloatV.insert(make_pair("ein20", new float[SEGSN])); initBranch("ein20", fSegsFloatV["ein20"]);
  fSegsFloatV.insert(make_pair("eout20", new float[SEGSN])); initBranch("eout20", fSegsFloatV["eout20"]);
  fSegsFloatV.insert(make_pair("sid21", new float[SEGSN])); initBranch("sid21", fSegsFloatV["sid21"]);
  fSegsFloatV.insert(make_pair("row21", new float[SEGSN])); initBranch("row21", fSegsFloatV["row21"]);
  fSegsFloatV.insert(make_pair("col21", new float[SEGSN])); initBranch("col21", fSegsFloatV["col21"]);
  fSegsFloatV.insert(make_pair("ein21", new float[SEGSN])); initBranch("ein21", fSegsFloatV["ein21"]);
  fSegsFloatV.insert(make_pair("eout21", new float[SEGSN])); initBranch("eout21", fSegsFloatV["eout21"]);
  fSegsFloatV.insert(make_pair("dr", new float[SEGSN])); initBranch("dr", fSegsFloatV["dr"]);

}



// --------------------------------------------------------------------------------------------------
void trRec::readCuts(string filename, int dump) {
  char  buffer[200];
  fCutFile = filename;
  if (dump) cout << "==> trRec: Reading " << fCutFile << " for cut settings" << endl;
  sprintf(buffer, "%s", fCutFile.c_str());
  ifstream is(buffer);
  char CutName[100];
  float CutValue;
  int ok(0);

  string fn(fCutFile);

  if (dump) {
    cout << "====================================" << endl;
    cout << "==> trRec: Cut file  " << fCutFile << endl;
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
