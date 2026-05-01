#include "fillHist.hh"

#include <iostream> 
#include <string>
#include <regex>

#include <TMath.h>
#include <TObjString.h>
#include <TH2D.h>

#include "../../../common/json.h" // nlohmann

using namespace std;


// ----------------------------------------------------------------------
fillHist::fillHist(const std::string &infile, const std::string &outfileName) {
  cout << "fillHist::fillHist() infile = " << infile << " outfileName = " << outfileName << endl;
  fInFile = TFile::Open(infile.c_str());
  if (!fInFile) {
    cout << "fillHist::fillHist() ERROR: could not open input file " << infile.c_str() << endl;
    return;
  }

  // -- get config from input file
  string config = ((TObjString*)(fInFile->Get("config_all")))->GetString().Data();
  nlohmann::json config_json = nlohmann::json::parse(config);

  // -- extract CDB settings from top-level config JSON
  string cdbDbconn = "";
  string cdbGlobalTag = "";
  if (config_json.contains("cdb") && config_json["cdb"].is_object()) {
    const auto &cdb = config_json["cdb"];
    if (cdb.contains("dbconn") && cdb["dbconn"].is_string()) {
      cdbDbconn = cdb["dbconn"].get<string>();
    }
    if (cdb.contains("globalTag") && cdb["globalTag"].is_string()) {
      cdbGlobalTag = cdb["globalTag"].get<string>();
    }
  }
  fConfigs["cdb_dbconn"] = cdbDbconn;
  fConfigs["cdb_globalTag"] = cdbGlobalTag;

  for (size_t i = 0; i < config_json["cmds"].size(); ++i) {
    string s = config_json["cmds"][i].get<string>();

    // Extract --conf value from command strings like:
    //   --conf trirec.conf
    //   --conf=trirec.conf
    string confValue;
    static const regex confPattern(R"((?:^|\s)--conf(?:=|\s+)(\"[^\"]+\"|'[^']+'|[^\s]+))");
    smatch match;
    if (regex_search(s, match, confPattern) && match.size() > 1) {
      confValue = match[1].str();
      if (confValue.size() >= 2) {
        const char first = confValue.front();
        const char last = confValue.back();
        if ((first == '"' && last == '"') || (first == '\'' && last == '\'')) {
          confValue = confValue.substr(1, confValue.size() - 2);
        }
      }
    }

    if (!confValue.empty()) {
      if (string::npos != s.find("mu3eSim")) {
        fConfigs["sim_conf"] = confValue;
      } else if (string::npos != s.find("mu3eSort")) {
        fConfigs["sort_conf"] = confValue;
      } else if (string::npos != s.find("mu3eTriRec")) {
        fConfigs["trirec_conf"] = confValue;
      } 
    }

    for (size_t i = 0; i < config_json["versions"].size(); ++i) {
      string s = config_json["versions"][i].get<string>();
      if (0 == i) fConfigs["sim_version"] = s;
      else if (1 == i) fConfigs["sort_version"] = s;
      else if (2 == i) fConfigs["trirec_version"] = s;
    }
  }

  cout << "fillHist::fillHist() fConfigs = " << fConfigs.size() << endl;
  for (auto &c : fConfigs) {
    cout << "   " << c.first << ": " << c.second << endl;
  }

  fOutFileName = outfileName;
  fOutFile = TFile::Open(fOutFileName.c_str(), "RECREATE");
  if (!fOutFile) {
    cout << "fillHist::fillHist() ERROR: could not open output file " << fOutFileName.c_str() << endl;
    return;
  }
}


// ----------------------------------------------------------------------
fillHist::~fillHist() {
  if (fOutFile) {
    fOutFile->cd();
    for (auto &h : fHistograms) {
      h.second->Write();
      delete h.second;
    }
    fOutFile->Close();
    delete fOutFile;
  }
  if (fInFile) {
    fInFile->Close();
    delete fInFile;
  }
}


