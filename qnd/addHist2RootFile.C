#include <iostream>
#include <sstream>

#include <dirent.h>  /// for directory reading
#include <algorithm> /// for sorting

// ----------------------------------------------------------------------
// -- extract a histogram from multiple files run-%d.root into one root file.
// -- must be called in directory where all the root files are located.
//
// -- Usage: 
//           cd data/slurm/mu3e-dev-ursl-fibres-smb-radiation
//           root
//           .L ~/mu3e/mu3eanca/qnd/addHist2RootFile.C
//           addRuns(9998, 9999) // for testing
// ----------------------------------------------------------------------



// ----------------------------------------------------------------------
void addHist2RootFile(string histname, string infile, string outfile) {
  
  int runnumber(-1);
  sscanf(infile.c_str(), "run-%d.root", &runnumber); 
  
  TFile *fIn  = TFile::Open(infile.c_str()); 
  TFile *fOut = TFile::Open(outfile.c_str(), "UPDATE");

  TH1 *h = (TH1*)fIn->Get(histname.c_str());
  h->SetName(Form("run%d_%s", runnumber, h->GetName())); 
  cout << "writing " << h->GetName() << " into " << outfile << endl;
  h->SetDirectory(fOut);
  h->Write();
  fOut->Close();

  fIn->Close();
}


// ----------------------------------------------------------------------
// tests: 9998 9999 
void addRuns(int run1 = 40000, int run2 = 41000,
             string histname = "stat/FibreSmbMuTrig/hFibreSmbDose2",
             string outfile = "run-out.root") {
  // -- must be in directory with the many rootfiles
  string dirName = "."; 

  vector<string> vfiles;
  DIR *folder;
  struct dirent *entry;
  
  folder = opendir(dirName.c_str());
  if (folder == NULL) {
    puts("Unable to read directory");
    return(1);
  } 
  
  while ((entry=readdir(folder))) {
    int runnumber(-1);
    sscanf(entry->d_name, "run-%d.root", &runnumber);
    if (runnumber < 0) {
      cout << "runnumber not parsed ->" << entry->d_name << "<-" << endl;
      continue;
    }
    if (runnumber < run1) continue;
    if (runnumber > run2) continue;
    
    if (8 == entry->d_type) {
      vfiles.push_back(entry->d_name);
    }
  }
  closedir(folder);
  
  sort(vfiles.begin(), vfiles.end());    

  for (unsigned int i = 0; i < vfiles.size(); ++i) {
    addHist2RootFile(histname, vfiles[i], outfile);
  }

  
}
