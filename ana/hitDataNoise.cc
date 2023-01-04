#include "hitDataNoise.hh"
#include "hitDataIncludes.hh"

// ----------------------------------------------------------------------
// -- Usage
// --------
//
// bin/runHitDataAna -f ~/data/mu3e/run2022/root_output_files/dataTree00442.root -D nmf -p noise
// bin/runHitDataAna -c chains/nmfchain -D nmfchain -p noise > & ! chain.log
//
// noise mask files are dumped per run.
// combine them ad libitum with anaNoiseMaskFiles
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
hitDataNoise::hitDataNoise(TChain *chain, string treeName): hitDataBase(chain, treeName) {
  cout << "==> hitDataNoise: constructor..." << endl;
  if (chain == 0) {
    cout << "You need to pass a chain!" << endl;
  }
  cout << "==> hitDataNoise: constructor fpChain: " << fpChain << "/" << fpChain->GetName()
       << " entries = " <<   fNentries
       << endl;

  fModeNoiseLimit = 1;
  fNoiseLevel = 1.5;
  fSuffix = ""; 
  
}


// ----------------------------------------------------------------------
hitDataNoise::~hitDataNoise() {
  cout << "==> hitDataNoise: destructor ..." << endl;
  if (!fpChain) return;
  delete fpChain->GetCurrentFile();
}



// ----------------------------------------------------------------------
void hitDataNoise::bookHist(int runnumber) {
  if (runnumber < 1) return;
 
  static bool first(true);
  stringstream ss;
  if (first) {
    first = false;
    hitDataBase::bookHist(runnumber);
    cout << "==> hitDataNoise: bookHist> run " << runnumber << endl;
    
    // -- create one per chipID, irrespective of it is present. Ignore the original setup ("get unique chipID")
    for (int i = 0; i < 120; ++i) {
      funique_chipIDs.emplace_back(i);
    }

    // -- create hitmap histo
    for (int i=0; i<120; i++) {
      ss.str("");
      ss << Form("hitmap_run%d_chipID%d", runnumber, i);
      fhitmaps.push_back(new TH2F(ss.str().c_str(), ss.str().c_str(), 256, 0, 256, 250, 0, 250));
      ss.str("");
      ss << Form("noisemap_run%d_chipID%d", runnumber, i);
      fnoisemaps.push_back(new TH1F(ss.str().c_str(), ss.str().c_str(), 2000, 0, 2000));
      // cout << "created " << ss.str().c_str() << endl;
    }
    
    fhErrors = new TH1F("hErrors", "hErrors", 120, 0., 120.);
    fhTotal  = new TH1F("hTotal", "hTotal", 120, 0., 120.);
    fhRatio  = new TH1F("hRatio", Form("Error ratio (row > 249) run %d", fRun), 120, 0., 120.);
  } else {
    fpHistFile->cd();
    fpHistFile->Write();

    for (int i=0; i<120; i++) {
      delete fhitmaps[i];
      delete fnoisemaps[i];
    }
    fhitmaps.clear();
    fnoisemaps.clear();
    
    for (int i=0; i<120; i++) {
      ss.str("");
      ss << Form("hitmap_run%d_chipID%d", runnumber, i);
      fhitmaps.push_back(new TH2F(ss.str().c_str(), ss.str().c_str(), 256, 0, 256, 250, 0, 250));
      ss.str("");
      ss << Form("noisemap_run%d_chipID%d", runnumber, i);
      fnoisemaps.push_back(new TH1F(ss.str().c_str(), ss.str().c_str(), 2000, 0, 2000));
      // cout << "created " << ss.str().c_str() << endl;
    }
  }
  
  fhRatio->Sumw2(kTRUE);
  fhErrors->Reset();
  fhTotal->Reset();
  fhRatio->Reset();
}



// ----------------------------------------------------------------------
void hitDataNoise::eventProcessing() {
  int VERBOSE(0);
  // -- count good hits in this event
  for (int ihit = 0; ihit < fv_col->size(); ++ihit) {
    fChipID = fv_chipID->at(ihit); 
    frow  = fv_row->at(ihit);
    fcol  = fv_col->at(ihit); 
    if (fChipID > 119) {
      if (VERBOSE) cout << Form("LVDS error: col/row/chip = %d/%d/%d", fcol, frow, fChipID)
                        << endl;
      continue;
    }
    if (VERBOSE) cout << Form("col/row/chip = %d/%d/%d", fcol, frow, fChipID)
                      << endl;
    fhitmaps.at(fChipID)->Fill(fcol, frow);

    fhTotal->Fill(fChipID);
    if (frow > 249) {
      fhErrors->Fill(fChipID);
    }
    if (fcol > 255) {
      fhErrors->Fill(fChipID);
    }
  }    

}