// ----------------------------------------------------------------------
void fillHist::bookHist(string mode, string annotation) {
  if ("relval" == mode) {
    // -- bookkeeping
    fHistograms["hinfo"] = new TH1D("hinfo", "info", 50, 0., 50.);
    fHistograms["hinfo"]->GetXaxis()->SetBinLabel(1, annotation.c_str());
    fHistograms["hinfo"]->GetXaxis()->SetBinLabel(2, "n_events");
    fHistograms["hinfo"]->GetXaxis()->SetBinLabel(3, "n_hi_tracks");

    fHistograms["hinfo"]->GetXaxis()->SetBinLabel(10, fConfigs["sim_conf"].c_str());
    fHistograms["hinfo"]->GetXaxis()->SetBinLabel(11, fConfigs["sim_version"].c_str());
    fHistograms["hinfo"]->GetXaxis()->SetBinLabel(20, fConfigs["sort_conf"].c_str());
    fHistograms["hinfo"]->GetXaxis()->SetBinLabel(21, fConfigs["sort_version"].c_str());
    fHistograms["hinfo"]->GetXaxis()->SetBinLabel(30, fConfigs["trirec_conf"].c_str());
    fHistograms["hinfo"]->GetXaxis()->SetBinLabel(31, fConfigs["trirec_version"].c_str());
    fHistograms["hinfo"]->GetXaxis()->SetBinLabel(35, fConfigs["cdb_dbconn"].c_str());
    fHistograms["hinfo"]->GetXaxis()->SetBinLabel(36, fConfigs["cdb_globalTag"].c_str());

    fHistograms["hpall"] = new TH1D("hpall", "p (all tracks)", 100, -100., 100.);
    fHistograms["hp"] = new TH1D("hp", "p", 100, -100., 100.);
    fHistograms["hperr2"] = new TH1D("hperr2", "perr2", 100, 0., 10.);
    fHistograms["hx0"] = new TH1D("hx0", "x0", 160, -80., 80.);
    fHistograms["hy0"] = new TH1D("hy0", "y0", 160, -80., 80.);
    fHistograms["hz0"] = new TH1D("hz0", "z0", 100, -100., 100.);
    fHistograms["ht0"] = new TH1D("ht0", "t0", 100, -100., 100.);
    fHistograms["ht0_err"] = new TH1D("ht0_err", "t0_err", 100, -100., 100.);
    fHistograms["ht0_rms"] = new TH1D("ht0_rms", "t0_rms", 100, -100., 100.);
    fHistograms["ht0_tl"] = new TH1D("ht0_tl", "t0_tl", 100, -100., 100.);
    fHistograms["ht0_fb"] = new TH1D("ht0_fb", "t0_fb", 100, -100., 100.);
    fHistograms["ht0_si"] = new TH1D("ht0_si", "t0_si", 100, -100., 100.);
    fHistograms["ht0_tl_rms"] = new TH1D("ht0_tl_rms", "t0_tl_rms", 100, 0., 100.);
    fHistograms["ht0_fb_rms"] = new TH1D("ht0_fb_rms", "t0_fb_rms", 100, 0., 100.);
    fHistograms["ht0_si_rms"] = new TH1D("ht0_si_rms", "t0_si_rms", 100, 0., 100.);
    fHistograms["hr"] = new TH1D("hr", "r", 100, -250., 250.);
    fHistograms["hrerr2"] = new TH1D("hrerr2", "rerr2", 100, 0., 100.);
    fHistograms["hchi2"] = new TH1D("hchi2", "chi2", 100, 0., 100.);
    fHistograms["htan01"] = new TH1D("htan01", "tan01", 63, -3.15, 3.15);
    fHistograms["hlam01"] = new TH1D("hlam01", "lam01", 100, -1., 1.);
    fHistograms["hnhit"] = new TH1D("hnhit", "nhit", 100, 0., 20.);
    fHistograms["httype"] = new TH1D("httype", "ttype", 1000, -500., 500.);
    fHistograms["hn_shared_hits"] = new TH1D("hn_shared_hits", "n_shared_hits", 50, 0., 50.);
    fHistograms["hn_shared_segs"] = new TH1D("hn_shared_segs", "n_shared_segs", 50, 0., 50.);
    fHistograms["hfarm_status"] = new TH1D("hfarm_status", "farm_status", 1000, -500., 500.);
    fHistograms["hsid0"] = new TH1D("hsid0", "sid0", 20000, 0., 20000.);
    fHistograms["hn"] = new TH1D("hn", "n", 100, 0., 100.);
    fHistograms["hn4"] = new TH1D("hn4", "n4", 100, 0., 100.);
    fHistograms["hn6"] = new TH1D("hn6", "n6", 100, 0., 100.);
    fHistograms["hn8"] = new TH1D("hn8", "n8", 100, 0., 100.);

    // -- hitmaps derived from hit positions
    fHistograms["h2hitmapvtx0"] = new TH2D("h2hitmapvtx0", "hitmap vtx0", 24, -62., 62., 8, 0., 8.);

    // -- hitmaps derived from hit positions for each vtx0 sensor
    for (int i = 0; i < 231; ++i) {      
      int ok = getVtxL0Ladder(i);
      if (ok >= 0) {
        string hname = Form("h2hitmapvtx0_sid%03d", i);
        fHistograms[hname] = new TH2D(hname.c_str(), hname.c_str(), 48, -62., 62., 80, -3.15, 3.15);
      }
    }
  }
}


