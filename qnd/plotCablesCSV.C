#include <dirent.h>  /// for directory reading


// ----------------------------------------------------------------------
void showGraph(string filename = "/Users/ursl/data/mu3e/2411-cableTesting/0450-0004-000001/0.1.csv") {
  TTree *t = new TTree("t", "tree from CSV");
  t->ReadFile(filename.c_str(), "PNT/I:FREQ/D:MAG/D");
  int n = t->Draw("MAG:FREQ","","goff");
  TGraph *g = new TGraph(n, t->GetV2(), t->GetV1());
  g->SetTitle(filename.c_str());
  TAxis *axis = g->GetYaxis();
  axis->SetLimits(-80., 10.);  

  g->Draw("al");
}


// ----------------------------------------------------------------------
void showAllGraphs(string dirPath = "/Users/ursl/data/mu3e/2411-cableTesting/0450-0004-000001") {

  if ('/' == dirPath.back()) dirPath.pop_back();
  string ofilename = dirPath.substr(dirPath.rfind("/") + 1);
  cout << "ofilename: " << ofilename << endl;
  
  vector<string> vfiles;
  DIR *folder;
  struct dirent *entry;
  
  folder = opendir(dirPath.c_str());
  if (folder == NULL) {
    cout << "Unable to read directory ->" << dirPath << "<-" << endl;
  } else {
    while ((entry=readdir(folder))) {
      if (8 == entry->d_type) {
        vfiles.push_back(entry->d_name);
      }
    }
    closedir(folder);
    sort(vfiles.begin(), vfiles.end());
  }

  c0.Clear();
  c0.Divide(2,3);

  int cnt(1), conn(0); 
  for (unsigned int i = 0; i < vfiles.size(); ++i) {
    c0.cd(cnt);
    cout << "############## " << vfiles[i] << endl;
    showGraph(dirPath + "/" + vfiles[i]);
    if (5 == cnt) {
      cout << "############## saveAs " << Form("%s-dabstp%d.pdf", ofilename.c_str(), conn) << endl;
      c0.SaveAs(Form("%s-dabstp%d.pdf", ofilename.c_str(), conn));
      c0.Clear();
      c0.Divide(2,3);
      cnt = 0; 
      ++conn;
    } 
    ++cnt;
  }
  
  
}

// ----------------------------------------------------------------------
void showAllCables() {
  
  showAllGraphs("/Users/ursl/data/mu3e/2411-cableTesting/0450-0004-000001");
  showAllGraphs("/Users/ursl/data/mu3e/2411-cableTesting/0450-0004-000002");
  showAllGraphs("/Users/ursl/data/mu3e/2411-cableTesting/0450-0005-000005");
  showAllGraphs("/Users/ursl/data/mu3e/2411-cableTesting/0504-0001-000005");
  showAllGraphs("/Users/ursl/data/mu3e/2411-cableTesting/0505-0001-000004");

}
