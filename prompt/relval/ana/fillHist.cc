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
  fFrames.name = "frames";
  fFrames.tree = nullptr;
  resetBranches(fFrames);
  fMcFrames.name = "mc_frames";
  fMcFrames.tree = nullptr;
  resetBranches(fMcFrames);
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
    
    fHistograms["hpall"] = new TH1D("hpall", "p (all tracks)", 40, 0., 80.);
    fHistograms["hp"] = new TH1D("hp", "p", 40, 0., 80.);
    fHistograms["hperr2"] = new TH1D("hperr2", "perr2", 50, 0., 2.);
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
    
    // -- resolution plots
    
    // -- efficiency plots
    fHistograms["h2lamvsprec"] = new TH2D("h2lamvsprec", "lam vs p", 30, 0., 60., 30, -1.5, 1.5);
    fHistograms["h2lamvspsim"] = new TH2D("h2lamvspsim", "lam vs p", 30, 0., 60., 30, -1.5, 1.5);
    fHistograms["h2lamvspeff"] = new TH2D("h2lamvspeff", "lam vs p", 30, 0., 60., 30, -1.5, 1.5);

    fHistograms["h1lamvspreceff"] = new TH1D("h1lamvspreceff", "efficiency projection (lam vs p)", 51, 0., 1.02);
  }
}