// ----------------------------------------------------------------------
void fillHist::run(int nevents) {
  fNevents = nevents;
  if (fNevents < 0) fNevents = fTree->GetEntries();
  if (fInFile) fInFile->cd();
  cout << "fillHist::run() fNevents = " << fNevents << endl;
  fHistograms["hinfo"]->SetBinContent(2, fNevents);
  int nHiTracks(0); // number of hits > 20 GeV
  for (int i = 0; i < fNevents; ++i) {
    fTree->GetEntry(i);
    //cout << "fillHist::run() i = " << i << " fp->size() = " << fp->size() << endl;
    fHistograms["hn"]->Fill(fn);
    fHistograms["hn4"]->Fill(fn4);
    fHistograms["hn6"]->Fill(fn6);
    fHistograms["hn8"]->Fill(fn8);

    // -- check that the number of tracks is the same as the number of hits
    if (!checkVectorSizes())  continue;
    
    // -- fill per-track histograms
    for (unsigned int j = 0; j < fp->size(); ++j) {
      fHistograms["hpall"]->Fill(fp->at(j));
  
      if (TMath::Abs(fp->at(j)) > 20.) {
        nHiTracks++;
        fHistograms["hx0"]->Fill(fx0->at(j));
        fHistograms["hy0"]->Fill(fy0->at(j));
        fHistograms["hz0"]->Fill(fz0->at(j));
        fHistograms["ht0"]->Fill(ft0->at(j));
        fHistograms["ht0_err"]->Fill(ft0_err->at(j));
        fHistograms["ht0_rms"]->Fill(ft0_rms->at(j));
        fHistograms["ht0_tl"]->Fill(ft0_tl->at(j));
        fHistograms["ht0_fb"]->Fill(ft0_fb->at(j));
        fHistograms["ht0_si"]->Fill(ft0_si->at(j));
        // -- check required for v6.3.3
        if (ft0_tl_rms) fHistograms["ht0_tl_rms"]->Fill(ft0_tl_rms->at(j));
        if (ft0_fb_rms) fHistograms["ht0_fb_rms"]->Fill(ft0_fb_rms->at(j));
        if (ft0_si_rms) fHistograms["ht0_si_rms"]->Fill(ft0_si_rms->at(j));
        fHistograms["hr"]->Fill(fr->at(j));
        fHistograms["hrerr2"]->Fill(frerr2->at(j));
        fHistograms["hp"]->Fill(fp->at(j));
        fHistograms["hperr2"]->Fill(fperr2->at(j));
        fHistograms["hchi2"]->Fill(fchi2->at(j));
        fHistograms["htan01"]->Fill(ftan01->at(j));
        fHistograms["hlam01"]->Fill(flam01->at(j));
        fHistograms["hnhit"]->Fill(fnhit->at(j));
        fHistograms["httype"]->Fill(fttype->at(j));
        fHistograms["hn_shared_hits"]->Fill(fn_shared_hits->at(j));
        fHistograms["hn_shared_segs"]->Fill(fn_shared_segs->at(j));
        fHistograms["hfarm_status"]->Fill(ffarm_status->at(j));

        // -- fill hitmap vtx0
        int vtxL0Ladder = getVtxL0Ladder(fsid0->at(j));
        if (vtxL0Ladder >= 0) {
          fHistograms["h2hitmapvtx0"]->Fill(fz0->at(j), vtxL0Ladder);
          fHistograms[Form("h2hitmapvtx0_sid%03d", fsid0->at(j))]->Fill(fz0->at(j), TMath::ATan2(fy0->at(j), fx0->at(j)));
        }
      }
    }
  }
  fHistograms["hinfo"]->SetBinContent(3, nHiTracks);
}


