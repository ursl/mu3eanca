#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <glob.h>

#include "calAbs.hh"


#include "calFibreAlignment.hh"
#include "calFibreQuality.hh"
#include "calMppcAlignment.hh"

#include "calPixelAlignment.hh"
#include "calPixelEfficiency.hh"
#include "calPixelQualityLM.hh"
#include "calPixelTimeCalibration.hh"

#include "calTileAlignment.hh"
#include "calTileQuality.hh"


using namespace std;

// ---------------------------------------------------------------------- 
// cdbPurgePayloads
// ----------------
//
// Compare each payload to the preceding one (in terms of runnumber). 
// If there is no difference (ine the BLOB), remove it.
// If there is a difference, keep and make it the new reference
//
// Usage: ./cdbPurgePayloads <payloadfiles>
//
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
int main(int argc, const char* argv[]) {
  cout << "cdbPurgePayloads" << endl;
  // -- command line arguments
  vector<string> filenames;
  for (int i = 1; i < argc; i++) {
    filenames.push_back(argv[i]);
  }


  calAbs *cal(0);
  string refBLOB("");

  string filename = filenames[0].substr(filenames[0].find_last_of("/")+1);
  string dirname = filenames[0].substr(0, filenames[0].find_last_of("/")+1);
  cout << "Reading reference payload from " << filename << " in directory " << dirname << endl;

  if (filenames[0].find("fibrequality") != string::npos) {
    cal = new calFibreQuality();
    cal->readPayloadFromFile(filename, dirname);
    refBLOB = cal->printBLOBString(cal->getPayload(filename).fBLOB, 0);
  } else if (filenames[0].find("pixelqualitylm") != string::npos) {
    cal = new calPixelQualityLM();
    cal->readPayloadFromFile(filename, dirname);
    refBLOB = cal->printBLOBString(cal->getPayload(filename).fBLOB, 0);
  } else if (filenames[0].find("tilequality") != string::npos) {
    cal = new calTileQuality();
    cal->readPayloadFromFile(filename, dirname);
    refBLOB = cal->printBLOBString(cal->getPayload(filename).fBLOB, 0);
  } else {
    cout << "Unknown payload type: " << filenames[0] << endl;
    return 0;
  }
  
  
  // -- read in all payloads
  for (unsigned int i = 1; i < filenames.size(); i++) {
    string filename = filenames[i].substr(filenames[i].find_last_of("/")+1);
    string dirname = filenames[i].substr(0, filenames[i].find_last_of("/")+1);
    cout << "Reading payload from " << filename << " in directory " << dirname << endl;
    cal->readPayloadFromFile(filename, dirname);
    string calBLOB = cal->printBLOBString(cal->getPayload(filename).fBLOB, 0);

    if (calBLOB == refBLOB) {
      cout << "Payload " << filenames[i] << " = reference, moving " << filenames[i] << " away" << endl;
      rename(filenames[i].c_str(), (filenames[i] + ".old").c_str());
    }
    else {
      cout << "Payload " << filenames[i] << " != reference, new reference: " << filenames[i] << endl;
      refBLOB = calBLOB;
    }
  }

}
