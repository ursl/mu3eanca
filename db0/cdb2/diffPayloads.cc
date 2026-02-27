#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <cstring>
#include <glob.h>
#include <map>

#include "Mu3eCalFactory.hh"
#include "calPixelQualityLM.hh"
#include "calFibreQuality.hh"
#include "calTileQuality.hh"
#include "calPixelEfficiency.hh"
#include "calEventStuffV1.hh"
#include "calPixelTimeCalibration.hh"
#include "calPixelAlignment.hh"
#include "calFibreAlignment.hh"
#include "calMppcAlignment.hh"
#include "calTileAlignment.hh"
#include "calDetSetupV1.hh"
#include "calEventStuffV1.hh"

#include "TH2D.h"
#include "TH1D.h"
#include "TFile.h"
#include "TCanvas.h"

#include "util.hh"

using namespace std;

// ----------------------------------------------------------------------
vector<string> glob(const string& pattern) {
  vector<string> vFiles;
  glob_t globbuf;
  string lPattern = pattern;
  cout << "globbing " << lPattern << endl;
  if (lPattern.find("*") == string::npos) {
    lPattern = lPattern + "*";
  }
  if (::glob(lPattern.c_str(), GLOB_TILDE, NULL, &globbuf) == 0) {
    for (size_t i = 0; i < globbuf.gl_pathc; i++) {
      vFiles.push_back(string(globbuf.gl_pathv[i]));
    }
  }
  globfree(&globbuf);
  return vFiles;
}


// ----------------------------------------------------------------------
int main(int argc, char *argv[]) {

  string firstGlob("unset"), secondGlob("unset"), sRunList("unset");
  // -- command line parsing 
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-a")) { firstGlob = string(argv[++i]);  }
    if (!strcmp(argv[i], "-b")) { secondGlob = string(argv[++i]); }
    if (!strcmp(argv[i], "-r")) { sRunList = string(argv[++i]); }
  }

  vector<int> vRuns;
  if (sRunList != "unset") {
    stringstream ss(sRunList);
    string token;
    while (getline(ss, token, ',')) {
      vRuns.push_back(stoi(token));
    }
  }


  vector<string> vFirstPayloads = glob(firstGlob);
  vector<string> vSecondPayloads = glob(secondGlob);
  // -- figure out what type of payload is used
  string sPayloadType;
  if (!vFirstPayloads.empty()) {
    if (vFirstPayloads[0].find("pixelqualitylm") != string::npos) {
      sPayloadType = "pixelqualitylm_";
    } else if (vFirstPayloads[0].find("fibrequality") != string::npos) {
      sPayloadType = "fibrequality_";
    } else if (vFirstPayloads[0].find("tilequality") != string::npos) {
      sPayloadType = "tilequality_";
    } else if (vFirstPayloads[0].find("pixelefficiency") != string::npos) {
      sPayloadType = "pixelefficiency_";
    } else if (vFirstPayloads[0].find("pixeltimecalibration") != string::npos) {
      sPayloadType = "pixeltimecalibration_";
    } else if (vFirstPayloads[0].find("pixelalignment") != string::npos) {
      sPayloadType = "pixelalignment_";
    } else if (vFirstPayloads[0].find("fibrealignment") != string::npos) {
      sPayloadType = "fibrealignment_";
    } else if (vFirstPayloads[0].find("mppcalignment") != string::npos) {
      sPayloadType = "mppcalignment_";
    } else if (vFirstPayloads[0].find("tilealignment") != string::npos) {
      sPayloadType = "tilealignment_";
    } else if (vFirstPayloads[0].find("detsetupv1") != string::npos) {
      sPayloadType = "detsetupv1_";
    } else if (vFirstPayloads[0].find("eventstuffv1") != string::npos) {
      sPayloadType = "eventstuffv1_";
    } else {
      cout << "Unknown payload type: " << vFirstPayloads[0] << endl;
      return 1;
    }
  } 

    
  string sFirstPayloadDir, sSecondPayloadDir;
  if (!vFirstPayloads.empty()) {
    string p = vFirstPayloads[0];
    string::size_type pos = p.find("/payloads/");
    sFirstPayloadDir = (pos != string::npos) ? p.substr(0, pos + 9) : p.substr(0, p.find_last_of("/") + 1);
    if (p.find("payloads/") == 0) sFirstPayloadDir = "payloads";
  } else {
    sFirstPayloadDir = firstGlob.substr(0, firstGlob.find_last_of("/") + 1);
  }
  if (!vSecondPayloads.empty()) {
    string p = vSecondPayloads[0];
    string::size_type pos = p.find("/payloads/");
    sSecondPayloadDir = (pos != string::npos) ? p.substr(0, pos + 9) : p.substr(0, p.find_last_of("/") + 1);
    if (p.find("payloads/") == 0) sSecondPayloadDir = "payloads";
  } else {
    sSecondPayloadDir = secondGlob.substr(0, secondGlob.find_last_of("/") + 1);
  }
  cout << "sFirstPayloadDir = " << sFirstPayloadDir << " sSecondPayloadDir = " << sSecondPayloadDir << endl;


  for (auto it: vRuns) {
    string sRun = "_iov_" + to_string(it);
    auto firstPayload = find_if(vFirstPayloads.begin(), vFirstPayloads.end(), 
                                [&sRun](const string& s) { return s.find(sRun) != string::npos; }); 
    auto secondPayload = find_if(vSecondPayloads.begin(), vSecondPayloads.end(), 
                                [&sRun](const string& s) { return s.find(sRun) != string::npos; });
    if ((firstPayload != vFirstPayloads.end()) && (secondPayload != vSecondPayloads.end())) {
      cout << "run = " << it << " with payloads " << *firstPayload << " and " << *secondPayload << endl;
    } else {
      cout << "run = " << it << " not found" << endl;
      continue;
    }  

    string sFirstPayloadHash = (*firstPayload).substr((*firstPayload).find_last_of("/")+1);
    string sSecondPayloadHash = (*secondPayload).substr((*secondPayload).find_last_of("/")+1);

    
    calAbs *pPQ1 = Mu3eCalFactory::instance()->createClassFromFile(sFirstPayloadHash, sFirstPayloadDir);
    pPQ1->readPayloadFromFile(sFirstPayloadHash, sFirstPayloadDir);
    pPQ1->calculate(sFirstPayloadHash);
    string sFirstPayloadBLOB = pPQ1->makeBLOB();

    calAbs *pPQ2 = Mu3eCalFactory::instance()->createClassFromFile(sSecondPayloadHash, sSecondPayloadDir);
    pPQ2->readPayloadFromFile(sSecondPayloadHash, sSecondPayloadDir);
    pPQ2->calculate(sSecondPayloadHash);
    string sSecondPayloadBLOB = pPQ2->makeBLOB();

    if (sFirstPayloadBLOB != sSecondPayloadBLOB) {
      cout << "run = " << it << " payloads are different: " 
           << sFirstPayloadHash << " " << sSecondPayloadHash 
           << endl;
    } else {
      cout << "run = " << it << " payloads are the same: " 
           << sFirstPayloadHash << " " << sSecondPayloadHash 
           << endl;
    }

    delete pPQ1;
    delete pPQ2;
  }

  return 0;
}