// ----------------------------------------------------------------------
int fillHist::getVtxL0Ladder(int sid0) {
  int vtxL0Ladder(-1);
  if (sid0 >= 1 && sid0 <= 6) {
    vtxL0Ladder = 0;
  } else if (sid0 >= 33 && sid0 <= 38) {
    vtxL0Ladder = 1;
  } else if (sid0 >= 65 && sid0 <= 70) {
    vtxL0Ladder = 2;
  } else if (sid0 >= 97 && sid0 <= 102) {
    vtxL0Ladder = 3;
  } else if (sid0 >= 129 && sid0 <= 134) {
    vtxL0Ladder = 4;
  } else if (sid0 >= 161 && sid0 <= 166) {
    vtxL0Ladder = 5;
  } else if (sid0 >= 193 && sid0 <= 198) {
    vtxL0Ladder = 6;
  } else if (sid0 >= 225 && sid0 <= 230) {
    vtxL0Ladder = 7;
  }
  return vtxL0Ladder;
}


// ----------------------------------------------------------------------
bool fillHist::checkVectorSizes() {
  if (fp->size() != fx0->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: fp->size() != fx0->size() " << fp->size() << " != " << fx0->size() << endl;
    return false;
  }
  if (fp->size() != fy0->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: fp->size() != fy0->size() " << fp->size() << " != " << fy0->size() << endl;
    return false;
  }
  if (fp->size() != fz0->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: fp->size() != fz0->size() " << fp->size() << " != " << fz0->size() << endl;
    return false;
  }
  if (fp->size() != ft0->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: fp->size() != ft0->size() " << fp->size() << " != " << ft0->size() << endl;
    return false;
  }
  if (fp->size() != ft0_err->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: fp->size() != ft0_err->size() " << fp->size() << " != " << ft0_err->size() << endl;
    return false;
  }
  if (fp->size() != ft0_tl->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: fp->size() != ft0_tl->size() " << fp->size() << " != " << ft0_tl->size() << endl;
    return false;
  }
  if (fp->size() != ft0_fb->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: fp->size() != ft0_fb->size() " << fp->size() << " != " << ft0_fb->size() << endl;
    return false;
  }
  if (fp->size() != ft0_si->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: fp->size() != ft0_si->size() " << fp->size() << " != " << ft0_si->size() << endl;
    return false;
  }
  if (ft0_tl_rms && fp->size() != ft0_tl_rms->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: fp->size() != ft0_tl_rms->size() " << fp->size() << " != " << ft0_tl_rms->size() << endl;
    return false;
  }
  if (ft0_fb_rms && fp->size() != ft0_fb_rms->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: fp->size() != ft0_fb_rms->size() " << fp->size() << " != " << ft0_fb_rms->size() << endl;
    return false;
  }
  if (ft0_si_rms && fp->size() != ft0_si_rms->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: fp->size() != ft0_si_rms->size() " << fp->size() << " != " << ft0_si_rms->size() << endl;
    return false;
  }
  if (fp->size() != fr->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: fp->size() != fr->size() " << fp->size() << " != " << fr->size() << endl;
    return false;
  }
  if (fp->size() != frerr2->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: fp->size() != frerr2->size() " << fp->size() << " != " << frerr2->size() << endl;
    return false;
  }
  if (fp->size() != fperr2->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: fp->size() != fperr2->size() " << fp->size() << " != " << fperr2->size() << endl;
    return false;
  }
  if (fp->size() != fchi2->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: fp->size() != fchi2->size() " << fp->size() << " != " << fchi2->size() << endl;
    return false;
  }
  if (fp->size() != ftan01->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: fp->size() != ftan01->size() " << fp->size() << " != " << ftan01->size() << endl;
    return false;
  }
  if (fp->size() != flam01->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: fp->size() != flam01->size() " << fp->size() << " != " << flam01->size() << endl;
    return false;
  }
  if (fp->size() != fsid0->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: fp->size() != fsid0->size() " << fp->size() << " != " << fsid0->size() << endl;
    return false;
  }
  if (fp->size() != fnhit->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: fp->size() != fnhit->size() " << fp->size() << " != " << fnhit->size() << endl;
    return false;
  }
  if (fp->size() != fttype->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: fp->size() != fttype->size() " << fp->size() << " != " << fttype->size() << endl;
    return false;
  }
  if (fp->size() != fn_shared_hits->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: fp->size() != fn_shared_hits->size() " << fp->size() << " != " << fn_shared_hits->size() << endl;
    return false;
  }
  if (fp->size() != fn_shared_segs->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: fp->size() != fn_shared_segs->size() " << fp->size() << " != " << fn_shared_segs->size() << endl;
    return false;
  }
  if (fp->size() != ffarm_status->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: fp->size() != ffarm_status->size() " << fp->size() << " != " << ffarm_status->size() << endl;
    return false;
  }
  if (fp->size() != fsid0->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: fp->size() != fsid0->size() " << fp->size() << " != " << fsid0->size() << endl;
    return false;
  }
  return true;
}