// ----------------------------------------------------------------------
void fillHist::run() {
  if (!fFrames.tree) {
    cout << "fillHist::run() ERROR: frames tree is not initialized" << endl;
    return;
  }
  if (fInFile) fInFile->cd();
  cout << "fillHist::run() nEvents = " << fFrames.nEvents << endl;
  fHistograms["hinfo"]->SetBinContent(2, fFrames.nEvents);
  int nHiTracks(0); // number of hits > 20 GeV
  
  // -- loop over frames tree
  for (int i = 0; i < fFrames.nEvents; ++i) {
    fFrames.tree->GetEntry(i);
    if (fMcFrames.tree) fMcFrames.tree->GetEntry(i);
    //cout << "fillHist::run() i = " << i << " fp->size() = " << fp->size() << endl;
    fHistograms["hn"]->Fill(fFrames.n);
    fHistograms["hn4"]->Fill(fFrames.n4);
    fHistograms["hn6"]->Fill(fFrames.n6);
    fHistograms["hn8"]->Fill(fFrames.n8);
    
    // -- check that the number of tracks is the same as the number of hits
    if (!checkVectorSizes(fFrames))  continue;
    
    // -- fill per-track histograms
    for (unsigned int j = 0; j < fFrames.p->size(); ++j) {
      fHistograms["hpall"]->Fill(TMath::Abs(fFrames.p->at(j)));
      
      // if (TMath::Abs(fFrames.p->at(j)) > 20.) {
      if (fFrames.goodReconstructedTrack(j, 42)) {
        nHiTracks++;
        fHistograms["hx0"]->Fill(fFrames.x0->at(j));
        fHistograms["hy0"]->Fill(fFrames.y0->at(j));
        fHistograms["hz0"]->Fill(fFrames.z0->at(j));
        fHistograms["ht0"]->Fill(fFrames.t0->at(j));
        fHistograms["ht0_err"]->Fill(fFrames.t0_err->at(j));
        fHistograms["ht0_rms"]->Fill(fFrames.t0_rms->at(j));
        fHistograms["ht0_tl"]->Fill(fFrames.t0_tl->at(j));
        fHistograms["ht0_fb"]->Fill(fFrames.t0_fb->at(j));
        fHistograms["ht0_si"]->Fill(fFrames.t0_si->at(j));
        // -- check required for v6.3.3
        if (fFrames.t0_tl_rms) fHistograms["ht0_tl_rms"]->Fill(fFrames.t0_tl_rms->at(j));
        if (fFrames.t0_fb_rms) fHistograms["ht0_fb_rms"]->Fill(fFrames.t0_fb_rms->at(j));
        if (fFrames.t0_si_rms) fHistograms["ht0_si_rms"]->Fill(fFrames.t0_si_rms->at(j));
        fHistograms["hr"]->Fill(fFrames.r->at(j));
        fHistograms["hrerr2"]->Fill(fFrames.rerr2->at(j));
        fHistograms["hp"]->Fill(TMath::Abs(fFrames.p->at(j)));
        fHistograms["hperr2"]->Fill(fFrames.perr2->at(j));
        fHistograms["hchi2"]->Fill(fFrames.chi2->at(j));
        fHistograms["htan01"]->Fill(fFrames.tan01->at(j));
        fHistograms["hlam01"]->Fill(fFrames.lam01->at(j));
        fHistograms["hnhit"]->Fill(fFrames.nhit->at(j));
        fHistograms["httype"]->Fill(fFrames.ttype->at(j));
        fHistograms["hn_shared_hits"]->Fill(fFrames.n_shared_hits->at(j));
        fHistograms["hn_shared_segs"]->Fill(fFrames.n_shared_segs->at(j));
        fHistograms["hfarm_status"]->Fill(fFrames.farm_status->at(j));
        
        // -- fill hitmap vtx0
        int vtxL0Ladder = getVtxL0Ladder(fFrames.sid0->at(j));
        if (vtxL0Ladder >= 0) {
          fHistograms["h2hitmapvtx0"]->Fill(fFrames.z0->at(j), vtxL0Ladder);
          fHistograms[Form("h2hitmapvtx0_sid%03d", fFrames.sid0->at(j))]->Fill(fFrames.z0->at(j), TMath::ATan2(fFrames.y0->at(j), fFrames.x0->at(j)));
        }
        
        // -- comparison to MC 
        // -- resolution in momentum 
        // frames->Draw("TMath::Abs(p)-mc_p", "1==mc && 1==mc_prime && TMath::Abs(mc_pid) == 11 && mc_vr < 22 && TMath::Abs(mc_vz) < 55", "")
        
        
        fHistograms["h2lamvsprec"]->Fill(TMath::Abs(fFrames.p->at(j)), fFrames.lam01->at(j));
        // -- efficiency
        // root [22] mc_frames->Draw("mc_lam:TMath::Abs(mc_p)>>hmc", "TMath::Abs(mc_pid) == 11 && mc_vr < 22 && TMath::Abs(mc_vz) < 55 && ttype==42", "colz")
        // (long long) 108677
        // root [23] frames->Draw("mc_lam:TMath::Abs(mc_p)>>hrec", "1==mc && 1==mc_prime && TMath::Abs(mc_pid) == 11 && mc_vr < 22 && TMath::Abs(mc_vz) < 55 && ttype==42", "colz")
        // (long long) 66659
        // root [24] heff->Divide(h1, h2)
        
      }
    }
  }
  fHistograms["hinfo"]->SetBinContent(3, nHiTracks);
    
  // -- loop over mcframes tree
  if (fMcFrames.tree) fMcFrames.nEvents = fMcFrames.tree->GetEntries();
  for (int i = 0; i < fMcFrames.nEvents; ++i) {
    fMcFrames.tree->GetEntry(i);
    //cout << "fillHist::run() i = " << i << " fp->size() = " << fp->size() << endl;
    fHistograms["hn"]->Fill(fMcFrames.n);
    fHistograms["hn4"]->Fill(fMcFrames.n4);
    fHistograms["hn6"]->Fill(fMcFrames.n6);
    fHistograms["hn8"]->Fill(fMcFrames.n8);
    
   
    // -- fill per-track histograms
    for (unsigned int j = 0; j < fMcFrames.p->size(); ++j) {
      if (fMcFrames.goodReconstructibleTrack(j, 42)) {
        fHistograms["h2lamvspsim"]->Fill(TMath::Abs(fMcFrames.p->at(j)), fMcFrames.lam01->at(j));
      }
    }
  }
  fHistograms["h2lamvspeff"]->Divide(fHistograms["h2lamvsprec"], fHistograms["h2lamvspsim"]);

  for (int ix = 1; ix <= fHistograms["h2lamvspeff"]->GetNbinsX(); ++ix) {
    for (int iy = 1; iy <= fHistograms["h2lamvspeff"]->GetNbinsY(); ++iy) {
      if (fHistograms["h2lamvspeff"]->GetBinContent(ix, iy) > 0) {  
        fHistograms["h1lamvspreceff"]->Fill(fHistograms["h2lamvspeff"]->GetBinContent(ix, iy));
      }
      if (fHistograms["h2lamvspeff"]->GetBinContent(ix, iy) > 1.) {  
        cout << "fillHist::run() ix = " << ix << " iy = " << iy << " bin content = " << fHistograms["h2lamvspeff"]->GetBinContent(ix, iy) << endl;
      }
    }
  }
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
bool fillHist::checkVectorSizes(const fillHist::TreeData &b) {
  if (b.p->size() != b.x0->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: p->size() != x0->size() " << b.p->size() << " != " << b.x0->size() << endl;
    return false;
  }
  if (b.p->size() != b.y0->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: p->size() != y0->size() " << b.p->size() << " != " << b.y0->size() << endl;
    return false;
  }
  if (b.p->size() != b.z0->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: p->size() != z0->size() " << b.p->size() << " != " << b.z0->size() << endl;
    return false;
  }
  if (b.p->size() != b.t0->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: p->size() != t0->size() " << b.p->size() << " != " << b.t0->size() << endl;
    return false;
  }
  if (b.p->size() != b.t0_err->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: p->size() != t0_err->size() " << b.p->size() << " != " << b.t0_err->size() << endl;
    return false;
  }
  if (b.p->size() != b.t0_tl->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: p->size() != t0_tl->size() " << b.p->size() << " != " << b.t0_tl->size() << endl;
    return false;
  }
  if (b.p->size() != b.t0_fb->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: p->size() != t0_fb->size() " << b.p->size() << " != " << b.t0_fb->size() << endl;
    return false;
  }
  if (b.p->size() != b.t0_si->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: p->size() != t0_si->size() " << b.p->size() << " != " << b.t0_si->size() << endl;
    return false;
  }
  if (b.t0_tl_rms && b.p->size() != b.t0_tl_rms->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: p->size() != t0_tl_rms->size() " << b.p->size() << " != " << b.t0_tl_rms->size() << endl;
    return false;
  }
  if (b.t0_fb_rms && b.p->size() != b.t0_fb_rms->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: p->size() != t0_fb_rms->size() " << b.p->size() << " != " << b.t0_fb_rms->size() << endl;
    return false;
  }
  if (b.t0_si_rms && b.p->size() != b.t0_si_rms->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: p->size() != t0_si_rms->size() " << b.p->size() << " != " << b.t0_si_rms->size() << endl;
    return false;
  }
  if (b.p->size() != b.r->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: p->size() != r->size() " << b.p->size() << " != " << b.r->size() << endl;
    return false;
  }
  if (b.p->size() != b.rerr2->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: p->size() != rerr2->size() " << b.p->size() << " != " << b.rerr2->size() << endl;
    return false;
  }
  if (b.p->size() != b.perr2->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: p->size() != perr2->size() " << b.p->size() << " != " << b.perr2->size() << endl;
    return false;
  }
  if (b.p->size() != b.chi2->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: p->size() != chi2->size() " << b.p->size() << " != " << b.chi2->size() << endl;
    return false;
  }
  if (b.p->size() != b.tan01->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: p->size() != tan01->size() " << b.p->size() << " != " << b.tan01->size() << endl;
    return false;
  }
  if (b.p->size() != b.lam01->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: p->size() != lam01->size() " << b.p->size() << " != " << b.lam01->size() << endl;
    return false;
  }
  if (b.p->size() != b.sid0->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: p->size() != sid0->size() " << b.p->size() << " != " << b.sid0->size() << endl;
    return false;
  }
  if (b.p->size() != b.nhit->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: p->size() != nhit->size() " << b.p->size() << " != " << b.nhit->size() << endl;
    return false;
  }
  if (b.p->size() != b.ttype->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: p->size() != ttype->size() " << b.p->size() << " != " << b.ttype->size() << endl;
    return false;
  }
  if (b.p->size() != b.n_shared_hits->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: p->size() != n_shared_hits->size() " << b.p->size() << " != " << b.n_shared_hits->size() << endl;
    return false;
  }
  if (b.p->size() != b.n_shared_segs->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: p->size() != n_shared_segs->size() " << b.p->size() << " != " << b.n_shared_segs->size() << endl;
    return false;
  }
  if (b.p->size() != b.farm_status->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: p->size() != farm_status->size() " << b.p->size() << " != " << b.farm_status->size() << endl;
    return false;
  }
  if (b.p->size() != b.sid0->size()) {
    cout << "fillHist::checkVectorSizes() ERROR: p->size() != sid0->size() " << b.p->size() << " != " << b.sid0->size() << endl;
    return false;
  }
  return true;
}

// ----------------------------------------------------------------------
void fillHist::setupTrees() {
  fFrames.name = "frames";
  fFrames.tree = fInFile->Get<TTree>(fFrames.name.c_str());
  fFrames.nEvents = fFrames.tree->GetEntries();
  if (!fFrames.tree) {
    cout << "fillHist::setupTrees() ERROR: could not get tree frames" << endl;
    return;
  } else{
    bindTreeBranches(fFrames);
  }
  
  fMcFrames.name = "mc_frames";
  fMcFrames.tree = fInFile->Get<TTree>(fMcFrames.name.c_str());
  fMcFrames.nEvents = fMcFrames.tree->GetEntries();
  if (!fMcFrames.tree) {
    cout << "fillHist::setupTree() ERROR: could not get tree mc_frames" << endl;
  } else {
    bindTreeBranches(fMcFrames);
  }
}


// ----------------------------------------------------------------------
void fillHist::resetBranches(fillHist::TreeData &b) {
  b.runId = b.frameId = 0;
  b.flags = 0;
  b.x0 = b.y0 = b.z0 = b.t0 = b.t0_err = b.t0_rms = b.t0_tl = b.t0_fb = b.t0_si = nullptr;
  b.t0_tl_rms = b.t0_fb_rms = b.t0_si_rms = nullptr;
  b.r = b.rerr2 = b.p = b.perr2 = b.chi2 = b.tan01 = b.lam01 = nullptr;
  b.nhit = b.ttype = b.n_shared_hits = b.n_shared_segs = b.farm_status = nullptr;
  b.sid0 = nullptr;
  b.n = b.n4 = b.n6 = b.n8 = 0;
  b.mc = b.mc_prime = b.mc_type = nullptr;
  b.mc_pid = b.mc_tid = b.mc_mid = nullptr;
  b.mc_weight = b.mc_p = b.mc_pt = b.mc_phi = b.mc_lam = b.mc_theta = nullptr;
  b.mc_vx = b.mc_vy = b.mc_vz = b.mc_vr = b.mc_vt = b.mc_t0 = nullptr;
}


// ----------------------------------------------------------------------
void fillHist::bindTreeBranches(fillHist::TreeData &data) {
  if (!data.tree) return;
  data.tree->SetBranchStatus("*", 0);
  resetBranches(data);
  
  initBranch(data.tree, "x0", &data.x0);
  initBranch(data.tree, "y0", &data.y0);
  initBranch(data.tree, "z0", &data.z0);
  initBranch(data.tree, "t0", &data.t0);
  initBranch(data.tree, "t0_err", &data.t0_err);
  initBranch(data.tree, "t0_rms", &data.t0_rms);
  initBranch(data.tree, "t0_tl", &data.t0_tl);
  initBranch(data.tree, "t0_fb", &data.t0_fb);
  initBranch(data.tree, "t0_si", &data.t0_si);
  initBranch(data.tree, "t0_tl_rms", &data.t0_tl_rms);
  initBranch(data.tree, "t0_fb_rms", &data.t0_fb_rms);
  initBranch(data.tree, "t0_si_rms", &data.t0_si_rms);
  initBranch(data.tree, "r", &data.r);
  initBranch(data.tree, "rerr2", &data.rerr2);
  initBranch(data.tree, "p", &data.p);
  initBranch(data.tree, "perr2", &data.perr2);
  initBranch(data.tree, "chi2", &data.chi2);
  initBranch(data.tree, "tan01", &data.tan01);
  initBranch(data.tree, "lam01", &data.lam01);
  initBranch(data.tree, "nhit", &data.nhit);
  initBranch(data.tree, "ttype", &data.ttype);
  initBranch(data.tree, "n_shared_hits", &data.n_shared_hits);
  initBranch(data.tree, "n_shared_segs", &data.n_shared_segs);
  initBranch(data.tree, "farm_status", &data.farm_status);
  initBranch(data.tree, "sid0", &data.sid0);
  initBranch(data.tree, "n", &data.n);
  initBranch(data.tree, "n4", &data.n4);
  initBranch(data.tree, "n6", &data.n6);
  initBranch(data.tree, "n8", &data.n8);
  initBranch(data.tree, "mc", &data.mc);
  initBranch(data.tree, "mc_prime", &data.mc_prime);
  initBranch(data.tree, "mc_type", &data.mc_type);
  initBranch(data.tree, "mc_pid", &data.mc_pid);
  initBranch(data.tree, "mc_tid", &data.mc_tid);
  initBranch(data.tree, "mc_mid", &data.mc_mid);
  initBranch(data.tree, "mc_weight", &data.mc_weight);
  initBranch(data.tree, "mc_p", &data.mc_p);
  initBranch(data.tree, "mc_pt", &data.mc_pt);
  initBranch(data.tree, "mc_phi", &data.mc_phi);
  initBranch(data.tree, "mc_lam", &data.mc_lam);
  initBranch(data.tree, "mc_theta", &data.mc_theta);
  initBranch(data.tree, "mc_vx", &data.mc_vx);
  initBranch(data.tree, "mc_vy", &data.mc_vy);
  initBranch(data.tree, "mc_vz", &data.mc_vz);
  initBranch(data.tree, "mc_vr", &data.mc_vr);
  initBranch(data.tree, "mc_vt", &data.mc_vt);
  initBranch(data.tree, "mc_t0", &data.mc_t0);
}


// ----------------------------------------------------------------------
void fillHist::initBranch(TTree *tree, string name, int* pvar) {
  if (tree->GetBranch(name.c_str()) == nullptr) {
    cout << "fillHist::initBranch() ERROR: branch " << name << " not found in tree" << endl;
    return;
  }
  tree->SetBranchStatus(name.c_str(), 1);
  tree->SetBranchAddress(name.c_str(), pvar);
}

// ----------------------------------------------------------------------
void fillHist::initBranch(TTree *tree, string name, float* pvar) {
  if (tree->GetBranch(name.c_str()) == nullptr) {
    cout << "fillHist::initBranch() ERROR: branch " << name << " not found in tree" << endl;
    return;
  }
  tree->SetBranchStatus(name.c_str(), 1);
  tree->SetBranchAddress(name.c_str(), pvar);
}

// ----------------------------------------------------------------------
void fillHist::initBranch(TTree *tree, string name, double* pvar) {
  if (tree->GetBranch(name.c_str()) == nullptr) {
    cout << "fillHist::initBranch() ERROR: branch " << name << " not found in tree" << endl;
    return;
  }
  tree->SetBranchStatus(name.c_str(), 1);
  tree->SetBranchAddress(name.c_str(), pvar);
}

// ----------------------------------------------------------------------
void fillHist::initBranch(TTree *tree, string name, string** pvar) {
  cout << "initBranch(" << name << "),  pvar = " << pvar << endl;
  if (tree->GetBranch(name.c_str()) == nullptr) {
    cout << "fillHist::initBranch() ERROR: branch " << name << " not found in tree" << endl;
    return;
  }
  tree->SetBranchStatus(name.c_str(), 1);
  tree->SetBranchAddress(name.c_str(), pvar);
}

// ----------------------------------------------------------------------
void fillHist::initBranch(TTree *tree, string name, vector<int>** pvect) {
  if (tree->GetBranch(name.c_str()) == nullptr) {
    cout << "fillHist::initBranch() ERROR: branch " << name << " not found in tree" << endl;
    return;
  }
  tree->SetBranchStatus(name.c_str(), 1);
  tree->SetBranchAddress(name.c_str(), pvect);
}
// ----------------------------------------------------------------------
void fillHist::initBranch(TTree *tree, string name, vector<unsigned int>** pvect) {
  if (tree->GetBranch(name.c_str()) == nullptr) {
    cout << "fillHist::initBranch() ERROR: branch " << name << " not found in tree" << endl;
    return;
  }
  tree->SetBranchStatus(name.c_str(), 1);
  tree->SetBranchAddress(name.c_str(), pvect);
}

// ----------------------------------------------------------------------
void fillHist::initBranch(TTree *tree, string name, vector<double>** pvect) {
  if (tree->GetBranch(name.c_str()) == nullptr) {
    cout << "fillHist::initBranch() ERROR: branch " << name << " not found in tree" << endl;
    return;
  }
  tree->SetBranchStatus(name.c_str(), 1);
  tree->SetBranchAddress(name.c_str(), pvect);
}

// ----------------------------------------------------------------------
bool fillHist::TreeData::goodReconstructibleTrack(int idx, int trkType) {
  bool result(false);
  if (TMath::Abs(mc_pid->at(idx)) != 11) return false;
  
  if (mc_vr->at(idx) > 22.) return false;
  if (TMath::Abs(mc_vz->at(idx)) > 55.) return false;
  if (TMath::Abs(mc_p->at(idx)) < 10.) return false;
  
  if (ttype->at(idx) != trkType) return false;
  
  if (mc_prime->at(idx) != 1) return false;
  if (mc->at(idx) != 1) return false;
  
  return true;
}

// ----------------------------------------------------------------------
bool fillHist::TreeData::goodReconstructedTrack(int idx, int trkType) {
  bool result(false);
  if (TMath::Abs(mc_pid->at(idx)) != 11) return false;
  
  if (mc_vr->at(idx) > 22.) return false;
  if (TMath::Abs(mc_vz->at(idx)) > 55.) return false;
  if (TMath::Abs(mc_p->at(idx)) < 10.) return false;

  if (TMath::Abs(p->at(idx)) < 10.) return false;

  if (ttype->at(idx) != trkType) return false;
  
  if (mc_prime->at(idx) != 1) return false;
  if (mc->at(idx) != 1) return false; 
  return true;
}