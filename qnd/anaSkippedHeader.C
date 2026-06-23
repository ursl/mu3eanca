#include <dirent.h>
#include <iostream>
#include <vector>
#include <string>


using namespace::std;

// ----------------------------------------------------------------------
void anaSkippedHeader(int minRun = 3000, int maxRun = 9600) {

  // -- get all files
  vector<string> vfiles;
  DIR *folder = opendir("/Users/ursl/data/mu3e/run2025/rawv2");
  if (folder == NULL) {
    cout << "Error failed to open /Users/ursl/data/mu3e/run2025/rawv2" << endl;
    return;
  }
  struct dirent *entry;
  while ((entry = readdir(folder))) {
    if (entry->d_type == DT_REG) {
      vfiles.push_back(string("/Users/ursl/data/mu3e/run2025/rawv2/") + string(entry->d_name));
    }
  }
  closedir(folder);
  for (auto &file : vfiles) {
    cout << "file = " << file << endl;
  }


  vector<int> vbadRuns = {6277,6252,6179,6031,5959,4755,4622,4621,4612,4611,4601,4600,4599,4598,4597,4596,4595,4594,4593,4592,4576,4495,4494,3823,3803,3802,3800,3658,3656,3650,3648,3643,3623,3571,3489,3485,3481,3473,3469,3310,3309,3031,3030,3029,3028,3027,3023,3017,3014,3013,3009,3008};

  // -- read each file
  TH1D *hgoodPixelData = new TH1D("hgoodPixelData", "good pixel data fraction", 102, -0.01, 1.01);
  TH1D *hgoodPixelDataUnsortedFraction = new TH1D("hgoodPixelDataUnsortedFraction", "good pixel data fraction: one pixel FEB unsorted", 102, -0.01, 1.01);
  TH1D *hgoodPixelDataVsRunNumber = new TH1D("hgoodPixelDataVsRunNumber", "good pixel data  fraction (vs run number)", maxRun - minRun, minRun, maxRun);
  hgoodPixelDataVsRunNumber->SetMarkerStyle(24);
  hgoodPixelDataVsRunNumber->SetMarkerSize(0.5);
  TH1D *hgoodPixelDataUnsortedFractionVsRunNumber = new TH1D("hgoodPixelDataUnsortedFractionVsRunNumber", "good pixel data fraction: one pixel FEB unsorted (vs run number)", maxRun - minRun, minRun, maxRun);
  hgoodPixelDataUnsortedFractionVsRunNumber->SetMarkerStyle(24);
  hgoodPixelDataUnsortedFractionVsRunNumber->SetMarkerSize(0.5);

  int nruns(0);
  for (auto &file : vfiles) {
    cout << "reading file = " << file << endl;
    int runnumber = ::stoi(file.substr(file.rfind("run") + 3, 5));
    cout << "runnumber = " << runnumber << endl;
    if (runnumber < minRun || runnumber > maxRun) continue;
    if (find(vbadRuns.begin(), vbadRuns.end(), runnumber) != vbadRuns.end()) continue;

    ifstream INS;
    INS.open(file.c_str());
    string line;
    long long goodPixelDataStart(0), goodPixelDataEnd(0), goodPixelDataUnsorted(0), lastFrame(0);
    while (getline(INS, line)) {
      if (line.find("first_frame_at_least_one_pixel_FEB_had_unsorted_hit_data") != string::npos) {
        size_t pos = line.rfind(":");
        string sframe = line.substr(pos+2, line.size() - pos - 3);
        cout << " unsorted ->" << sframe << "<- " << endl;
        goodPixelDataUnsorted = ::stol(sframe);
        // int iframe = ::stoi(sframe);
        // cout << "iframe = " << iframe << endl;
      }
      if (line.find("last_frame_of_the_run") != string::npos) {
        size_t pos = line.rfind(":");
        string sframe = line.substr(pos+2, line.size() - pos - 3);
        cout << " lastframe ->" << sframe << "<- " << endl;
        lastFrame = ::stol(sframe);
      }
      if (line.find("start_frame_good_pixel_data") != string::npos) {
        size_t pos = line.rfind(":");
        string sframe = line.substr(pos+2, line.size() - pos - 3);
        cout << " start ->" << sframe << "<- " << endl;
        goodPixelDataStart = ::stol(sframe);
      }
      if (line.find("end_frame_good_pixel_data") != string::npos) {
        size_t pos = line.rfind(":");
        string sframe = line.substr(pos+2, line.size() - pos - 3);
        cout << " endpixel ->" << sframe << "<- " << endl;
        goodPixelDataEnd = ::stol(sframe);
      }
    }
    double goodPixelDataFraction = (goodPixelDataEnd - goodPixelDataStart + 1.) / (lastFrame - goodPixelDataStart + 1.);
    cout << "goodPixelDataFraction = " << goodPixelDataFraction << " " 
    << goodPixelDataStart << " "
    << goodPixelDataEnd << " "
    << lastFrame << " "
    << goodPixelDataFraction << endl;
    if (goodPixelDataFraction < 0.) goodPixelDataFraction = 1.0;
    if (goodPixelDataFraction > 1.) goodPixelDataFraction = -1.0;
    hgoodPixelData->Fill(goodPixelDataFraction);
    double goodPixelDataUnsortedFraction = (goodPixelDataUnsorted - goodPixelDataStart + 1.) / (lastFrame - goodPixelDataStart + 1.);
    if (goodPixelDataUnsortedFraction < 0.) goodPixelDataUnsortedFraction = 1.0;
    if (goodPixelDataUnsortedFraction > 1.) goodPixelDataUnsortedFraction = -1.0;
    cout << "goodPixelDataUnsortedFraction = " << goodPixelDataUnsortedFraction << endl;
    hgoodPixelDataUnsortedFraction->Fill(goodPixelDataUnsortedFraction);
    int ibin = hgoodPixelDataVsRunNumber->FindBin(runnumber);
    hgoodPixelDataVsRunNumber->SetBinContent(ibin, goodPixelDataFraction);
    ibin = hgoodPixelDataUnsortedFractionVsRunNumber->FindBin(runnumber);
    hgoodPixelDataUnsortedFractionVsRunNumber->SetBinContent(ibin, goodPixelDataUnsortedFraction);
    INS.close();
    nruns++;
  }

  gStyle->SetOptStat(0);
  TCanvas *c1 = new TCanvas("c1", "c1", 1000, 1000);
  c1->SetLogy(0);
  hgoodPixelData->Draw("hist");
  tl->SetTextSize(0.03);
  tl->DrawLatexNDC(0.2, 0.8, Form("runMin = %d, runMax = %d", minRun, maxRun));
  tl->DrawLatexNDC(0.2, 0.7, Form("mean: %5.2f", hgoodPixelData->GetMean()));
  c1->SaveAs(Form("hgoodPixelData-%d-%d.pdf", minRun, maxRun));
 
  c1->Clear();
  c1->SetLogy(0);
  hgoodPixelDataUnsortedFraction->Draw("hist");
  tl->SetTextSize(0.03);
  tl->DrawLatexNDC(0.2, 0.8, Form("runMin = %d, runMax = %d", minRun, maxRun));
  tl->DrawLatexNDC(0.2, 0.7, Form("mean: %5.2f", hgoodPixelDataUnsortedFraction->GetMean()));
  c1->SaveAs(Form("hgoodPixelDataUnsortedFraction-%d-%d.pdf", minRun, maxRun));

  c1->Clear();
  c1->SetLogy(0);
  hgoodPixelDataVsRunNumber->Draw("p");
  c1->SaveAs(Form("hgoodPixelDataVsRunNumber-%d-%d.pdf", minRun, maxRun));

  c1->Clear();
  c1->SetLogy(0);
  hgoodPixelDataUnsortedFractionVsRunNumber->Draw("p");
  c1->SaveAs(Form("hgoodPixelDataUnsortedFractionVsRunNumber-%d-%d.pdf", minRun, maxRun));
  delete c1;
  delete hgoodPixelData;
  delete hgoodPixelDataUnsortedFraction;
  delete hgoodPixelDataVsRunNumber;
  delete hgoodPixelDataUnsortedFractionVsRunNumber;
  cout << "nruns = " << nruns << endl;
}


// ----------------------------------------------------------------------
void runAll() {
  anaSkippedHeader(3000, 9600);
  anaSkippedHeader(3000, 6000);
  anaSkippedHeader(7000, 9600);
}