// ----------------------------------------------------------------------
void fillHist::setupTree(const std::string &treeName) {
  fTree = fInFile->Get<TTree>(treeName.c_str());
  if (!fTree) {
    cout << "fillHist::setupTree() ERROR: could not get tree " << treeName << endl;
    return;
  }

  // Read only branches we explicitly bind below.
  fTree->SetBranchStatus("*", 0);
  
  // -- NB: The following lines are essential. Else there will be a crash when compiler does not do default zeroing!
  fx0 = fy0 = fz0 = ft0 = ft0_err = ft0_rms = ft0_tl = ft0_fb = ft0_si = 0;
  ft0_tl_rms = ft0_fb_rms = ft0_si_rms = 0;
  fr = frerr2 = fp = fperr2 = fchi2 = ftan01 = flam01 = 0;
  fnhit = fttype = fn_shared_hits = fn_shared_segs = ffarm_status = 0;
  fsid0 = 0;
  fn = fn4 = fn6 = fn8 = 0;
  fmc = fmc_prime = fmc_type = 0;
  fmc_pid = fmc_tid = fmc_mid = 0;
  fmc_weight = fmc_p = fmc_pt = fmc_phi = fmc_lam = fmc_theta = 0;
  fmc_vx = fmc_vy = fmc_vz = fmc_vr = fmc_vt = fmc_t0 = 0;
  
  initBranch("x0", &fx0);
  initBranch("y0", &fy0);
  initBranch("z0", &fz0);
  initBranch("t0", &ft0);
  initBranch("t0_err", &ft0_err);
  initBranch("t0_rms", &ft0_rms);
  initBranch("t0_tl", &ft0_tl);
  initBranch("t0_fb", &ft0_fb);
  initBranch("t0_si", &ft0_si);
  initBranch("t0_tl_rms", &ft0_tl_rms);
  initBranch("t0_fb_rms", &ft0_fb_rms);
  initBranch("t0_si_rms", &ft0_si_rms);
  initBranch("r", &fr);
  initBranch("rerr2", &frerr2);
  initBranch("p", &fp);
  initBranch("perr2", &fperr2);
  initBranch("chi2", &fchi2);
  initBranch("tan01", &ftan01);
  initBranch("lam01", &flam01);
  initBranch("nhit", &fnhit);
  initBranch("ttype", &fttype);
  initBranch("n_shared_hits", &fn_shared_hits);
  initBranch("n_shared_segs", &fn_shared_segs);
  initBranch("farm_status", &ffarm_status);
  initBranch("sid0", &fsid0);
  initBranch("n", &fn);
  initBranch("n4", &fn4);
  initBranch("n6", &fn6);
  initBranch("n8", &fn8);
}


