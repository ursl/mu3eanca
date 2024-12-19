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
                                                 fLadderPN(pn), fLadderInformation("nada") {
  cout << "anaLadder::anaLadder(" << fDirectory
       << ", " << fLadderPN
       << ") ctor" << endl;

  // -- fill map with empty struct
  for (int ic = 1; ic <= 6; ++ic) {
    string chipID = Form("C%d", ic);
    struct noise_scan a;
    fNoiseScan.insert({chipID, a});
    fNoiseScan[chipID].ThHigh.clear();
    fNoiseScan[chipID].NoisyPixels.clear();
    fNoiseScan[chipID].NoisyHits.clear();
    fNoiseScan[chipID].Iterations.clear();
    fNoiseScan[chipID].NotMaskablePixels.clear();
    fNoiseScan[chipID].Errorrate_link_A.clear();
    fNoiseScan[chipID].Errorrate_link_B.clear();
    fNoiseScan[chipID].Errorrate_link_C.clear();

    struct errorRate er = {-999, -999, {-2, -2, -2}};
    fAnaErrorRate.insert({chipID, er});

    fAnaLastStableThreshold.insert({chipID, -999});
  }

  parseFiles();
  if (fLadderInformation == "nada") fLadderInformation = "goodRead";

  anaErrorRate();
  anaLVCurrents();

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
  parseNoiseScans();
  parseCheckContact();
}


