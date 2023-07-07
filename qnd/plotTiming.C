#include <fstream> /// ifstream
#include <dirent.h>  /// for directory reading
#include <algorithm> /// for sorting

// ----------------------------------------------------------------------
// moor>rr
// Loading libs 1
// root [0] .L plotTiming.C
// root [1] plotTiming("json")
// root [2] plotTiming("rest")
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
void plotTiming(string filepart = "json",
                double trirecTime = 30000, 
                string dirName = "cdb-timing1") {
  
  // -- definition of timing measurements
  vector<string> vTiming = {
    "ctor", 
    "registerCalibration",
    "getpayload",
    "update"
  };

  vector<string> vFiles;
  
  // -- read all files in directory with matching pattern
  struct dirent *entry;
  DIR *folder = opendir(dirName.c_str());
  if (folder == NULL) {
    puts("Unable to read directory");
    return(1);
  } 
  
  while ((entry=readdir(folder))) {
    if (8 == entry->d_type) {
      string fname(entry->d_name); 
      if (string::npos == fname.find(filepart)) continue;
      vFiles.push_back(dirName + "/" + fname);
    }
  }
  closedir(folder);
  sort(vFiles.begin(), vFiles.end());    
  
  // -- figure out which calibrations to plot from first file
  vector<string> vCal;
  map<string, double> vCalMaxTime;
  ifstream INS;
  string sline;
  INS.open(vFiles[0]);
  while (getline(INS, sline)) {
    if ((string::npos != sline.find("::timing")) && (string::npos != sline.find("ctor"))
        ) {
      // -- rfind :: and \s
      size_t ista = sline.rfind("::") + 2;
      size_t iend = sline.rfind(" ctor");
      string cal = sline.substr(ista, iend-ista);
      vCal.push_back(cal);
    }

    for (auto itc: vCal) {
      if ((string::npos != sline.find("::timing"))
          && (string::npos != sline.find(itc))
          && (string::npos != sline.find("update"))
          ) {
        size_t itime = sline.find("us");
        string stime = sline.substr(0, itime);
        double dtime = ::stod(stime);
        vCalMaxTime.insert(make_pair(itc, dtime));
      }
    }
  }
  INS.close();
  
  // -- define histograms for each calibration and timing setup
  map<string, TH1D*> mHists;
  for (auto it: vTiming) {
    for (auto ic: vCal) {
      string name = ic + "_" + it;
      string title = ic + "_" + it + " (" + filepart + ")";
      double dmax = 2.*vCalMaxTime[ic];
      if (string::npos != it.find("ctor")) dmax = 0.01*dmax;
      if (string::npos != it.find("registerCalibration")) dmax = 0.01*dmax;
      mHists.insert(make_pair(name, new TH1D(name.c_str(), title.c_str(),
                                             100, 0., dmax)));
    }
  }
  string title = "trirec running time (" + filepart + ")";
  mHists.insert(make_pair("trirec", new TH1D("trirec", title.c_str(), 100, 0., trirecTime)));
  setTitles(mHists["trirec"], "time [ms]", "entries", 0.05, 1.0, 1.0, 0.035);

  // -- do analysis of all defined vTiming and fill histograms
  for (auto itf: vFiles) {
    INS.open(itf); 
    while (getline(INS, sline)) {
      for (auto itt: vTiming) {
        if ((string::npos != sline.find("::timing"))
            && (string::npos != sline.find(itt))
          ) {
          string scal; 
          for (auto itc: vCal) {
            if (string::npos != sline.find(itc)) {
              scal = itc;
              break;
            }
          }
            
          size_t itime = sline.find("us");
          string stime = sline.substr(0, itime);
          double dtime = ::stod(stime);
          if (0) cout << itf
                      << " cal -> " << scal
                      << " t ->" << itt << "<-"
                      << " time ->" << stime << "/" << dtime << "<-"
                      << " sline: " << sline
                      << endl;
          string name = scal + "_" + itt;
          mHists[name]->Fill(dtime);
          setTitles(mHists[name], "time [usec]", "entries", 0.05, 1.0, 1.0, 0.035);
        }
      }

      if ((string::npos != sline.find("::timing")) && (string::npos != sline.find("trirec::main"))
          ) {
        size_t itime = sline.find("ms");
        string stime = sline.substr(0, itime);
        double dtime = ::stod(stime);
        if (1) cout << itf
                    << " trirec::main> stime = " << stime
                    << " / dtime = " << dtime
                    << endl;
        mHists["trirec"]->Fill(dtime);
      }

    }
    INS.close();
  }

  for (auto it: mHists) {
    it.second->Draw();
    c0.SaveAs(("plotTiming-" + filepart + "-" + it.first + ".pdf").c_str());
  }
  
}
