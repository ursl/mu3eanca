#include "anaLadder.hh"

#include <iostream>
#include <vector>

#include <TROOT.h>
#include <TBranch.h>
#include <TVector3.h>
#include <TChain.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TTimeStamp.h>

#include "util/util.hh"
#include "db0/cdb2/cdbUtil.hh"

using namespace std;

// ----------------------------------------------------------------------
anaLadder::anaLadder(string dirname, string pn): fDirectory(dirname),
                                                 fLadderPN(pn) {
  cout << "anaLadder::anaLadder(" << fDirectory
       << ", " << fLadderPN
       << ") ctor" << endl;

  // -- fill map with empty struct
  for (int ic = 1; ic <= 6; ++ic) {
    string chipID = Form("C%d", ic);
    struct noise_scan a;
    fNoiseScan.insert({chipID, a});
    fNoiseScan[chipID].NoisyPixels.clear();
  }

  parseFiles();

  printAll();
};

// ----------------------------------------------------------------------
anaLadder::anaLadder() {
  //  cout << "anaLadder::anaLadder ctor" << endl;
  bookHist();
};

// ----------------------------------------------------------------------
anaLadder::~anaLadder() {

}

// ----------------------------------------------------------------------
void anaLadder::parseFiles() {

  vector<string> halves = {"US", "DS"};
 
  // -- start with noise files to fill a vector
  for (auto it: halves) {
    int startIdx(1); 
    if (it == "DS") startIdx = 4;
    ifstream INS;
    string noiseFile = fDirectory + "/noise_scan_" + fLadderPN + "_" + it + ".json";
    cout << "Reading file: " << noiseFile << endl;   
    INS.open(noiseFile);
    
    std::stringstream buffer;
    buffer << INS.rdbuf();
    INS.close();
    string lBuffer = buffer.str();
    
    vector<string> params = {"Noisy Pixels", "Noisy Hits", "Iterations"
                             , "Not maskable Pixels"
                             , "Errorrate_link_A",  "Errorrate_link_B",  "Errorrate_link_C"
    };
    
    for (auto ip: params) {
      for (int ic = startIdx; ic < startIdx+3; ++ic) {
        string chipID = Form("C%d", ic);
        vector<string> getChip = {chipID, ip};
        string sarr = jsonGetVector(lBuffer, getChip);
        vector<string> svector = split(sarr, ',');
        cout << fLadderPN + "_" + it + "_" + chipID << ": " << ip << endl;        
        vector<int> *v;
        if (ip == "Noisy Pixels") v = &(fNoiseScan[chipID].NoisyPixels);
        if (ip == "Noisy Hits") v = &(fNoiseScan[chipID].NoisyHits);
        if (ip == "Iterations") v = &(fNoiseScan[chipID].Iterations);
        if (ip == "Not maskable Pixels") v = &(fNoiseScan[chipID].NotMaskablePixels);
        if (ip == "Errorrate_link_A") v =  &(fNoiseScan[chipID].Errorrate_link_A);
        if (ip == "Errorrate_link_B") v =  &(fNoiseScan[chipID].Errorrate_link_B);
        if (ip == "Errorrate_link_C") v =  &(fNoiseScan[chipID].Errorrate_link_C);
        for (auto iv: svector) {
          v->push_back(stoi(iv));
        }
      }
    }
  }
}


// ----------------------------------------------------------------------
void anaLadder::bookHist() {
  
}


  
// ----------------------------------------------------------------------
void anaLadder::printAll() {
  cout << "=== printAll ===" << endl;  
  cout << "Directory: " << fDirectory
       << " PN: " << fLadderPN
       << endl;
  for (auto it: fNoiseScan) {
    // -- print chipID
    cout << it.first << " " << fLadderPN << endl;
    // -- 
    cout << "Noisy Pixels:" << endl;
    for (auto iv: it.second.NoisyPixels) {
      cout << iv << ",";
    }
    cout << endl;
    // -- 
    cout << "Noisy Hits:" << endl;
    for (auto iv: it.second.NoisyHits) {
      cout << iv << ",";
    }
    cout << endl;
    // -- 
    cout << "Iterations:" << endl;
    for (auto iv: it.second.Iterations) {
      cout << iv << ",";
    }
    cout << endl;
    // -- 
    cout << "Not maskable pixels:" << endl;
    for (auto iv: it.second.NotMaskablePixels) {
      cout << iv << ",";
    }
    cout << endl;
    // -- 
    cout << "Errorrate_link_A:" << endl;
    for (auto iv: it.second.Errorrate_link_A) {
      cout << iv << ",";
    }
    cout << endl;
    // -- 
    cout << "Errorrate_link_B:" << endl;
    for (auto iv: it.second.Errorrate_link_B) {
      cout << iv << ",";
    }
    cout << endl;
    // -- 
    cout << "Errorrate_link_C:" << endl;
    for (auto iv: it.second.Errorrate_link_C) {
      cout << iv << ",";
    }
    cout << endl;
  }
}