// ----------------------------------------------------------------------
void fillHist::initBranch(string name, int* pvar) {
  if (fTree->GetBranch(name.c_str()) == nullptr) {
    cout << "fillHist::initBranch() ERROR: branch " << name << " not found in tree" << endl;
    return;
  }
  fTree->SetBranchStatus(name.c_str(), 1);
  fTree->SetBranchAddress(name.c_str(), pvar);
}

// ----------------------------------------------------------------------
void fillHist::initBranch(string name, float* pvar) {
  if (fTree->GetBranch(name.c_str()) == nullptr) {
    cout << "fillHist::initBranch() ERROR: branch " << name << " not found in tree" << endl;
    return;
  }
  fTree->SetBranchStatus(name.c_str(), 1);
  fTree->SetBranchAddress(name.c_str(), pvar);
}

// ----------------------------------------------------------------------
void fillHist::initBranch(string name, double* pvar) {
  if (fTree->GetBranch(name.c_str()) == nullptr) {
    cout << "fillHist::initBranch() ERROR: branch " << name << " not found in tree" << endl;
    return;
  }
  fTree->SetBranchStatus(name.c_str(), 1);
  fTree->SetBranchAddress(name.c_str(), pvar);
}

// ----------------------------------------------------------------------
void fillHist::initBranch(string name, string** pvar) {
  cout << "initBranch(" << name << "),  pvar = " << pvar << endl;
  if (fTree->GetBranch(name.c_str()) == nullptr) {
    cout << "fillHist::initBranch() ERROR: branch " << name << " not found in tree" << endl;
    return;
  }
  fTree->SetBranchStatus(name.c_str(), 1);
  fTree->SetBranchAddress(name.c_str(), pvar);
}

// ----------------------------------------------------------------------
void fillHist::initBranch(string name, vector<int>** pvect) {
  if (fTree->GetBranch(name.c_str()) == nullptr) {
    cout << "fillHist::initBranch() ERROR: branch " << name << " not found in tree" << endl;
    return;
  }
  fTree->SetBranchStatus(name.c_str(), 1);
  fTree->SetBranchAddress(name.c_str(), pvect);
}
// ----------------------------------------------------------------------
void fillHist::initBranch(string name, vector<unsigned int>** pvect) {
  if (fTree->GetBranch(name.c_str()) == nullptr) {
    cout << "fillHist::initBranch() ERROR: branch " << name << " not found in tree" << endl;
    return;
  }
  fTree->SetBranchStatus(name.c_str(), 1);
  fTree->SetBranchAddress(name.c_str(), pvect);
}

// ----------------------------------------------------------------------
void fillHist::initBranch(string name, vector<double>** pvect) {
  if (fTree->GetBranch(name.c_str()) == nullptr) {
    cout << "fillHist::initBranch() ERROR: branch " << name << " not found in tree" << endl;
    return;
  }
  fTree->SetBranchStatus(name.c_str(), 1);
  fTree->SetBranchAddress(name.c_str(), pvect);
}