// ----------------------------------------------------------------------
void anaLadder::parseNoiseScans() {
  for (auto it: fHalves) {
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
    
    vector<string> params = {"ThHigh", "Noisy Pixels", "Noisy Hits", "Iterations"
                             , "Not maskable Pixels"
                             , "Errorrate_link_A",  "Errorrate_link_B",  "Errorrate_link_C"
    };
    
    for (auto ip: params) {
      for (int ic = startIdx; ic < startIdx+3; ++ic) {
        string chipID = Form("C%d", ic);
        vector<string> getChip = {chipID, ip};
        string sarr = jsonGetVector(lBuffer, getChip);
        if (sarr == "parseError") {
          fLadderInformation = "parseError(" + noiseFile + ")";
          continue;
        }
        vector<string> svector = split(sarr, ',');
        // cout << fLadderPN + "_" + it + "_" + chipID << ": " << ip << endl;        
        vector<int> *v;
        if (ip == "ThHigh") v = &(fNoiseScan[chipID].ThHigh);
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
void anaLadder::parseCheckContact() {
  for (auto it: fHalves) {
    int startIdx(1); 
    if (it == "DS") startIdx = 4;
    ifstream INS;
    string noiseFile = fDirectory + "/check_contact_" + fLadderPN + "_" + it + ".json";
    cout << "Reading file: " << noiseFile << endl;   
    INS.open(noiseFile);
    
    std::stringstream buffer;
    buffer << INS.rdbuf();
    INS.close();
    string lBuffer = buffer.str();
    
    vector<string> params = {"LV current (mA)"};
    
    for (auto ip: params) {
      for (int ic = startIdx; ic < startIdx+3; ++ic) {
        string chipID = Form("C%d", ic);
        vector<string> getChip = {chipID, ip};
        string sarr = jsonGetVector(lBuffer, getChip);
        if (sarr == "parseError") {
          fLadderInformation = "parseError(" + noiseFile + ")";
          continue;
        }
        vector<string> svector = split(sarr, ',');
        // cout << fLadderPN + "_" + it + "_" + chipID << ": " << ip << endl;        
        vector<int> *v;
        if (ip == "LV current (mA)") v = &(fCheckContact[chipID].LVCurrent);
        for (auto iv: svector) {
          v->push_back(stof(iv));
        }
      }
    }
  }
}


// ----------------------------------------------------------------------
void anaLadder::anaErrorRate(int stableLinkCut)  {
  // -- check for broken links (error rate > 1e5 for all indices)
  int maxErr(100000);
  for (auto it: fNoiseScan) {
    int cntA(0), cntB(0), cntC(0);
    for (unsigned int ii = 0; ii < it.second.Errorrate_link_A.size(); ++ii) {
      if (it.second.Errorrate_link_A[ii] > maxErr) {
        ++cntA;
      } 
      if (it.second.Errorrate_link_B[ii] > maxErr) {
        ++cntB;
      } 
      if (it.second.Errorrate_link_C[ii] > maxErr) {
        ++cntC;
      } 

      // -- store minimal error rate per link (complicated by the initial value -2)
      if (fAnaErrorRate[it.first].linkErrors[0] < 0) {
        fAnaErrorRate[it.first].linkErrors[0] = it.second.Errorrate_link_A[ii];
      } else if (it.second.Errorrate_link_A[ii] < fAnaErrorRate[it.first].linkErrors[0]) {
        fAnaErrorRate[it.first].linkErrors[0] = it.second.Errorrate_link_A[ii];
      }
      if (fAnaErrorRate[it.first].linkErrors[1] < 0) {
        fAnaErrorRate[it.first].linkErrors[1] = it.second.Errorrate_link_B[ii];
      } else if (it.second.Errorrate_link_B[ii] < fAnaErrorRate[it.first].linkErrors[1]) {
        fAnaErrorRate[it.first].linkErrors[1] = it.second.Errorrate_link_B[ii];
      }
      if (fAnaErrorRate[it.first].linkErrors[2] < 0) {
        fAnaErrorRate[it.first].linkErrors[2] = it.second.Errorrate_link_C[ii];
      } else if (it.second.Errorrate_link_C[ii] < fAnaErrorRate[it.first].linkErrors[2]) {
        fAnaErrorRate[it.first].linkErrors[2] = it.second.Errorrate_link_C[ii];
      }
      
    }
    if (cntA == it.second.Errorrate_link_A.size()) {
      fAnaErrorRate[it.first].linkErrors[0] = -99;
    }
    if (cntB == it.second.Errorrate_link_B.size()) {
      fAnaErrorRate[it.first].linkErrors[1] = -99;
    }
    if (cntC == it.second.Errorrate_link_C.size()) {
      fAnaErrorRate[it.first].linkErrors[2] = -99;
    }
  }

  for (auto it: fNoiseScan) {
    cout << fLadderPN << " errorRate: " << it.first << ": ";
    int minThr = 999, minIdx(-999), errorRate(-999);
    // -- try to find all perfect links (0 error rate at lowest threshold)
    for (unsigned int ii = 0; ii < it.second.Errorrate_link_A.size(); ++ii) {
      if ((0 == it.second.Errorrate_link_A[ii])
          && (0 == it.second.Errorrate_link_B[ii])
          && (0 == it.second.Errorrate_link_C[ii])
          ) {
        int thr = it.second.ThHigh[ii];        
        if (thr < minThr) {
          minThr = thr;
          minIdx = ii;
          errorRate = 0;
        }
      }
    }
    // -- not all links are perfect. 
    //    Try to find lowest threshold with errorrate < 500 ignoring
    //    - broken links
    //    - perfect links
    if (minIdx < 0)  {
      for (unsigned int ii = 0; ii < it.second.Errorrate_link_A.size(); ++ii) {
        if (fAnaErrorRate[it.first].linkErrors[0] > 0) {
          if (it.second.Errorrate_link_A[ii] > stableLinkCut) continue;
        }
        if (fAnaErrorRate[it.first].linkErrors[1] > 0) {
          if (it.second.Errorrate_link_B[ii] > stableLinkCut) continue;
        }
        if (fAnaErrorRate[it.first].linkErrors[2] > 0) {
          if (it.second.Errorrate_link_C[ii] > stableLinkCut) continue;
        }
        int thr = it.second.ThHigh[ii];        
        if (thr < minThr) {
          minThr = thr;
          minIdx = ii;
          if (fAnaErrorRate[it.first].linkErrors[0] > -90) {
            errorRate = it.second.Errorrate_link_A[ii];
          }
          if (fAnaErrorRate[it.first].linkErrors[1] > -90) {
            if (errorRate < it.second.Errorrate_link_B[ii]) errorRate = it.second.Errorrate_link_B[ii];
          }
          if (fAnaErrorRate[it.first].linkErrors[2] > -90) {
            if (errorRate < it.second.Errorrate_link_C[ii]) errorRate = it.second.Errorrate_link_C[ii];
          }
        }
      }
    }
    cout << " errors = " << errorRate << " minThr = " << minThr << " minIdx = " << minIdx;
    cout << " link quality: " 
         << fAnaErrorRate[it.first].linkErrors[0] << " " 
         << fAnaErrorRate[it.first].linkErrors[1] << " " 
         << fAnaErrorRate[it.first].linkErrors[2] << " " 
         << endl;
  }
  
  
}


// ----------------------------------------------------------------------
void anaLadder::printAll() {
  cout << "=== printAll ===" << endl;  
  cout << "Directory: " << fDirectory
       << " PN: " << fLadderPN
       << " status: " << fLadderInformation
       << endl;

  // -- Information from noise_scan files
  cout << fLadderPN << " broken links: ";
  for (auto it: fAnaErrorRate) {
    if (0)    cout << it.first << ": " 
                   << ((it.second.linkErrors[0] < -990)? "A":"")
                   << ((it.second.linkErrors[1] < -990)? "B":"")
                   << ((it.second.linkErrors[2] < -990)? "C":"")
                   << "  ";
  }
  if (0) cout << endl;

  cout << fLadderPN << " link quality: ";
  for (auto it: fAnaErrorRate) {
    if (0) cout << it.first << ": " 
                <<       Form("(%d/%d/%d) ", it.second.linkErrors[0], it.second.linkErrors[1],it.second.linkErrors[2])
                << "  ";
  }
  if (0) cout << endl;

  // -- Information from check_contact files
  cout << fLadderPN << " LV currents: ";
  for (auto it: fCheckContact) {
    cout << it.second.LVCurrent[2] << " ";
  }
  cout << endl;
  
}


// ----------------------------------------------------------------------
void anaLadder::anaLVCurrents(int mode) {
  for (auto it: fCheckContact) {
    fAnaLVCurrents.insert({it.first, it.second.LVCurrent[2]});
  }
}


// ----------------------------------------------------------------------
void anaLadder::bookHist() {

}