// ----------------------------------------------------------------------
void hitDataNoise::runEndAnalysis(int runnumber) {
  cout << "==> runEndAnalysis for run = " << runnumber << endl;
  fhRatio->Divide(fhErrors, fhTotal);
  
  printNonZero(fhErrors);

  // -- find noisy pixels per chipID
  std::map<int, std::vector<std::pair<uint8_t, uint8_t>>> noisy_pixels;
  vector<string> vPrint;
  
  bool DBX(false);
  for (int chipID : funique_chipIDs) {
    // -- skip scintillator and bad chipIDs
    if (chipID >= 120) continue;
    
    noisy_pixels[chipID] = std::vector<std::pair<uint8_t, uint8_t> >();
    
    // -- try to find noise level
    TH2F *h2 = fhitmaps.at(chipID);
    TH1F *h1 = fnoisemaps.at(chipID);
    for (int32_t ny = 1; ny <= h2->GetYaxis()->GetNbins(); ny++) {
      for (int32_t nx = 1; nx <= h2->GetXaxis()->GetNbins(); nx++) {
        fnoisemaps.at(chipID)->Fill(h2->GetBinContent(nx, ny));
      }
    }
    // -- determine noise_limit, based on modeNoiseLevel and noiseLevel
    double noise_limit(0.);
    if (1 == fModeNoiseLimit) {
      noise_limit = fNoiseLevel;
    } else if (2 == fModeNoiseLimit) {
      noise_limit = h1->GetMean() + fNoiseLevel*h1->GetRMS() + 0.5;
    }
    cout << "chipID " << chipID
         << " (maximum: " << h2->GetMaximum() << ") mean(nhit) = " << h1->GetMean()
         << " RMS = " << h1->GetRMS()
         << " noise level = " << noise_limit
         << endl;
        
    int tot_noisy_pixels = 0;
    vector<uint8_t> vNoise; 
    string spix(Form("(max = %d), pix: ", static_cast<int>(h2->GetMaximum())));
    for (int32_t nx = 1; nx <= fhitmaps.at(chipID)->GetXaxis()->GetNbins(); nx++) {
      if (DBX) cout << "filling col " << nx-1 << endl;
      for (int32_t ny = 1; ny <= fhitmaps.at(chipID)->GetYaxis()->GetNbins(); ny++) {
        int nhit = static_cast<int>(fhitmaps.at(chipID)->GetBinContent(nx, ny)); 
        // -- if the noise limit is 0 then there will be NO noisy pixels
        if ((noise_limit > 0) && (nhit > noise_limit)) {
          if (DBX) std::cout << "Chip ID " << chipID << ", Found noisy pixel at " << nx-1 << ", " << ny-1
			     << " nhits = " << nhit << " noise_limit = " << noise_limit
			     << std::endl;
          tot_noisy_pixels++;
          noisy_pixels[chipID].push_back(std::pair<uint8_t, uint8_t>(nx-1, ny-1));
          vNoise.push_back(0x0);
          spix += string(Form("%d/%d(%d) ", nx-1, ny-1, nhit));
        } else {
          vNoise.push_back(0xff);
        }
      }
      // -- fill up to 255
      // for (int32_t ny = 251; ny <= 256; ny++) {
      //   if (DBX) cout << "+filling row " << ny-1 << endl;
      //   vNoise.push_back(0xda);
      // }

      // -- store 0xdada as end-of-col marker
      vNoise.push_back(0xda);
      vNoise.push_back(0xda);
      // -- store number of col
      vNoise.push_back(0xda);
      vNoise.push_back(nx-1);
      // -- store LVDS error flag
      vNoise.push_back(0xda);
      if (fhErrors->GetBinContent(chipID+1) > 0) {
        vNoise.push_back(0x01);
      } else {
        vNoise.push_back(0x00);
      }
    }
    std::cout << " with a  total of " << tot_noisy_pixels << " (" << tot_noisy_pixels*100/64000 << "%)\n";
    writeNoiseMaskFile(vNoise, runnumber, chipID, fSuffix, fOutputDirectoryName);

    vPrint.push_back(Form("chipID %3d, n. level = %5.3f, N(n. pixels) = %d %s",
                          chipID, noise_limit, tot_noisy_pixels, spix.c_str()));
    vNoise.clear();
  }

  ofstream o(Form("%s/summaryNoiseMaskFile%s-run%d.txt", fOutputDirectoryName.c_str(), fSuffix.c_str(), runnumber)); 
  cout << "Summary of noisy (n.) pixels for run "  << runnumber << endl;
  o << "Summary of noisy (n.) pixels for run "  << runnumber << endl;
  for (unsigned int i = 0; i < vPrint.size(); ++i) {
    cout << vPrint[i] << endl;
    o << vPrint[i] << endl;
  }
  o.close();
    
  return;
}


// ----------------------------------------------------------------------
// -- dump vector<uint8_t> into mask file
// ----------------------------------------------------------------------
void hitDataNoise::writeNoiseMaskFile(vector<uint8_t> noise, int runnumber, int chipID,
                                      string name, string dir) {
  string filename("");
  if (runnumber > 0) {
    filename = Form("%s/noiseMaskFile%s-run%d-chipID%d", dir.c_str(), name.c_str(), runnumber, chipID);
  } else {
    filename = Form("%s/noiseMaskFile%s-chipID%d", dir.c_str(), name.c_str(), chipID);
  }
 
  ofstream o(filename); 
  for (unsigned int i = 0; i < noise.size(); ++i) {
    o << noise[i];
  }
  o.close();
}


